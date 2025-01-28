#include "Quest.hpp"

std::vector<QuestWorld> QuestVM::worlds;
std::mutex QuestVM::newworldsMutex;
std::thread QuestVM::QuestVMThread;
std::queue<QuestWorld> QuestVM::newworlds;
