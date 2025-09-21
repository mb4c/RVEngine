#include "Random.hpp"

f32 Random::Range(const f32 min, const f32 max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<> dist(min, max);
    return dist(gen);
}
i32 Random::Range(const i32 min, const i32 max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}