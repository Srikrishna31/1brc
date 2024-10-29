//
// Created by coolk on 27-10-2024.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <limits>
#include <chrono>


using namespace std::string_literals;

struct Stats {
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double sum = 0.0;
    int64_t count = 0;
};

void processFile(const std::string& filename) {
    auto file = std::ifstream{filename};
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    auto statsMap = std::unordered_map<std::string, Stats>{};
    auto line = ""s;

    while (std::getline(file, line)) {
        auto ss = std::stringstream {line};
        auto station = ""s, measurementStr = ""s;
        if (!std::getline(ss, station, ';') || !std::getline(ss, measurementStr)) {
            continue;
        }
        double measurement = std::stod(measurementStr);

        auto& stat = statsMap[station];

        stat.min = std::min(stat.min, measurement);
        stat.max = std::max(stat.max, measurement);
        stat.sum += measurement;
        stat.count++;
    }

    std::cout << "{";
    for (const auto& [station, stat]: statsMap) {
        double mean = stat.sum / stat.count;
        std::cout << station << "=" << stat.min << "/" << mean << "/" << stat.max << ", " << std::endl;
    }
    std::cout << "\b\b}" << std::endl;
}

auto timeFuncInvocation = [](auto&& func, auto&&... params) -> double {
    const auto& start = std::chrono::high_resolution_clock::now();
    std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
    const auto& stop = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(stop - start).count();
};
int main(int , char** )
{
    try {
        auto time = timeFuncInvocation(processFile, "D:/Github_Projects/1brc/data/measurements-1000000000.txt"s);
        std::cout << std::endl << "Time taken to process: " << time / 1000 << " seconds" << std::endl;

    } catch(std::exception& e) {
        std::cout << e.what();
        return -1;
    }

    return 0;
}