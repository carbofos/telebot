#include "EventQueueVM.hpp"

std::queue<std::shared_ptr<Event>> EventQueueVM::queue_;
std::mutex EventQueueVM::mutex_;
std::condition_variable EventQueueVM::cond_var_;
