#include <random>

int generate_random_int(int min, int max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

double generate_random_double(double min, double max)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dist(min, max);
    return dist(gen);
}
