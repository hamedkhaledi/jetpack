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
int CountMislle;

struct Pos
{
    float x, y;
};
enum obstacleMode
{
    coin,
    lazer
} ObstacleMode;
enum zappermode
{
    vertical,
    Horizontal,
    Rotary
} ZapperMode;
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
    bool Gravity;
    int MoveDirection; //1=bala ,-1=paiin
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
    SDL_Rect ArrayRect[6];
    int Angle;
    int AngleSign;
    bool active;
    zappermode ZapperMode;
} Coins, Zapper[3], Lazers[5];
struct Things
{
    Pos Position;
    bool Show;
    int showDalay;
    Pos Velocity;
    int TextureNumber;
    SDL_Rect Rect;
    Pos PositionStart;
    int FirstScore;
} Missle[6], SpeedToken, GravityToken;
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
        uniform_int_distribution<std::mt19937::result_type> dist1(1024, 4096);
        Zapper[i].PositionStart.x = dist1(rng);
        uniform_int_distribution<std::mt19937::result_type> dist2(Zapper[i].size, 520 - Zapper[i].size);
        Zapper[i].PositionStart.y = dist2(rng);
        Zapper[i].Rect = {Zapper[i].PositionStart.x,
                          Zapper[i].PositionStart.y - sin((float)90 * PI / 180) * Zapper[i].size / 2 + 15,
                          Zapper[i].size,
                          Zapper[i].size};
    } while (SBDL::hasIntersectionRect(Zapper[i].Rect, Coins.Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 1) % 3].Rect) || SBDL::hasIntersectionRect(Zapper[i].Rect, Zapper[(i + 2) % 3].Rect));
}
void DetermineZapper(int i)
{
    mt19937 rng;
    rng.seed(std::random_device()());
    if (Zapper[i].ShowRandDelay % 150 == i * 50)
    {
        uniform_int_distribution<std::mt19937::result_type> dist3(0, 10);
        Zapper[i].ShowRand = dist3(rng);
        Zapper[i].ShowRandDelay = i * 50 + 1;
    }
    else
        Zapper[i].ShowRandDelay++;
    if (Zapper[i].ShowRand == 1)
    {
        uniform_int_distribution<std::mt19937::result_type> dist1(200, 300);
        uniform_int_distribution<std::mt19937::result_type> dist2(0, 2);
        Zapper[i].size = dist1(rng);
        Zapper[i].Show = true;
        Zapper[i].Height = Zapper[i].size;
        Zapper[i].Width = Zapper[i].size;
        if (dist2(rng) == 0)
            Zapper[i].ZapperMode = vertical;
        else if (dist2(rng) == 1)
            Zapper[i].ZapperMode = Horizontal;
        else
            Zapper[i].ZapperMode = Rotary;
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
        Lazers[i].active = false;
    }
}
void MisslePattern()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand(10240, 20480);
    uniform_int_distribution<std::mt19937::result_type> Rand1(1, 6);
    CountMislle = Rand1(rng);
    for (int i = 0; i < CountMislle; i++)
    {
        Missle[i].Show = true;
        Missle[i].Position.x = Rand(rng) + 10 * i;
        uniform_int_distribution<std::mt19937::result_type> Rand2(10, 500);
        Missle[i].Position.y = Rand2(rng);
        Missle[i].Velocity.x = BackgrondVelocity * 2;
        Missle[i].Velocity.y = 2;
        Missle[i].TextureNumber = 0;
        Missle[i].showDalay = 0;
    }
}
void SpeedTokenPattern()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand(2048, 4096);
    uniform_int_distribution<std::mt19937::result_type> Rand2(150, 400);
    uniform_int_distribution<std::mt19937::result_type> Rand3(50, 100);
    SpeedToken.Show = true;
    SpeedToken.Position.x = Rand(rng);
    SpeedToken.PositionStart.y = Rand2(rng); //shoro harekat sinosi
    SpeedToken.Velocity.x = BackgrondVelocity / 2;
    SpeedToken.Velocity.y = Rand3(rng); //damane harekat sinosi
    SpeedToken.TextureNumber = 0;
    SpeedToken.showDalay = 0;
    SpeedToken.FirstScore = 0;
}
void GravityTokenPattern()
{
    mt19937 rng;
    rng.seed(std::random_device()());
    uniform_int_distribution<std::mt19937::result_type> Rand(4096, 10240);
    uniform_int_distribution<std::mt19937::result_type> Rand2(150, 400);
    GravityToken.Show = true;
    GravityToken.Position.x = Rand(rng);
    GravityToken.Position.y = Rand2(rng);
    GravityToken.Velocity.x = BackgrondVelocity;
}
void DisableAllObstales()
{
    for (int i = 0; i < 3; i++)
        Zapper[i].Show = false;
    for (int i = 0; i < 5; i++)
        Lazers[i].Show = false;
    for (int i = 0; i < 6; i++)
        Missle[i].Show = false;
    SpeedToken.Show = false;
    GravityToken.Show = false;
}
void EnableAllObstales()
{
    if (ObstacleMode == lazer)
        ObstacleMode = coin;
    for (int i = 0; i < 3; i++)
        Zapper[i].Show = true;
    for (int i = 0; i < 6; i++)
        Missle[i].Show = true;
    SpeedToken.Show = true;
    GravityToken.Show = true;
    MisslePattern();
}
void RestartBarry()
{
    r1 = 0;
    r2 = rand() % 3 + 1;
    Barry.Position.y = 455;
    Barry.vel = 0;
    Barry.ShapeNumber = 1;
    Barry.Gravity = false;
    RunCounter = 0;
    Barry.MoveDirection = 1;
}
void Restart()
{
    CoinPattern();
    LazerPattern();
    MisslePattern();
    SpeedTokenPattern();
    GravityTokenPattern();
    RestartBarry();
    srand(time(NULL));
    Background1[0].show = true;
    Background2[0].show = false;
    for (int i = 0; i < 3; i++)
    {
        Background1[i].Position.x = 2048 * i;
        Background2[i].Position.x = 2048 * (i + 3);
        Zapper[i].PositionStart.x = -500;
    }
}
int main()
{
    mt19937 rng;
    rng.seed(random_device()());
    srand(time(NULL));
    SBDL::InitEngine("Jetpack", 1024, 550);
    Texture BackgroundTexture[4][3];
    Texture BarryTexture[6];
    Texture LazarTexture[4];
    Texture WarninigMissleTexture[3];
    Texture MissleTexture[6];
    Texture SmokeTexture[6];
    Texture SpeedTokenTexture[4];
    Texture BarryGravityTexture[6];
    Texture ZapperTexture = SBDL::loadTexture("assets/pic/zappers/h1.png");
    Texture GravityTokenTexture = SBDL::loadTexture("assets/pic/menu/gravity_token.png");
    for (int j = 0; j < 4; j++)
    {
        LazarTexture[j] = SBDL::loadTexture("assets/pic/lazer/laser" + to_string(j) + ".png");
        SpeedTokenTexture[j] = SBDL::loadTexture("assets/pic/speedToken/speed token" + to_string(j + 1) + ".png");
    }
    for (int j = 1; j <= 5; j++)
        BarryTexture[j] = SBDL::loadTexture("assets/pic/barry/barry" + to_string(j) + ".png");
    for (int j = 0; j < 6; j++)
    {
        SmokeTexture[j] = SBDL::loadTexture("assets/pic/smoke/smoke " + to_string(j + 1) + ".png");
        MissleTexture[j] = SBDL::loadTexture("assets/pic/missle/missle (" + to_string(j + 1) + ").png");
        BarryGravityTexture[j] = SBDL::loadTexture("assets/pic/barry/gg" + to_string(j + 1) + ".png");
    }
    for (int j = 0; j < 3; j++)
    {
        BackgroundTexture[0][j] = SBDL::loadTexture("assets/pic/back/Lab" + to_string(j) + ".png");
        BackgroundTexture[1][j] = SBDL::loadTexture("assets/pic/back/Sector" + to_string(j) + ".png");
        BackgroundTexture[2][j] = SBDL::loadTexture("assets/pic/back/Volcano" + to_string(j) + ".png");
        BackgroundTexture[3][j] = SBDL::loadTexture("assets/pic/back/warehouse" + to_string(j) + ".png");
        WarninigMissleTexture[j] = SBDL::loadTexture("assets/pic/missle/" + to_string(j + 1) + "m.png");
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
        Score++;
        if (Score % 500 == 0)
            BackgrondVelocity += 1;
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
        if (Barry.Gravity == false)
        {
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
        }
        else
        {
            if (SBDL::keyPressed(SDL_SCANCODE_SPACE))
            {
                RunCounter = 0;
                Barry.MoveDirection *= -1;
                Barry.vel += 0;
                if (Barry.MoveDirection == 1)
                    Barry.ShapeNumber = 4;
                else
                    Barry.ShapeNumber = 5;
            }
            Barry.vel += Barry.MoveDirection * 0.3;
            Barry.Position.y += Barry.vel;
            if (Barry.Position.y <= 10)
            {
                if (RunCounter % 20 < 10)
                    Barry.ShapeNumber = 2;
                else
                    Barry.ShapeNumber = 3;
                RunCounter++;
                Barry.Position.y = 10;
                Barry.vel = 0;
            }
            if (Barry.Position.y >= 455)
            {
                if (RunCounter % 20 < 10)
                    Barry.ShapeNumber = 0;
                else
                    Barry.ShapeNumber = 1;

                RunCounter++;
                Barry.Position.y = 455;
                Barry.vel = 0;
            }
            SBDL::showTexture(BarryGravityTexture[Barry.ShapeNumber], 50, Barry.Position.y);
            Barry.Rect = {50, Barry.Position.y, BarryGravityTexture[Barry.ShapeNumber].width, BarryGravityTexture[Barry.ShapeNumber].height};
        }

        //END BARRY
        //ZAPPERS

        for (int i = 0; i < 3; i++)
        {
            if (Zapper[i].ZapperMode == vertical)
                Zapper[i].Angle = 90;
            else if (Zapper[i].ZapperMode == Horizontal)
                Zapper[i].Angle = 0;
            else
                Zapper[i].Angle++;
            Zapper[i].Height = Zapper[i].size * sin((float)Zapper[i].Angle * PI / 180);
            Zapper[i].Width = Zapper[i].size * cos((float)Zapper[i].Angle * PI / 180);
            ZapperTexture.width = Zapper[i].size;
            SBDL::showTexture(ZapperTexture, Zapper[i].PositionStart.x, Zapper[i].PositionStart.y, Zapper[i].Angle);
            if (Zapper[i].PositionEnd.x < -500 * (i + 1) && ObstacleMode == coin)
                DetermineZapper(i);
            Zapper[i].PositionStart.x -= BackgrondVelocity;
            Zapper[i].PositionEnd.x = Zapper[i].PositionStart.x + Zapper[i].Width;
            for (int j = 0; j < 6; j++)
            {
                Zapper[i].ArrayRect[j] = {Zapper[i].PositionStart.x + (1 - cos((float)Zapper[i].Angle * PI / 180)) * ZapperTexture.width / 2 + Zapper[i].Width * j / 6,
                                          Zapper[i].PositionStart.y - sin((float)Zapper[i].Angle * PI / 180) * Zapper[i].size / 2 + ZapperTexture.height / 2 + Zapper[i].Height * j / 6,
                                          40,
                                          40};
                SBDL::drawRectangle(Zapper[i].ArrayRect[j], 0, 0, 0, 200); //نمایش رکت ها
                if (SBDL::hasIntersectionRect(Barry.Rect, Zapper[i].ArrayRect[j]) && Zapper[i].Show)
                {
                    Barry.Gravity = false;
                    cout << "yes";
                }
            }
            Zapper[i].Rect = {Zapper[i].PositionStart.x,
                              Zapper[i].PositionStart.y - sin((float)90 * PI / 180) * Zapper[i].size / 2 + 15,
                              Zapper[i].size,
                              Zapper[i].size};
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
                uniform_int_distribution<std::mt19937::result_type> Rand(0, 4);
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
        //SPEEDTOKEN
        if (ObstacleMode == lazer)
            if (SpeedToken.Position.x > 1024)
                SpeedToken.Position.x = -500;
        if (SpeedToken.Position.x < 1024 && SpeedToken.Position.x >= -200 && SpeedToken.Show == true)
        {
            if (SpeedToken.showDalay % 5 == 0)
                SpeedToken.TextureNumber = (SpeedToken.TextureNumber + 1) % 4;
            SBDL::showTexture(SpeedTokenTexture[SpeedToken.TextureNumber], SpeedToken.Position.x, SpeedToken.Position.y);
            SpeedToken.Position.y = SpeedToken.PositionStart.y + SpeedToken.Velocity.y * sin(2 * SpeedToken.showDalay * PI / 180);
            SpeedToken.showDalay++;
        }
        SpeedToken.Rect = {
            SpeedToken.Position.x,
            SpeedToken.Position.y,
            SpeedTokenTexture[SpeedToken.TextureNumber].width,
            SpeedTokenTexture[SpeedToken.TextureNumber].height};
        SBDL::drawRectangle(SpeedToken.Rect, 0, 0, 0, 200);
        SpeedToken.Position.x -= SpeedToken.Velocity.x;
        if (SBDL::hasIntersectionRect(Barry.Rect, SpeedToken.Rect) && SpeedToken.Show && SpeedToken.FirstScore == 0)
        {
            SpeedToken.Show = false;
            SpeedToken.FirstScore = Score;
            SpeedToken.Position.x = -100;
            BackgrondVelocity *= 4;
        }

        if (SpeedToken.FirstScore > 0)
        {
            DisableAllObstales();
            if (Score - SpeedToken.FirstScore > 500)
            {
                BackgrondVelocity = BackgrondVelocity / 4;
                SpeedToken.FirstScore = 0;
                ObstacleMode = coin;
                EnableAllObstales();
            }
        }
        else if (SpeedToken.Position.x <= -500)
            SpeedTokenPattern();
        //END SPEEDTOKEN
        //GRAVITY
        if (ObstacleMode == lazer)
            if (GravityToken.Position.x > 1024)
                GravityToken.Position.x = -500;
        if (GravityToken.Position.x < 1024 && GravityToken.Position.x >= -200 && GravityToken.Show)
            SBDL::showTexture(GravityTokenTexture, GravityToken.Position.x, GravityToken.Position.y);
        GravityToken.Rect = {
            GravityToken.Position.x,
            GravityToken.Position.y,
            GravityTokenTexture.width,
            GravityTokenTexture.height};
        if (SBDL::hasIntersectionRect(Barry.Rect, GravityToken.Rect) && GravityToken.Show)
        {
            Barry.Gravity = true;
            GravityToken.Show = false;
        }
        GravityToken.Position.x -= BackgrondVelocity;
        if (GravityToken.Position.x <= -500 && Barry.Gravity == false)
            GravityTokenPattern();

        //END GRAVITY
        //MISSLE
        int TedadMoshakRadshode = 0; //تعداد موشک های رد شده از صفحه
        if (ObstacleMode == lazer)
            for (int i = 0; i < 6; i++)
                if (Missle[i].Position.x > 3072 || Missle[i].Position.x < -200)
                    Missle[i].Position.x = -500;
        for (int i = 0; i < CountMislle; i++)
        {
            if (Missle[i].Position.x < 3072 && Missle[i].Position.x >= 2000 && Missle[i].Show)
            {
                SBDL::showTexture(WarninigMissleTexture[0], 1024 - WarninigMissleTexture[1].width, Missle[i].Position.y);
                if (Barry.Position.y - Missle[i].Position.y < 0)
                    Missle[i].Position.y -= Missle[i].Velocity.y;
                else if (Barry.Position.y - Missle[i].Position.y > 0)
                    Missle[i].Position.y += Missle[i].Velocity.y;
            }
            else if (Missle[i].Position.x < 2000 && Missle[i].Position.x >= 1024 && Missle[i].Show)
                SBDL::showTexture(WarninigMissleTexture[1], 1024 - WarninigMissleTexture[1].width, Missle[i].Position.y);
            else if (Missle[i].Position.x < 1024 && Missle[i].Position.x >= -200)
            {
                if (Missle[i].showDalay % 3 == 0)
                    Missle[i].TextureNumber = (Missle[i].TextureNumber + 1) % 6;
                Missle[i].Rect = {
                    Missle[i].Position.x,
                    Missle[i].Position.y,
                    MissleTexture[Missle[i].TextureNumber].width,
                    MissleTexture[Missle[i].TextureNumber].height};
                if (Missle[i].Show)
                {
                    SBDL::drawRectangle(Missle[i].Rect, 0, 0, 0, 200);
                    SBDL::showTexture(MissleTexture[Missle[i].TextureNumber], Missle[i].Position.x, Missle[i].Position.y);
                    SBDL::showTexture(SmokeTexture[Missle[i].TextureNumber], Missle[i].Position.x + MissleTexture[Missle[i].TextureNumber].width, Missle[i].Position.y);
                }
                Missle[i].showDalay++;
            }
            Missle[i].Position.x -= Missle[i].Velocity.x;
            if (Missle[i].Position.x <= -500)
                TedadMoshakRadshode++;
            if (SBDL::hasIntersectionRect(Barry.Rect, Missle[i].Rect) && Missle[i].Show)
            {
                Barry.Gravity = false;
            }
        }
        if (TedadMoshakRadshode == CountMislle && ObstacleMode == coin)
            MisslePattern();
        //END MISSLE

        //LAZERS
        if (ObstacleMode == lazer)
        {
            for (int i = 0; i < 3; i++)
                if (Zapper[i].PositionStart.x > 1024)
                    Zapper[i].PositionStart.x = -500;

            if (Lazers[0].ShowRandDelay > 300 && Lazers[0].ShowRandDelay <= 400) // 5 ta gheir faal
            {
                for (int i = 0; i < 5; i++)
                {
                    Lazers[i].active = false;
                    Lazers[i].Rect = {0,
                                      10 + 100 * i,
                                      LazarTexture[0].width,
                                      LazarTexture[0].height};
                    if (Lazers[i].Show == true)
                        if (i < 3)
                            SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                        else
                            SBDL::showTexture(LazarTexture[3], 0, 10 + 100 * i);
                }
            }

            if (Lazers[0].ShowRandDelay > 400 && Lazers[0].ShowRandDelay <= 550) // 3 ta 5 faal
            {
                if (Lazers[0].ShowRandDelay % 10 == 0)
                    if (Lazers[0].ShowRand == 1)
                        Lazers[0].ShowRand = 2;
                    else
                        Lazers[0].ShowRand = 1;
                for (int i = 0; i < 3; i++)
                    if (Lazers[i].Show == true)
                        SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                for (int i = 3; i < 5; i++)
                    if (Lazers[i].Show == true)
                    {
                        Lazers[i].active = true;
                        SBDL::showTexture(LazarTexture[Lazers[0].ShowRand], 0, 10 + 100 * i);
                    }
            }
            if (Lazers[0].Show || Lazers[1].Show || Lazers[2].Show)
            {
                if (Lazers[0].ShowRandDelay > 550 && Lazers[0].ShowRandDelay <= 650)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        Lazers[i].active = false;
                        if (Lazers[i].Show == true)
                            if (i >= 3)
                                SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                            else
                                SBDL::showTexture(LazarTexture[3], 0, 10 + 100 * i);
                    }
                }
                if (Lazers[0].ShowRandDelay > 650 && Lazers[0].ShowRandDelay <= 800)
                {
                    if (Lazers[0].ShowRandDelay % 10 == 0)
                        if (Lazers[0].ShowRand == 1)
                            Lazers[0].ShowRand = 2;
                        else
                            Lazers[0].ShowRand = 1;
                    for (int i = 3; i < 5; i++)
                        if (Lazers[i].Show == true)
                            SBDL::showTexture(LazarTexture[0], 0, 10 + 100 * i);
                    for (int i = 0; i < 3; i++)
                        if (Lazers[i].Show == true)
                        {
                            SBDL::showTexture(LazarTexture[Lazers[0].ShowRand], 0, 10 + 100 * i);
                            Lazers[i].active = true;
                        }
                }
            }
            if (Lazers[0].ShowRandDelay > 800)
            {
                LazerPattern();
                ObstacleMode = coin;
            }
            else
            {
                for (int i = 0; i < 5; i++)
                {
                    if (SBDL::hasIntersectionRect(Barry.Rect, Lazers[i].Rect) && Lazers[i].active && Lazers[i].Show)
                    {
                        Barry.Gravity = false;
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