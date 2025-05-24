#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using TIME_POINT = std::chrono::system_clock::time_point;

class Timer
{
public:

    template <class Func>
    void start(int target_time, Func&& method)
    {
        worker_ = std::make_unique<std::thread>([=]() mutable 
        {
            std::cout << "\n\nSTART TIMER " << target_time <<  "\n\n";
            // std::this_thread::sleep_until(target_time);
            std::this_thread::sleep_for(std::chrono::seconds(target_time));

            std::cout << "SEND DATA!!!!";

            method();
        });
        worker_->detach();
    }

private:

    std::unique_ptr<std::thread> worker_;
    std::atomic<bool> cancelled_ = false;
};
