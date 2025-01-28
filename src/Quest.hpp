#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <thread>
#include <chrono>
#include "telegames.hpp"
#include "EventQueueTelegram.hpp"
#include "EventQueueVM.hpp"

using json = nlohmann::json;

struct QuestQuestionButton {
    std::string button;
    std::string id;
    void from_json(const json& j) {
        j.at("button").get_to(button);
        j.at("id").get_to(id);
    }
};

struct QuestAttrQuestion {
    std::string question;
    std::string id;
    std::vector<QuestQuestionButton> buttons;

    void from_json(const json& j) {
        const auto& value = j.at("value");
        value.at("question").get_to(question);
        for (const auto& jb : value.at("buttons")) {
            QuestQuestionButton button;
            button.from_json(jb);
            buttons.push_back(button);
        }

    }
};

struct QuestNode {
    QuestNodeType type;
    // std::string description;
    std::string id;
    std::any attributes;

    void from_json(const json& j) {
        j.at("type").get_to(type);
        j.at("id").get_to(id);

        if (type == Text || type == HTML) {
            attributes = j.at("attributes").at("value").get<std::string>();
        } else if (type == Question) {
            QuestAttrQuestion question;
            question.from_json(j.at("attributes"));
            attributes = question;
        } else {
            attributes = j.at("attributes").at("value").get<json>();
        }
    }
};

struct QuestProgram {
    std::string name;
    std::string id;
    std::vector<QuestNode> nodes;

    void from_json(const json& j) {
        std::cout << j << std::endl;
        for (const auto& nodeJson : j) {
            QuestNode node;
            node.from_json(nodeJson);
            nodes.push_back(node);
        }
    }
};

struct Quest {
public:
    std::string name;
    std::string description;
    std::string id;
    QuestProgram program;

    void from_json(const json& j) {
        j.at("name").get_to(name);
        j.at("description").get_to(description);
        j.at("id").get_to(id);
        program.from_json(j.at("program"));
    }
};

using QuestWorldID = std::pair<TG_USERID, const std::shared_ptr<const Quest>>;

class QuestWorld {
    public:
        QuestWorld(const TG_USERID tg_userid, std::shared_ptr<const Quest> &quest) : worldID(std::make_pair(tg_userid, quest)) {
            std::cout << "Creating QuestWorld for " << tg_userid << std::endl;
            instructionPointer = 0;
            std::cout << "QuestWorld Quest description: " << quest->description << std::endl;
            std::cout << "QuestWorld.. program size: " << quest->program.nodes.size() << std::endl;
        }

        std::shared_ptr<Event> executeNextStep() {
            std::cout << "Executing next step " << instructionPointer << " out of " << quest()->program.nodes.size() << std::endl;
            std::shared_ptr<Event> event = nullptr;
            if (worldID.second->program.nodes.size() == 0)
                throw std::runtime_error("No nodes in program");
            if (instructionPointer < worldID.second->program.nodes.size()) {
                QuestNodeType command = worldID.second->program.nodes[instructionPointer].type;
                switch (command) {
                    case Text:
                    case HTML:
                        std::cout << "Executing Text/HTML Node: " << worldID.second->program.nodes[instructionPointer].id << std::endl;
                        event = std::make_shared<Event>(command, tg_userid(), worldID.second->program.nodes[instructionPointer].attributes);
                        break;
                    case Question:
                        std::cout << "Waiting for answer in node: " << worldID.second->program.nodes[instructionPointer].id << std::endl;
                        event = std::make_shared<Event>(command, tg_userid(), worldID.second->program.nodes[instructionPointer].attributes);
                        isPaused = true;
                        break;
                    case Image:
                        std::cout << "Executing Image Node: " << worldID.second->program.nodes[instructionPointer].id << std::endl;
                        break;
                    case Pause:
                        std::cout << "Executing Pause Node: " << worldID.second->program.nodes[instructionPointer].id << std::endl;
                        isPaused = true;
                        // std::thread([this]() {
                        //     std::this_thread::sleep_for(std::chrono::seconds(std::any_cast<int>(quest.program.nodes[instructionPointer].attributes)));
                        //     // event - switch paused to false
                        //     /// евенты должны быть глобальными в одну очередь
                        //     // this->eventsIn.push_back(EventIn());
                        // }).detach();
                        break;
                    default:
                        std::cerr << "Unsupported node type: " << command << std::endl;
                        break;
                }
                if (instructionPointer >= quest()->program.nodes.size()) {
                    isPaused = true;
                    return std::make_shared<Event>(QuestNodeType::Finish, tg_userid(), std::string("Quest finished"));
                }
                ++instructionPointer;
                return event;
            }
            // if (instructionPointer >= quest.program.nodes.size())
            //     return EventOut();
            return event;
        }

