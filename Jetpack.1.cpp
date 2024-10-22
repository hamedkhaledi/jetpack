#include "SBDL.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
#define PI 3.14159265
using namespace std;

long long int Score;
int r1, r2; //taein texture random baraye back1 , back2
int RunCounter;
int BackgrondVelocity = 8;
int counterCoin = 0;
int linesCounter, eachLineLength;
int **grid;

struct Pos
{
    float x, y;
};
enum obstacleMode
{
    coin,
    lazer
} ObstacleMode;
struct background
{
    Pos Position;
    bool show;
} Background1[3], Background2[3];
struct Joy
{
    Pos Position;
    Texture Shape;
    int ShapeNumber;
    float vel;
    SDL_Rect Rect;
} Barry;
struct FlippingThing
{
    int size;
    Texture *images = nullptr;
    std::string folderAddress = "assets/pic/coin/";
    int currentIndex;
    bool show;
    Pos Position;
    SDL_Rect Rect;
} * *coinsAr;
struct Obstacle
{
    Pos PositionStart;
    Pos PositionEnd;
    bool Show;
    int Width;
    int Height;
    int size;
    int ShowRand;
    int ShowRandDelay;
    int ShapeNumber;
    SDL_Rect Rect;
    int Angle;
    int AngleSign;
    bool active;
} Coins, Zapper[3], Lazers[3];
void initializeFlippingCoin(FlippingThing &flipping)
{
    flipping.images = new Texture[6];
    flipping.size = 6;
    flipping.currentIndex = 0;
    flipping.show = true;
    for (int i = 0; i < flipping.size; i++)
    {
        string temp = flipping.folderAddress + to_string(i + 1);
        temp.append(".png");
        flipping.images[i] = SBDL::loadTexture(temp);
    }
}

