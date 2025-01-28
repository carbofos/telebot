#define BOOST_TEST_MODULE TeleBotUnitTest
#include <boost/test/included/unit_test.hpp>
#include "Quest.hpp"



BOOST_AUTO_TEST_SUITE(QuestQuestionButtonTests)

BOOST_AUTO_TEST_CASE(from_json_valid_input) {
    json test_json = R"({
        "button": "Start",
        "id": "12345"
    })"_json;

    QuestQuestionButton q;
    q.from_json(test_json);

    BOOST_CHECK_EQUAL(q.button, "Start");
    BOOST_CHECK_EQUAL(q.id, "12345");
}

BOOST_AUTO_TEST_CASE(from_json_missing_field) {
    json test_json = R"({
        "id": "67890"
    })"_json;

    QuestQuestionButton q;

    BOOST_CHECK_THROW(q.from_json(test_json), json::out_of_range);
}

BOOST_AUTO_TEST_CASE(from_json_invalid_type) {
    json test_json = R"({
        "button": 123, 
        "id": "abcd"
    })"_json;

    QuestQuestionButton q;

    BOOST_CHECK_THROW(q.from_json(test_json), json::type_error);
}

BOOST_AUTO_TEST_SUITE_END()