        bool getIsPaused() const {
                    return isPaused;
                }

        int64_t getIPbyNodeId(std::string nodeId) const {
            for (size_t i = 0; i < worldID.second->program.nodes.size(); ++i) {
                if (worldID.second->program.nodes[i].id == nodeId) {
                    return i;
                }
            }
            return -1;
        }

        // void gotoNode(uint32_t position) {
        void gotoNode(std::string position) {
            // TODO: check if position is valid
            auto new_instructionPointer = getIPbyNodeId(position);
            if (new_instructionPointer >= 0) {
                instructionPointer = new_instructionPointer;
                isPaused = false;
            }
            else {
                std::cerr << "Invalid position: " << position << std::endl;
            }
        }

        TG_USERID tg_userid() const {
            return worldID.first;
        }

        const std::shared_ptr<const Quest> quest() {
            return worldID.second;
        }

        const QuestWorldID worldID;
        // const TG_USERID tg_userid;
        // const Quest &quest;
    private:
        bool isPaused = false;
        int instructionPointer = 0;
};

class QuestVM {
public:
    static void addworld(TG_USERID tg_userid, std::shared_ptr<const Quest> quest) {
        std::cout << "Running QuestVM for " << tg_userid << std::endl << "\"" << quest->description << "\"" << std::endl;
        newworlds.push(QuestWorld(tg_userid, quest));
    }

    static std::vector<QuestWorld>::iterator get_world_by_tguseerid(TG_USERID tg_userid) {
        for (auto it = worlds.begin(); it != worlds.end(); ++it) {
            if (it->tg_userid() == tg_userid) {
                return it;
            }
        }
        return worlds.end();
    }

    static void questvm_thread() {
        std::cout << "questvm_thread()" << std::endl;
        while (true) {
            {
                std::lock_guard<std::mutex> lock(newworldsMutex);
                while (!newworlds.empty()) {
                    worlds.push_back(newworlds.front());
                    newworlds.pop();
                }
            }

            for (auto& world : worlds) {
                // std::cout << "Iter" << std::endl;
                if (!world.getIsPaused()) {
                    auto event = world.executeNextStep();
                    if (event) {
                        EventQueueTelegram::push(event);
                    }
                }
            }
            if (!EventQueueVM::isEmpty()) {
                    std::cout << "EventQueueVM is not empty" << std::endl;
                    std::shared_ptr<Event> event = EventQueueVM::pop();
                    if (event) {
                        // if (event->data.type() == typeid(uint32_t)) {
                        //     uint32_t newposition = std::any_cast<uint32_t>(event->data);
                        //     std::cout << "Goto new position: " << newposition << std::endl;
                        //     world.gotoNode(newposition);
                        // }
                        auto w = get_world_by_tguseerid(event->tg_userid);
                        if (w != worlds.end())
                            if (event->data.type() == typeid(std::string)) {
                                std::string newposition = std::any_cast<std::string>(event->data);
                                std::cout << "Goto new position: " << newposition << std::endl;
                                w->gotoNode(newposition);
                        }
                    }
                }
       std::this_thread::sleep_for(std::chrono::milliseconds(50));
       }
    }

    static void run() {
        std::cout << "--------------------------------------" << std::endl;
        QuestVMThread = std::thread(questvm_thread);
        QuestVMThread.detach();
    }

private:
    static std::queue<QuestWorld> newworlds;
    static std::vector<QuestWorld> worlds;
    static std::mutex newworldsMutex;
    static std::thread QuestVMThread;
};
