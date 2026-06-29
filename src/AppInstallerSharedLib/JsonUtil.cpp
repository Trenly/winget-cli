// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "winget/JsonUtil.h"
#include "AppInstallerStrings.h"

namespace AppInstaller::JSON
{
    template<>
    std::optional<std::string> GetValue(const Json::Value& node)
    {
        std::optional<std::string> value;

        if (node.isString())
        {
            value = node.asString();
        }

        return value;
    }

    template<>
    std::optional<uint32_t> GetValue(const Json::Value& node)
    {
        std::optional<uint32_t> value;

        if (node.isUInt())
        {
            value = node.asUInt();
        }

        return value;
    }

    template<>
    std::optional<bool> GetValue(const Json::Value& node)
    {
        std::optional<bool> value;

        if (node.isBool())
        {
            value = node.asBool();
        }

        return value;
    }

    template<>
    std::optional<std::vector<std::string>> GetValue(const Json::Value& node)
    {
        std::vector<std::string> result;

        if (node.isArray())
        {
            for (const Json::Value& entry : node)
            {
                if (!entry.isString())
                {
                    return std::nullopt;
                }

                result.emplace_back(entry.asString());
            }

            return result;
        }

        return std::nullopt;
    }

    namespace
    {
        std::string NormalizeJsonKey(std::string_view keyName)
        {
            return std::string{ keyName };
        }

        std::string NormalizeJsonKey(std::wstring_view keyName)
        {
            return Utility::ConvertToUTF8(keyName);
        }
    }

    std::optional<std::reference_wrapper<const Json::Value>> GetJsonValueFromNode(const Json::Value& node, std::string_view keyName)
    {
        if (node.isNull() || !node.isObject())
        {
            return {};
        }

        const std::string normalizedKeyName = NormalizeJsonKey(keyName);
        if (!node.isMember(normalizedKeyName))
        {
            return {};
        }

        return node[normalizedKeyName];
    }

