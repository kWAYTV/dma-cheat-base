#include <Pch.hpp>

#ifndef DISABLE_TIMERS

#include "Timer.hpp"

// Initialize static members
map<string, Timer::FunctionData> Timer::functionData;
mutex Timer::dataMutex;

Timer::Timer(const string& name) : name(name) {
    startTimePoint = chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    auto endTimePoint = chrono::high_resolution_clock::now();
    auto start = chrono::time_point_cast<chrono::milliseconds>(startTimePoint).time_since_epoch().count();
    auto end = chrono::time_point_cast<chrono::milliseconds>(endTimePoint).time_since_epoch().count();
    auto duration = end - start;

    {
        lock_guard<mutex> lock(dataMutex);
        auto& data = functionData[name];
        data.totalTime += duration;
        data.invocations++;
        auto averageTime = data.totalTime / data.invocations;

		LOG_INFO("{}' took '{}'ms - Average '{}'ms - Invocations '{}'", name, duration, averageTime, data.invocations);
    }
}

#endif // DISABLE_TIMERS
