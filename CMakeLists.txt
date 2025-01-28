cmake_minimum_required(VERSION 3.14)

project(telebot)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(telebot src/main.cpp src/EventQueueTelegram.cpp src/EventQueueVM.cpp src/Quest.cpp)
add_executable(testunit src/testunit.cpp)

find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL)
find_package(Boost 1.69 COMPONENTS system filesystem REQUIRED )
find_package(Threads REQUIRED)

target_link_libraries(telebot PRIVATE 
    TgBot
    Threads::Threads
    Boost::system
    Boost::filesystem
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
)
