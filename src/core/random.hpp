#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <vector>
#include <stdexcept>

class Random {
private:
    inline static std::random_device rd;
    inline static std::mt19937 generator{rd()};

public:
    static int integer(int min, int max) {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(generator);
    }

    static double real(double min, double max) {
        std::uniform_real_distribution<double> distribution(min, max);
        return distribution(generator);
    }

    template <typename T>
    static const T& choice(const std::vector<T>& values) {
        if (values.empty())
            throw std::runtime_error("Cannot choose from an empty vector");
        return values[integer(0, static_cast<int>(values.size()) - 1)];
    }
};

#endif
