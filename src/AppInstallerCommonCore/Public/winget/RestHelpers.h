// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <json/json.h>
#include <winget/Certificates.h>
#include <winget/Rest.h>
#include <winget/SharedThreadGlobals.h>

#include <memory>
#include <optional>

namespace AppInstaller::Rest
{
    using HttpString = std::wstring;
    using HttpHeaders = std::multimap<HttpString, HttpString>;

    struct Request
    {
        HttpString Method;
        HttpString Uri;
        HttpHeaders Headers;
        HttpHeaders AuthHeaders;
        std::optional<std::string> Body;
    };

    struct Response
    {
        uint32_t StatusCode = 0;
        HttpHeaders Headers;
        std::string Body;
    };

    struct IHttpClient
    {
        virtual ~IHttpClient() = default;

        virtual Response Send(const Request& request) const = 0;

        virtual Response Get(const HttpString& uri, const HttpHeaders& headers = {}, const HttpHeaders& authHeaders = {}) const = 0;

        virtual Response Post(const HttpString& uri, const ::Json::Value& body, const HttpHeaders& headers = {}, const HttpHeaders& authHeaders = {}) const = 0;

        virtual void SetPinningConfiguration(
            const Certificates::PinningConfiguration&,
            std::shared_ptr<ThreadLocalStorage::ThreadGlobals>) {}
    };

    std::unique_ptr<IHttpClient> CreateWinHttpClient(
        std::optional<Certificates::PinningConfiguration> pinningConfiguration = {},
        std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals = {});

    std::wstring GetHeaderValue(const HttpHeaders& headers, std::wstring_view headerName);

    namespace Json
    {
        ::Json::Value Parse(std::string_view content);
        ::Json::Value Parse(std::wstring_view content);
        std::string Serialize(const ::Json::Value& value);

        const ::Json::Value* GetOptionalValue(const ::Json::Value& value, std::string_view fieldName);
        const ::Json::Value& GetRequiredValue(const ::Json::Value& value, std::string_view fieldName);

        std::optional<std::string> GetStringValue(const ::Json::Value& value);
        std::optional<int> GetIntValue(const ::Json::Value& value);
        std::optional<uint64_t> GetUInt64Value(const ::Json::Value& value);
        std::optional<bool> GetBoolValue(const ::Json::Value& value);
        std::vector<std::string> GetStringArray(const ::Json::Value& value);
    }

    std::optional<::Json::Value> ValidateAndExtractJsonResponse(const Response& response);
}
