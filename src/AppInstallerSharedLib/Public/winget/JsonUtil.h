// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <json/json.h>

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace AppInstaller::JSON
{
    // For JSON CPP Lib
    template<class T>
    std::optional<T> GetValue(const Json::Value& node);

    template<>
    std::optional<std::string> GetValue<std::string>(const Json::Value& node);

    template<>
    std::optional<uint32_t> GetValue<uint32_t>(const Json::Value& node);

    template<>
    std::optional<bool> GetValue<bool>(const Json::Value& node);

    template<>
    std::optional<std::vector<std::string>> GetValue<std::vector<std::string>>(const Json::Value& node);

    std::optional<std::reference_wrapper<const Json::Value>> GetJsonValueFromNode(const Json::Value& node, std::string_view keyName);
    std::optional<std::reference_wrapper<const Json::Value>> GetJsonValueFromNode(const Json::Value& node, std::wstring_view keyName);

    std::optional<std::string> GetRawStringValueFromJsonValue(const Json::Value& value);
    std::optional<std::string> GetRawStringValueFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::optional<std::string> GetRawStringValueFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::optional<bool> GetRawBoolValueFromJsonValue(const Json::Value& value);
    std::optional<bool> GetRawBoolValueFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::optional<bool> GetRawBoolValueFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::optional<int> GetRawIntValueFromJsonValue(const Json::Value& value);
    std::optional<int> GetRawIntValueFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::optional<int> GetRawIntValueFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::optional<uint64_t> GetRawUInt64ValueFromJsonValue(const Json::Value& value);
    std::optional<uint64_t> GetRawUInt64ValueFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::optional<uint64_t> GetRawUInt64ValueFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::optional<std::reference_wrapper<const Json::Value>> GetRawJsonArrayFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::optional<std::reference_wrapper<const Json::Value>> GetRawJsonArrayFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::vector<std::string> GetRawStringArrayFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::vector<std::string> GetRawStringArrayFromJsonNode(const Json::Value& node, std::wstring_view keyName);
    std::set<std::string> GetRawStringSetFromJsonNode(const Json::Value& node, std::string_view keyName);
    std::set<std::string> GetRawStringSetFromJsonNode(const Json::Value& node, std::wstring_view keyName);

    std::wstring GetUtilityString(std::string_view nodeName);
    Json::Value GetStringValue(std::string_view value);

    // Base64 encode
    std::string Base64Encode(const std::vector<BYTE>& input);

    // Base64 decode
    std::vector<BYTE> Base64Decode(const std::string& input);

    bool IsValidNonEmptyStringValue(std::optional<std::string>& value);
    bool IsValidNonEmptyStringValue(std::optional<std::wstring>& value);
}
