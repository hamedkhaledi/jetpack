
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
using namespace std;
#include <random>

int main()
{
    mt19937 rng;
    rng.seed(random_device()());
    for (int i = 0; i < 6; i++)
    {
        // distribution in range [1, 6]
        uniform_int_distribution<mt19937::result_type> dist6(0, 6);
        cout << dist6(rng) << endl;
    }
}