void RestartBackground(background Background[])
{
    for (int i = 0; i < 3; i++)
        Background[i].Position.x = 2048 * (i + 3);

    Background[0].show = false;
}
void DetermineZapperPos(int i)
{
    mt19937 rng;
    rng.seed(std::random_device()());
    do
    {
        uniform_int_distribution<std::mt19937::result_type> dist1(1024, 3072);
        Zapper[i].PositionStart.x = dist1(rng);
        uniform_int_distribution<std::mt19937::result_type> dist2(100, 350);
        Zapper[i].PositionStart.y = dist2(rng);
        Zapper[i].size = sqrt(Zapper[i].Width * Zapper[i].Width + Zapper[i].Height * Zapper[i].Height);
        Zapper[i].Rect = {Zapper[i].PositionStart.x + (1 - cos((float)Zapper[i].Angle * PI / 180)) * Zapper[i].size / 2,
                          Zapper[i].PositionStart.y - (Zapper[i].AngleSign) * (sin((float)Zapper[i].Angle * PI / 180) * Zapper[i].size / 2) + 15,
                          Zapper[i].Width,
                          Zapper[i].Height};
    } while (SBDL::hasIntersectionRect(Zapper[i].Rect, Coins.Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 1) % 3].Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 2) % 3].Rect));
}
void DetermineZapper(int i)
{
    mt19937 rng;
    rng.seed(std::random_device()());
    if (Zapper[i].ShowRandDelay % 150 == i * 50)
    {
        uniform_int_distribution<std::mt19937::result_type> dist3(0, 15);
        Zapper[i].ShowRand = dist3(rng);
        Zapper[i].ShowRandDelay = i * 50 + 1;
    }
    else
        Zapper[i].ShowRandDelay++;
    if (Zapper[i].ShowRand == 1)
    {
        uniform_int_distribution<std::mt19937::result_type> dist1(1, 200);
        Zapper[i].Width = dist1(rng);
        if (Zapper[i].Width > 100)
        {
            uniform_int_distribution<std::mt19937::result_type> dist2(1, 200);
            Zapper[i].Height = dist2(rng);
        }
        else
        {
            uniform_int_distribution<std::mt19937::result_type> dist2(150, 250);
            Zapper[i].Height = dist2(rng);
        }

        do
        {
            uniform_int_distribution<std::mt19937::result_type> RandAngleSign(-1, 1);
            Zapper[i].AngleSign = RandAngleSign(rng);
        } while (Zapper[i].AngleSign == 0);
        Zapper[i].Angle = Zapper[i].AngleSign * (atan((float)Zapper[i].Height / Zapper[i].Width) * 180 / PI);
        cout << Zapper[i].Angle << endl;
        DetermineZapperPos(i);
        Zapper[i].ShowRandDelay = i * 50 + 1;
    }
}
void DetermineCoinPos()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand1(1024, 5120);
    Coins.PositionStart.x = Rand1(rng);
    uniform_int_distribution<std::mt19937::result_type> Rand2(10, 540 - 30 * linesCounter);
    Coins.PositionStart.y = Rand2(rng);
    Coins.PositionEnd.x = Coins.PositionStart.x + 30 * eachLineLength;
    Coins.PositionEnd.y = Coins.PositionStart.y + 30 * linesCounter;
    Coins.Rect = {Coins.PositionStart.x,
                  Coins.PositionStart.y,
                  30 * eachLineLength, 30 * linesCounter};
}
void CoinPattern()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand(1, 27);
    ifstream fin("assets/coinPatterns/coinPattern" + to_string(Rand(rng)) + ".txt");
    linesCounter = 0;
    eachLineLength = 0;
    string temp;
    // baa in while gharaare te'dad-e khathaa va tool-e har khat ro be dast biaarim
    while (getline(fin, temp))
    {
        eachLineLength = max(eachLineLength, (int)(temp.size()));
        linesCounter++;
    }
    //	aaraayeE ke sekkeHaa-e charkhaan raa dar khod negah midaarad
    coinsAr = new FlippingThing *[linesCounter];
    for (int i = 0; i < linesCounter; i++)
        coinsAr[i] = new FlippingThing[eachLineLength];
    //	aaraayeE ke moshakhas mikonad ke dar har noghte aayaaa sekke daarim, yaa oon khoone khaalie!
    grid = new int *[linesCounter];
    for (int i = 0; i < linesCounter; i++)
        grid[i] = new int[eachLineLength];
    // baa do ta dastoor-e ba'di, be ifstreamemoon migim ke bargard avval-e file!
    fin.clear();
    fin.seekg(0, std::ios::beg);
    int lineCounterTemp = 0;
    //aaraayeHaa-e grid va coinsAr ro meghdaarDehi mikonim baa tavajjoh be file
    counterCoin = 0;
    while (getline(fin, temp))
    {
        if (!temp.empty())
        {
            for (int i = 0; i < temp.size(); i++)
            {
                grid[lineCounterTemp][i] = (temp[i] - '0');
                if (grid[lineCounterTemp][i] == 1)
                    initializeFlippingCoin(coinsAr[lineCounterTemp][i]);
            }
        }
        lineCounterTemp++;
    }
    DetermineCoinPos();
    int i = -1;
    while (i < 2)
    {
        i++;
        if (SBDL::hasIntersectionRect(Zapper[i].Rect, Coins.Rect))
        {
            i = -1;
            DetermineCoinPos();
        }
    }
}
void LazerPattern()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand(1024, 2048);
    Lazers[0].PositionStart.x = Rand(rng);
    for (int i = 0; i < 5; i++)
    {
        uniform_int_distribution<std::mt19937::result_type> Rand1(0, 1);
        Lazers[i].Show = Rand1(rng);
    }
}
void Restart()
{
    CoinPattern();
    srand(time(NULL));

    Background1[0].show = true;
    Background2[0].show = false;
    for (int i = 0; i < 3; i++)
    {

        Background1[i].Position.x = 2048 * i;
        Background2[i].Position.x = 2048 * (i + 3);
        Zapper[i].PositionStart.x = -500;
    }

    r1 = 0;
    r2 = rand() % 3 + 1;
    Barry.Position.y = 455;
    Barry.vel = 0;
    Barry.ShapeNumber = 1;
    RunCounter = 0;
    LazerPattern();
}
int main()
{
    mt19937 rng;
    rng.seed(random_device()());
    srand(time(NULL));
    SBDL::InitEngine("Jetpack", 2048, 550);
    Texture BackgroundTexture[4][3];
    Texture BarryTexture[5];
    Texture LazarTexture[3];
    Texture ZapperTexture = SBDL::loadTexture("assets/pic/zappers/h1.png");

    for (int j = 1; j <= 5; j++)
        BarryTexture[j] = SBDL::loadTexture("assets/pic/barry/barry" + to_string(j) + ".png");
    for (int j = 0; j < 3; j++)
    {
        BackgroundTexture[0][j] = SBDL::loadTexture("assets/pic/back/Lab" + to_string(j) + ".png");
        BackgroundTexture[1][j] = SBDL::loadTexture("assets/pic/back/Sector" + to_string(j) + ".png");
        BackgroundTexture[2][j] = SBDL::loadTexture("assets/pic/back/Volcano" + to_string(j) + ".png");
        BackgroundTexture[3][j] = SBDL::loadTexture("assets/pic/back/warehouse" + to_string(j) + ".png");
        LazarTexture[j] = SBDL::loadTexture("assets/pic/lazer/laser" + to_string(j) + ".png");
    }

    const int FPS = 60;           //frame per second
    const int delay = 1000 / FPS; //delay we need at each frame

    Restart();
    //
    //	streami ke be file-e mored-e nazaremoon vasl mishe... ba komakesh gharaare mohtaviaat-e file ro bekhoonim

    //

    while (SBDL::isRunning())
    {
        int startTime = SBDL::getTime();
        SBDL::updateEvents();
        SBDL::clearRenderScreen();

        //BACKGROUND
        for (int i = 0, j; i < 3; i++)
        {
            SBDL::showTexture(BackgroundTexture[r1][i], Background1[i].Position.x, 0);
            SBDL::showTexture(BackgroundTexture[r2][i], Background2[i].Position.x, 0);
            Background1[i].Position.x -= BackgrondVelocity;
            Background2[i].Position.x -= BackgrondVelocity;
            if (Background1[2].Position.x <= -2048)
            {
                j = rand() % 4;     //// safhe tekrari load nashe
                while (r2 == j)     //// safhe tekrari load nashe
                    j = rand() % 4; //// safhe tekrari load nashe
                r1 = j;             //// safhe tekrari load nashe
                RestartBackground(Background1);
                Background2[0].show = true;
            }
            if (Background2[2].Position.x <= -2048)
            {
                j = rand() % 4;     //// safhe tekrari load nashe
                while (r1 == j)     //// safhe tekrari load nashe
                    j = rand() % 4; //// safhe tekrari load nashe
                r2 = j;             //// safhe tekrari load nashe
                RestartBackground(Background2);
                Background1[0].show = true;
            }
        }
        //END BACKGROUND
        //BARRY
        if (SBDL::keyHeld(SDL_SCANCODE_SPACE))
        {
            RunCounter = 0;
            Barry.vel -= 0.3;
            Barry.ShapeNumber = 3;
        }
        else if (Barry.Position.y < 455)
        {
            RunCounter = 0;
            Barry.vel += 0.3;
            Barry.ShapeNumber = 4;
        }
        Barry.Position.y += Barry.vel;
        if (Barry.Position.y <= 10)
        {

            Barry.Position.y = 10;
            Barry.vel = 0;
        }
        if (Barry.Position.y >= 455)
        {
            if (RunCounter % 20 < 10)
                Barry.ShapeNumber = 2;
            else
                Barry.ShapeNumber = 1;

            RunCounter++;
            Barry.Position.y = 455;
            Barry.vel = 0;
        }
        SBDL::showTexture(BarryTexture[Barry.ShapeNumber], 50, Barry.Position.y);
        Barry.Rect = {50, Barry.Position.y, BarryTexture[Barry.ShapeNumber].width, BarryTexture[Barry.ShapeNumber].height};
        //END BARRY
        //ZAPPERS
        for (int i = 0; i < 3; i++)
        {
            ZapperTexture.width = Zapper[i].size;
            ZapperTexture.height = 30;
            SBDL::showTexture(ZapperTexture, Zapper[i].PositionStart.x, Zapper[i].PositionStart.y, Zapper[i].Angle);
            if (Zapper[i].PositionEnd.x < -500 * (i + 1) && ObstacleMode == coin)
                DetermineZapper(i);
            Zapper[i].PositionStart.x -= BackgrondVelocity;
            Zapper[i].PositionEnd.x = Zapper[i].PositionStart.x + Zapper[i].Width;
            Zapper[i].Rect = {Zapper[i].PositionStart.x + (1 - cos((float)Zapper[i].Angle * PI / 180)) * ZapperTexture.width / 2,
                              Zapper[i].PositionStart.y - Zapper[i].AngleSign * (sin((float)Zapper[i].Angle * PI / 180) * Zapper[i].size / 2) + ZapperTexture.height / 2,
                              Zapper[i].Width,
                              Zapper[i].Height};
            SBDL::drawRectangle(Zapper[i].Rect, 0, 0, 0, 200);
        }
        //END ZAPPERS
        //COINS
        counterCoin++;

        for (int i = 0; i < linesCounter; i++)
            for (int j = 0; j < eachLineLength; j++)
            {
                if (grid[i][j] == 1)
                {
                    coinsAr[i][j].Position.x = 30 * j + Coins.PositionStart.x - coinsAr[i][j].images[coinsAr[i][j].currentIndex].width / 2;
                    coinsAr[i][j].Position.y = Coins.PositionStart.y + 30 * i;
                    coinsAr[i][j].Rect = {coinsAr[i][j].Position.x, coinsAr[i][j].Position.y, coinsAr[i][j].images[coinsAr[i][j].currentIndex].width, coinsAr[i][j].images[coinsAr[i][j].currentIndex].height};
                    if (SBDL::hasIntersectionRect(Barry.Rect, coinsAr[i][j].Rect) && coinsAr[i][j].show)
                    {
                        coinsAr[i][j].show = false;
                        Score++;
                    }

                    if (coinsAr[i][j].show)
                        SBDL::showTexture(coinsAr[i][j].images[coinsAr[i][j].currentIndex], coinsAr[i][j].Position.x, coinsAr[i][j].Position.y);
                    if (counterCoin > 2)
                        coinsAr[i][j].currentIndex = (coinsAr[i][j].currentIndex + 1) % coinsAr[i][j].size;
                }
            }
        Coins.PositionStart.x -= BackgrondVelocity;
        Coins.PositionEnd.x = Coins.PositionStart.x + 30 * eachLineLength;
        Coins.Rect = {Coins.PositionStart.x,
                      Coins.PositionStart.y,
                      30 * eachLineLength, 30 * linesCounter};
        if (counterCoin > 2)
            counterCoin = 0;
        if (Coins.PositionEnd.x < 0 && ObstacleMode == coin)
        {
            if (Lazers[0].ShowRandDelay == 0)
            {
                uniform_int_distribution<std::mt19937::result_type> Rand(0, 5);
                if (Rand(rng) == 1)
                    ObstacleMode = lazer;
                else
                    ObstacleMode = coin;
            }
            if (ObstacleMode == coin)
            {
                CoinPattern();
                Lazers[0].ShowRandDelay = 0;
            }
        }

        //END COINS
        //LAZERS
        if (ObstacleMode == lazer)
        {

            if (Lazers[0].ShowRandDelay > 200 && Lazers[0].ShowRandDelay <= 300)
            {
                for (int i = 0; i < 5; i++)
                {
                    Lazers[i].active = false;
                    Lazers[i].Rect = {0,
                                      10 + 100 * i,
                                      LazarTexture[0].width,
                                      LazarTexture[0].height};
                    if (Lazers[i].Show == true)
                        SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                }
            }
            if (Lazers[0].ShowRandDelay > 300 && Lazers[0].ShowRandDelay <= 450)
            {
                if (Lazers[0].ShowRandDelay % 10 == 0)
                    if (Lazers[0].ShowRand == 1)
                        Lazers[0].ShowRand = 2;
                    else
                        Lazers[0].ShowRand = 1;
                for (int i = 0; i < 3; i++)
                    if (Lazers[i].Show == true)
                    {
                        SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                        Lazers[i].active = false;
                    }
                for (int i = 3; i < 5; i++)
                    if (Lazers[i].Show == true)
                    {
                        Lazers[i].active = true;
                        SBDL::showTexture(LazarTexture[Lazers[0].ShowRand], 0, 10 + 100 * i);
                    }
            }
            if (Lazers[0].ShowRandDelay > 450 && Lazers[0].ShowRandDelay <= 600)
            {
                if (Lazers[0].ShowRandDelay % 10 == 0)
                    if (Lazers[0].ShowRand == 1)
                        Lazers[0].ShowRand = 2;
                    else
                        Lazers[0].ShowRand = 1;
                for (int i = 3; i < 5; i++)
                    if (Lazers[i].Show == true)
                    {
                        SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                        Lazers[i].active = false;
                    }
                for (int i = 0; i < 3; i++)
                    if (Lazers[i].Show == true)
                    {
                        SBDL::showTexture(LazarTexture[Lazers[0].ShowRand], 0, 10 + 100 * i);
                        Lazers[i].active = true;
                    }
            }
            if (Lazers[0].ShowRandDelay > 600)
            {
                LazerPattern();
                ObstacleMode = coin;
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    if (SBDL::hasIntersectionRect(Barry.Rect, Lazers[i].Rect) && Lazers[i].active)
                    {
                        cout << "lose";
                    }
                }
                Lazers[0].ShowRandDelay++;
            }
        }

        //END LAZERS
        SBDL::updateRenderScreen();
        int elapsedTime = SBDL::getTime() - startTime;
        if (elapsedTime < delay)
            SBDL::delay(delay - elapsedTime);
    }
}