#pragma once
#include <cstdint>
#include <any>
#include <nlohmann/json.hpp>

using TG_USERID = int64_t;

enum QuestNodeType {
    Text,
    Image,
    Pause,
    Question,
    HTML,
    GoTo,
    Finish
};

// enum EventType {
//     evText,
//     evImage,
//     evPause,
//     evQuestion,
//     evHTML,
//     evResume,
//     evAnswer,
//     evMessageToUser,
// };

NLOHMANN_JSON_SERIALIZE_ENUM(QuestNodeType, {
    {Text, "Text"},
    {Image, "Image"},
    {Pause, "Pause"},
    {Question, "Question"},
    {HTML, "HTML"},
    {GoTo, "GoTo"},
    {Finish, "Finish"}
})

struct Event {
    Event(QuestNodeType type, TG_USERID tg_userid, std::any data) : type(type), tg_userid(tg_userid), data(data) {};
    QuestNodeType type;
    TG_USERID tg_userid;
    std::any data;
};