    std::optional<std::reference_wrapper<const Json::Value>> GetJsonValueFromNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetJsonValueFromNode(node, NormalizeJsonKey(keyName));
    }

    std::optional<std::string> GetRawStringValueFromJsonValue(const Json::Value& value)
    {
        if (value.isNull() || !value.isString())
        {
            return {};
        }

        return value.asString();
    }

    std::optional<std::string> GetRawStringValueFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> jsonValue = GetJsonValueFromNode(node, keyName);

        if (jsonValue)
        {
            return GetRawStringValueFromJsonValue(jsonValue.value().get());
        }

        return {};
    }

    std::optional<std::string> GetRawStringValueFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawStringValueFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::optional<int> GetRawIntValueFromJsonValue(const Json::Value& value)
    {
        if (value.isNull() || !value.isInt())
        {
            return {};
        }

        return value.asInt();
    }

    std::optional<int> GetRawIntValueFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> jsonValue = GetJsonValueFromNode(node, keyName);

        if (jsonValue)
        {
            return GetRawIntValueFromJsonValue(jsonValue.value().get());
        }

        return {};
    }

    std::optional<int> GetRawIntValueFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawIntValueFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::optional<uint64_t> GetRawUInt64ValueFromJsonValue(const Json::Value& value)
    {
        if (value.isNull() || !value.isUInt64())
        {
            return {};
        }

        return value.asUInt64();
    }

    std::optional<uint64_t> GetRawUInt64ValueFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> jsonValue = GetJsonValueFromNode(node, keyName);

        if (jsonValue)
        {
            return GetRawUInt64ValueFromJsonValue(jsonValue.value().get());
        }

        return {};
    }

    std::optional<uint64_t> GetRawUInt64ValueFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawUInt64ValueFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::optional<bool> GetRawBoolValueFromJsonValue(const Json::Value& value)
    {
        if (value.isNull() || !value.isBool())
        {
            return {};
        }

        return value.asBool();
    }

    std::optional<bool> GetRawBoolValueFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> jsonValue = GetJsonValueFromNode(node, keyName);

        if (jsonValue)
        {
            return GetRawBoolValueFromJsonValue(jsonValue.value().get());
        }

        return {};
    }

    std::optional<bool> GetRawBoolValueFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawBoolValueFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::optional<std::reference_wrapper<const Json::Value>> GetRawJsonArrayFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> jsonValue = GetJsonValueFromNode(node, keyName);

        if (!jsonValue || !jsonValue.value().get().isArray())
        {
            return {};
        }

        return jsonValue.value();
    }

    std::optional<std::reference_wrapper<const Json::Value>> GetRawJsonArrayFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawJsonArrayFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::vector<std::string> GetRawStringArrayFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> arrayValue = GetRawJsonArrayFromJsonNode(node, keyName);

        std::vector<std::string> result;
        if (!arrayValue)
        {
            return result;
        }

        for (auto const& value : arrayValue.value().get())
        {
            std::optional<std::string> item = GetRawStringValueFromJsonValue(value);
            if (item)
            {
                result.emplace_back(std::move(item.value()));
            }
        }

        return result;
    }

    std::vector<std::string> GetRawStringArrayFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawStringArrayFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::set<std::string> GetRawStringSetFromJsonNode(const Json::Value& node, std::string_view keyName)
    {
        std::optional<std::reference_wrapper<const Json::Value>> arrayValue = GetRawJsonArrayFromJsonNode(node, keyName);

        std::set<std::string> result;
        if (!arrayValue)
        {
            return result;
        }

        for (auto const& value : arrayValue.value().get())
        {
            std::optional<std::string> item = GetRawStringValueFromJsonValue(value);
            if (item)
            {
                result.emplace(std::move(item.value()));
            }
        }

        return result;
    }

    std::set<std::string> GetRawStringSetFromJsonNode(const Json::Value& node, std::wstring_view keyName)
    {
        return GetRawStringSetFromJsonNode(node, NormalizeJsonKey(keyName));
    }

    std::wstring GetUtilityString(std::string_view nodeName)
    {
        return Utility::ConvertToUTF16(nodeName);
    }

    Json::Value GetStringValue(std::string_view value)
    {
        return Json::Value(std::string{ value });
    }

    std::string Base64Encode(const std::vector<BYTE>& input)
    {
        if (input.size() == 0)
        {
            return {};
        }

        std::wstring result;
        DWORD resultSize = 0;
        CryptBinaryToStringW(input.data(), static_cast<DWORD>(input.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &resultSize);
        THROW_LAST_ERROR_IF(resultSize == 0);

        result.resize(resultSize);
        THROW_LAST_ERROR_IF(!CryptBinaryToStringW(input.data(), static_cast<DWORD>(input.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, result.data(), &resultSize));
        // Resize to remove trailing null terminator
        result.resize(resultSize);

        return Utility::ConvertToUTF8(result);
    }

    std::vector<BYTE> Base64Decode(const std::string& input)
    {
        if (input.empty())
        {
            return {};
        }

        auto inputWide = Utility::ConvertToUTF16(input);

        std::vector<BYTE> result;
        DWORD resultSize = 0;
        CryptStringToBinaryW(inputWide.data(), static_cast<DWORD>(inputWide.size()), CRYPT_STRING_BASE64, nullptr, &resultSize, nullptr, nullptr);
        THROW_LAST_ERROR_IF(resultSize == 0);

        result.resize(resultSize);
        THROW_LAST_ERROR_IF(!CryptStringToBinaryW(inputWide.data(), static_cast<DWORD>(inputWide.size()), CRYPT_STRING_BASE64, result.data(), &resultSize, nullptr, nullptr));

        return result;
    }
    bool IsValidNonEmptyStringValue(std::optional<std::string>& value)
    {
        if (Utility::IsEmptyOrWhitespace(value.value_or("")))
        {
            return false;
        }

        return true;
    }

    bool IsValidNonEmptyStringValue(std::optional<std::wstring>& value)
    {
        if (Utility::IsEmptyOrWhitespace(value.value_or(L"")))
        {
            return false;
        }

        return true;
    }
}
