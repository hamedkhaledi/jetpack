
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
using namespace std;
#include <random>

#define PI 3.14159265

int main()
{
    double param, result;
    param = 1.0;
    result = atan(1) * 180 / PI;
    printf("The arc tangent of %f is %f degrees\n", param, result);
    return 0;
}