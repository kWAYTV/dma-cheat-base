#ifndef TIMER_HPP
#define TIMER_HPP

#ifdef DISABLE_TIMERS
#define TIMER(name) 
#else
#define TIMER(name) Timer timer(name)

class Timer
{
public:
    Timer(const string& name);
    ~Timer();

private:
    struct FunctionData {
        long long totalTime = 0;
        int invocations = 0;
    };

    string name;
    chrono::time_point<chrono::high_resolution_clock> startTimePoint;

    static map<string, FunctionData> functionData;
    static mutex dataMutex;
};
#endif // DISABLE_TIMERS

#endif // TIMER_HPP
