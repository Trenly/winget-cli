// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include <winget/JsonUtil.h>

using namespace AppInstaller;

Json::Value GetTestJsonObject()
{
    Json::Value jsonObject;
    jsonObject["Key1"] = "Value1";
    jsonObject["Key2"] = "Value2";
    jsonObject["IntKey"] = 100;

    Json::Value arrayValue{ Json::arrayValue };
    arrayValue[0] = "ArrayValue1";
    arrayValue[1] = "ArrayValue2";
    arrayValue[2] = "ArrayValue3";
    jsonObject["Array"] = std::move(arrayValue);

    return jsonObject;
}

TEST_CASE("GetUtilityString", "[RestSource]")
{
    REQUIRE(JSON::GetUtilityString("jsoncpp") == L"jsoncpp");
    REQUIRE(JSON::GetUtilityString("  ") == L"  ");
}

TEST_CASE("GetJsonValueFromNode", "[RestSource]")
{
    Json::Value jsonObject = GetTestJsonObject();
    std::optional<std::reference_wrapper<const Json::Value>> actual = JSON::GetJsonValueFromNode(jsonObject, L"Key1");
    REQUIRE(actual);
    REQUIRE(actual.value().get().asString() == "Value1");

    std::optional<std::reference_wrapper<const Json::Value>> absentKey = JSON::GetJsonValueFromNode(jsonObject, L"Key3");
    REQUIRE(!absentKey);

    Json::Value emptyObject;
    std::optional<std::reference_wrapper<const Json::Value>> empty = JSON::GetJsonValueFromNode(emptyObject, L"Key1");
    REQUIRE(!empty);
}

TEST_CASE("GetRawStringValueFromJsonValue", "[RestSource]")
{
    std::optional<std::string> stringTest = JSON::GetRawStringValueFromJsonValue(Json::Value{ "jsoncpp " });
    REQUIRE(stringTest);
    REQUIRE(stringTest.value() == "jsoncpp ");

    std::optional<std::string> emptyTest = JSON::GetRawStringValueFromJsonValue(Json::Value{ "   " });
    REQUIRE(emptyTest);
    REQUIRE(emptyTest.value() == "   ");

    Json::Value obj;
    std::optional<std::string> nullTest = JSON::GetRawStringValueFromJsonValue(obj);
    REQUIRE(!nullTest);

    Json::Value integer = 100;
    std::optional<std::string> mismatchFieldTest = JSON::GetRawStringValueFromJsonValue(integer);
    REQUIRE(!mismatchFieldTest);
}

TEST_CASE("GetRawStringValueFromJsonNode", "[RestSource]")
{
    Json::Value jsonObject = GetTestJsonObject();

    std::optional<std::string> stringTest = JSON::GetRawStringValueFromJsonNode(jsonObject, L"Key1");
    REQUIRE(stringTest);
    REQUIRE(stringTest.value() == "Value1");

    std::optional<std::string> emptyTest = JSON::GetRawStringValueFromJsonNode(jsonObject, L"Key3");
    REQUIRE(!emptyTest);

    std::optional<std::string> mismatchFieldTest = JSON::GetRawStringValueFromJsonNode(jsonObject, L"IntKey");
    REQUIRE(!mismatchFieldTest);
}

TEST_CASE("GetRawIntValueFromJsonValue", "[RestSource]")
{
    Json::Value jsonObject = 100;
    std::optional<int> expected = JSON::GetRawIntValueFromJsonValue(jsonObject);
    REQUIRE(expected);
    REQUIRE(expected.value() == 100);

    std::optional<int> mismatchFieldTest = JSON::GetRawIntValueFromJsonValue(Json::Value{ "jsoncpp" });
    REQUIRE(!mismatchFieldTest);
}

TEST_CASE("GetRawJsonArrayFromJsonNode", "[RestSource]")
{
    Json::Value jsonObject = GetTestJsonObject();
    std::optional<std::reference_wrapper<const Json::Value>> expected = JSON::GetRawJsonArrayFromJsonNode(jsonObject, L"Array");
    REQUIRE(expected);
    REQUIRE(expected.value().get().size() == 3);
    REQUIRE(expected.value().get()[0].asString() == "ArrayValue1");

    std::optional<std::reference_wrapper<const Json::Value>> mismatchFieldTest = JSON::GetRawJsonArrayFromJsonNode(jsonObject, L"Keyword");
    REQUIRE(!mismatchFieldTest);
}

TEST_CASE("GetRawStringArrayFromJsonNode", "[RestSource]")
{
    Json::Value jsonObject = GetTestJsonObject();
    std::vector<std::string> expected = JSON::GetRawStringArrayFromJsonNode(jsonObject, L"Array");
    REQUIRE(expected.size() == 3);
    REQUIRE(expected[0] == "ArrayValue1");

    std::vector<std::string> mismatchFieldTest = JSON::GetRawStringArrayFromJsonNode(jsonObject, L"Keyword");
    REQUIRE(mismatchFieldTest.size() == 0);
}
