#include "EventQueueTelegram.hpp"

std::queue<std::shared_ptr<Event>> EventQueueTelegram::queue_;
std::mutex EventQueueTelegram::mutex_;
std::condition_variable EventQueueTelegram::cond_var_;
