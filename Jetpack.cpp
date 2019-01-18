#include "SBDL.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
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
    int ShowRand;
    int ShowRandDelay;
    SDL_Rect Rect;
} Coins, Zapper[3];
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

void RestartBackground(Pos Background[])
{
    for (int i = 0; i < 3; i++)
        Background[i].x = 2048 * (i + 3);
}
void DeterminZapperPos(int i)
{
    mt19937 rng;
    rng.seed(std::random_device()());
    /* if (Zapper[i].ShowRandDelay % 25 == 0)
    {
        uniform_int_distribution<std::mt19937::result_type> dist3(0, 0);
        Zapper[i].ShowRand = dist3(rng);
        Zapper[i].ShowRandDelay = 1;
    }
    if (Zapper[i].ShowRand == 0)
    {*/
    int counter = 0;
    do
    {
        uniform_int_distribution<std::mt19937::result_type> dist1(1024, 2048);
        Zapper[i].PositionStart.x = dist1(rng);
        uniform_int_distribution<std::mt19937::result_type> dist2(0, 1024);
        Zapper[i].PositionStart.y = dist2(rng);
        Zapper[i].Rect = {Zapper[i].PositionStart.x,
                          Zapper[i].PositionStart.y,
                          300,
                          50};
        counter++;
        cout << counter << " ";
    } while (SBDL::hasIntersectionRect(Zapper[i].Rect, Coins.Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 1) % 3].Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 2) % 3].Rect));

    /*Zapper[i].ShowRandDelay = 0;
     }
    else
        Zapper[i].ShowRandDelay++;*/
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
    uniform_int_distribution<std::mt19937::result_type> Rand1(1024, 5120);
    Coins.PositionStart.x = Rand1(rng);
    uniform_int_distribution<std::mt19937::result_type> Rand2(0, 440 - 15 * linesCounter);
    Coins.PositionStart.y = Rand2(rng);
    Coins.PositionEnd.x = Coins.PositionStart.x + 15 * eachLineLength;
    Coins.PositionEnd.y = Coins.PositionStart.y + 15 * linesCounter;
    Coins.Rect = {Coins.PositionStart.x,
                  Coins.PositionStart.y,
                  Coins.PositionEnd.x - Coins.PositionStart.x,
                  Coins.PositionEnd.y - Coins.PositionStart.y};
    for (int i = 0; i < 3; i++)
    {
        while (SBDL::hasIntersectionRect(Zapper[i].Rect, Coins.Rect))
        {
            cout << "yes";
            Coins.PositionStart.x = Rand1(rng);
            Coins.PositionStart.y = Rand2(rng);
            Coins.PositionEnd.x = Coins.PositionStart.x + 15 * eachLineLength;
            Coins.PositionEnd.y = Coins.PositionStart.y + 15 * linesCounter;
            Coins.Rect = {Coins.PositionStart.x,
                          Coins.PositionStart.y,
                          Coins.PositionEnd.x - Coins.PositionStart.x,
                          Coins.PositionEnd.y - Coins.PositionStart.y};
        }
    }
}
void Restart()
{
    CoinPattern();
    srand(time(NULL));
    for (int i = 0; i < 3; i++)
    {
        Background1[i].x = 2048 * i;
        Background2[i].x = 2048 * (i + 3);
        DeterminZapperPos(i);
    }

    r1 = 0;
    r2 = rand() % 3 + 1;
    Barry.Position.y = 370;
    Barry.vel = 0;
    Barry.ShapeNumber = 1;
    RunCounter = 0;
}
int main()
{
    mt19937 rng;
    rng.seed(random_device()());
    srand(time(NULL));
    SBDL::InitEngine("Jetpack", 1024, 460);
    Texture BackgroundTexture[4][3];
    Texture BarryTexture[5];
    Texture ZapperTexture = SBDL::loadTexture("assets/pic/zappers/h1.png");
    for (int j = 1; j <= 5; j++)
        BarryTexture[j] = SBDL::loadTexture("assets/pic/barry/barry" + to_string(j) + ".png");
    for (int j = 0; j < 3; j++)
    {
        BackgroundTexture[0][j] = SBDL::loadTexture("assets/pic/back/Lab" + to_string(j) + ".png");
        BackgroundTexture[1][j] = SBDL::loadTexture("assets/pic/back/Sector" + to_string(j) + ".png");
        BackgroundTexture[2][j] = SBDL::loadTexture("assets/pic/back/Volcano" + to_string(j) + ".png");
        BackgroundTexture[3][j] = SBDL::loadTexture("assets/pic/back/warehouse" + to_string(j) + ".png");
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
            SBDL::showTexture(BackgroundTexture[r1][i], Background1[i].x, 0);
            SBDL::showTexture(BackgroundTexture[r2][i], Background2[i].x, 0);
            Background1[i].x -= BackgrondVelocity;
            Background2[i].x -= BackgrondVelocity;
            if (Background1[2].x <= -2048)
            {
                j = rand() % 4;     //// safhe tekrari load nashe
                while (r2 == j)     //// safhe tekrari load nashe
                    j = rand() % 4; //// safhe tekrari load nashe
                r1 = j;             //// safhe tekrari load nashe
                RestartBackground(Background1);
            }
            if (Background2[2].x <= -2048)
            {
                j = rand() % 4;     //// safhe tekrari load nashe
                while (r1 == j)     //// safhe tekrari load nashe
                    j = rand() % 4; //// safhe tekrari load nashe
                r2 = j;             //// safhe tekrari load nashe
                RestartBackground(Background2);
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
        else if (Barry.Position.y < 370)
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
        if (Barry.Position.y >= 370)
        {
            if (RunCounter % 20 < 10)
                Barry.ShapeNumber = 2;
            else
                Barry.ShapeNumber = 1;

            RunCounter++;
            Barry.Position.y = 370;
            Barry.vel = 0;
        }
        SBDL::showTexture(BarryTexture[Barry.ShapeNumber], 50, Barry.Position.y);
        Barry.Rect = {50, Barry.Position.y, BarryTexture[Barry.ShapeNumber].width, BarryTexture[Barry.ShapeNumber].height};
        //END BARRY
        //COINS
        counterCoin++;
        for (int i = 0; i < linesCounter; i++)
            for (int j = 0; j < eachLineLength; j++)
            {
                if (grid[i][j] == 1)
                {
                    coinsAr[i][j].Position.x = 15 * j + Coins.PositionStart.x - coinsAr[i][j].images[coinsAr[i][j].currentIndex].width / 2;
                    coinsAr[i][j].Position.y = Coins.PositionStart.y + 15 * i;
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
        Coins.PositionEnd.x = Coins.PositionStart.x + 15 * eachLineLength;
        if (Coins.PositionEnd.x < 0)
        {
            CoinPattern();
            Coins.PositionStart.x = 1024;
        }
        if (counterCoin > 2)
            counterCoin = 0;
        //END COINS
        //ZAPPERS
        for (int i = 0; i < 3; i++)
        {
            SBDL::showTexture(ZapperTexture, Zapper[i].PositionStart.x, Zapper[i].PositionStart.y);
            Zapper[i].PositionStart.x -= BackgrondVelocity;
            Zapper[i].PositionEnd.x = Zapper[i].PositionStart.x + ZapperTexture.width;

            if (Zapper[i].PositionEnd.x < 0)
            {
                if (Zapper[i].ShowRandDelay % 50 == 0)
                {
                    Zapper[i].ShowRand = rand() % 4;
                    Zapper[i].ShowRandDelay = 0;
                }
                if (Zapper[i].ShowRand == 0)
                {
                    DeterminZapperPos(i);
                    Zapper[i].ShowRandDelay = 0;
                }
                else
                    Zapper[i].ShowRandDelay++;
            }
        }
        //END ZAPPERS

        SBDL::updateRenderScreen();
        int elapsedTime = SBDL::getTime() - startTime;
        if (elapsedTime < delay)
            SBDL::delay(delay - elapsedTime);
    }
}