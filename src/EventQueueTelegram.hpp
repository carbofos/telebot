#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "telegames.hpp"
#include <iostream>

class EventQueueTelegram {
public:
    static void push(const std::shared_ptr<Event> event) {
        if (event == nullptr) {
            std::cerr << "Trying to push nullptr to queue EventQueueTelegram" << std::endl;
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(event);
        cond_var_.notify_one();
        std::cout << "Pushed event to queue EventQueueTelegram: " << event->type << std::endl;
    }

    static std::shared_ptr<Event> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [] { return !queue_.empty(); });
        auto event = queue_.front();
        queue_.pop();
        return event;
    }

    static bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    static std::queue<std::shared_ptr<Event>> queue_;
    static std::mutex mutex_;
    static std::condition_variable cond_var_;
};
