#include <tgbot/tgbot.h>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include "telegames.hpp"
#include "Quest.hpp"
#include "EventQueueVM.hpp"
#include "EventQueueTelegram.hpp"

void EventProcessorTelegram(TgBot::Bot &bot) {
    std::cout << "Starting event processor" << std::endl;
    while (true) {
        std::shared_ptr<Event>  event = EventQueueTelegram::pop();
        std::cout << "Event processor got new event type" << event->type << std::endl;
        if (event) {
            if (event->data.type() == typeid(QuestAttrQuestion)) {
                QuestAttrQuestion question = std::any_cast<QuestAttrQuestion>(event->data);
                std::cout << "Question: " << question.question << std::endl;
                TgBot::InlineKeyboardMarkup::Ptr markup(new TgBot::InlineKeyboardMarkup);
                // markup->oneTimeKeyboard = true;
                // markup->resizeKeyboard = true;
                if (question.buttons.size() > 0) {
                    std::vector<TgBot::InlineKeyboardButton::Ptr> row;

                    std::cout << "Buttons: " << std::endl;
                    for (const auto& b : question.buttons) {
                        TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton);
                        button->text = b.button;
                        button->callbackData = b.id;
                        row.push_back(button);
                    }
                    markup->inlineKeyboard.push_back(row);
                }
                bot.getApi().sendMessage(event->tg_userid, question.question, nullptr, nullptr, markup);
            }
            else if (event->data.type() == typeid(std::string)) {
                std::string s = std::any_cast<std::string>(event->data);
                std::cout << "msg: " << s << std::endl;
                if (event->type == QuestNodeType::Text) {
                    bot.getApi().sendMessage(event->tg_userid, s);
                } else if (event->type == QuestNodeType::HTML) {
                    bot.getApi().sendMessage(event->tg_userid, s, nullptr, nullptr, nullptr, "HTML");
                }
                // bot.getApi().sendMessage(event->tg_userid, s, nullptr, nullptr, nullptr, nullptr, "HTML");
                // bot.getApi().sendMessage(message->chat->id, htmlMessage, false, 0, nullptr, nullptr, "HTML");
            }
            else {
                std::cerr << "Unsupported event type for Question" << std::endl;
            }
        }
    }
}

int main() {
    std::string jsonString = R"({
        "name": "quest1",
        "description": "Это очень долгожданный первый квест, который все очень долго долго ждали",
        "id": "111",
        "program": [
            {
                "type": "Text",
                "id": "123",
                "attributes": {
                    "value": "Добро пожаловать в первый квест"
                }
            },
            {
                "type": "Text",
                "id": "124",
                "attributes": {
                    "value": "Начнем"
                }
            },
            {
                "type": "HTML",
                "id": "125",
                "attributes": {
                    "value": "<b>Жирный текст</b>, <i>курсив</i>, <u>подчеркнутый</u><code>код</code>, <pre>preformatted fixed-width code</pre>"
                }
            },
            {
                "type": "Question",
                "id": "300",
                "attributes": {
                    "value": {
                        "question": "Ну ответьте же наконец!",
                        "buttons": [
                            {
                                "button": "Ответ 1",
                                "id": "123"
                            },
                            {
                                "button": "Большой Ответ 2",
                                "id": "124"
                            },
                            {
                                "button": "очень очень Большой Ответ 3",
                                "id": "125"
                            }
                        ]
                    }
                }
            }
        ]
    })";

    const std::string token = getenv("TELEGRAM_TOKEN");
        if (token.empty()) {
            std::cerr << "Telegram token not found in environment variable TELEGRAM_TOKEN" << std::endl;
            return 1;
        }
    std::cout << token << std::endl;

    json j = json::parse(jsonString);
    std::map<std::string, std::shared_ptr<const Quest>> quests;
    {
        Quest quest;
        quest.from_json(j);
        quests[token] = std::make_shared<const Quest>(quest);
    }
    QuestVM::run();

    TgBot::Bot bot(token);
    
    bot.getEvents().onCommand("start", [&bot, &quests, &token](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Привет! Вы вошли в квест");
        std::cout << "UserID connected: " << message->from->id << std::endl;
        std::cout << "Quest description: " << quests[token]->description << std::endl;
        std::cout << ".. program size: " << quests[token]->program.nodes.size() << std::endl;
        TG_USERID tg_userid = message->from->id;
        QuestVM::addworld(tg_userid, quests[token]);
    });

    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
        TG_USERID tg_userid = query->message->chat->id;
        int64_t messageId = query->message->messageId;
        //TODO: надо сохранять message->id и отправлять answer только если это сообщение валидно
        std::cout << "Callback data: " <<  query->message->chat->id << ", message ID " <<  query->message->messageId << std::endl;
        // uint32_t gotoPosition;
        // try {
        //     gotoPosition = std::stoi(query->data);
        //     std::cout << "Callback data: " << query->data << ", message ID " << messageId << std::endl;
        // } catch (const std::invalid_argument& e) {
        //     std::cerr << "Invalid argument: " << e.what() << std::endl;
        //     return;
        // } catch (const std::out_of_range& e) {
        //     std::cerr << "Out of range: " << e.what() << std::endl;
        //     return;
        // }
        // TODO: Check if ID exists in the world
        // std::shared_ptr<Event> event = std::make_shared<Event>(QuestNodeType::GoTo, tg_userid, gotoPosition);
        std::shared_ptr<Event> event = std::make_shared<Event>(QuestNodeType::GoTo, tg_userid, query->data);
        EventQueueVM::push(event);
        bot.getApi().answerCallbackQuery(query->id);
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        if (StringTools::startsWith(message->text, "/")) {
            return;
        }
        std::cout << "Message from UserID : " << message->from->id << std::endl;
        });

    std::thread(EventProcessorTelegram, std::ref(bot)).detach();


    std::cout << "Бот запущен..." << std::endl;
    TgBot::TgLongPoll longPoll(bot);
    while (true) {
    try {
            longPoll.start();
            std::cout << ".";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << std::endl;
        }
    }

    return 0;
}
