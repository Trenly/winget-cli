// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <winget/Certificates.h>
#include <winget/RestHelpers.h>
#include <winget/SharedThreadGlobals.h>
#include <json/json.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AppInstaller::Http
{
    struct HttpClientHelper
    {
        using HttpRequestHeaders = std::unordered_map<std::wstring, std::wstring>;

        struct HttpResponseHandlerResult
        {
            // The custom response handler result. Default is empty.
            std::optional<::Json::Value> Result = std::nullopt;

            // Indicates whether to use default handling logic by HttpClientHelper instead (i.e. the custom response handler does not handle the specific response).
            bool UseDefaultHandling = false;
        };

        using HttpResponseHandler = std::function<HttpResponseHandlerResult(const Rest::Response&)>;

        HttpClientHelper();
        HttpClientHelper(std::shared_ptr<Rest::IHttpClient> restHttpClient);

        Rest::Response Post(const std::wstring& uri, const ::Json::Value& body, const HttpRequestHeaders& headers = {}, const HttpRequestHeaders& authHeaders = {}) const;

        std::optional<::Json::Value> HandlePost(const std::wstring& uri, const ::Json::Value& body, const HttpRequestHeaders& headers = {}, const HttpRequestHeaders& authHeaders = {}, const HttpResponseHandler& customHandler = {}) const;

        Rest::Response Get(const std::wstring& uri, const HttpRequestHeaders& headers = {}, const HttpRequestHeaders& authHeaders = {}) const;

        std::optional<::Json::Value> HandleGet(const std::wstring& uri, const HttpRequestHeaders& headers = {}, const HttpRequestHeaders& authHeaders = {}, const HttpResponseHandler& customHandler = {}) const;

        void SetPinningConfiguration(const Certificates::PinningConfiguration& configuration, std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals = {});

    protected:
        std::optional<::Json::Value> ValidateAndExtractResponse(const Rest::Response& response) const;

    private:
        std::shared_ptr<Rest::IHttpClient> m_restHttpClient;
    };
}
