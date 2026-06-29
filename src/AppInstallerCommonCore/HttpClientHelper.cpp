// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include <AppInstallerDownloader.h>
#include <AppInstallerRuntime.h>
#include <AppInstallerStrings.h>
#include <winget/HttpClientHelper.h>
#include <winget/WebRequest.h>
#include <winget/RestHelpers.h>

namespace AppInstaller::Http
{
    namespace
    {
        Rest::HttpHeaders ToRestHeaders(const HttpClientHelper::HttpRequestHeaders& headers)
        {
            Rest::HttpHeaders result;
            for (const auto& header : headers)
            {
                result.emplace(header.first, header.second);
            }

            return result;
        }

        // If the caller does not pass in a user agent header, put the default one on the request.
        void EnsureDefaultUserAgent(Rest::Request& request)
        {
            bool userAgentHeaderPresent = false;
            for (const auto& header : request.Headers)
            {
                if (Utility::CaseInsensitiveEquals(header.first, std::wstring{ Utility::Http::Header::UserAgent }))
                {
                    userAgentHeaderPresent = true;
                    break;
                }
            }

            if (!userAgentHeaderPresent)
            {
                static std::wstring c_defaultUserAgent = Utility::ConvertToUTF16(AppInstaller::Runtime::GetDefaultUserAgent());
                request.Headers.emplace(std::wstring{ Utility::Http::Header::UserAgent }, c_defaultUserAgent);
            }
        }
    }

    HttpClientHelper::HttpClientHelper()
        : m_restHttpClient(Rest::CreateWinHttpClient())
    {}

    HttpClientHelper::HttpClientHelper(std::shared_ptr<Rest::IHttpClient> restHttpClient)
        : m_restHttpClient(std::move(restHttpClient))
    {
        if (!m_restHttpClient)
        {
            m_restHttpClient = Rest::CreateWinHttpClient();
        }
    }

    Rest::Response HttpClientHelper::Post(
        const std::wstring& uri,
        const ::Json::Value& body,
        const HttpClientHelper::HttpRequestHeaders& headers,
        const HttpClientHelper::HttpRequestHeaders& authHeaders) const
    {
        AICLI_LOG(Repo, Info, << "Sending http POST request to: " << Utility::ConvertToUTF8(uri));
        Rest::Request request;
        request.Method = L"POST";
        request.Uri = uri;
        request.Headers = ToRestHeaders(headers);
        request.AuthHeaders = ToRestHeaders(authHeaders);
        request.Body = Rest::Json::Serialize(body);
        EnsureDefaultUserAgent(request);
        return m_restHttpClient->Send(request);
    }

    std::optional<::Json::Value> HttpClientHelper::HandlePost(
        const std::wstring& uri,
        const ::Json::Value& body,
        const HttpClientHelper::HttpRequestHeaders& headers,
        const HttpClientHelper::HttpRequestHeaders& authHeaders,
        const HttpResponseHandler& customHandler) const
    {
        Rest::Response httpResponse = Post(uri, body, headers, authHeaders);

        if (customHandler)
        {
            auto handlerResult = customHandler(httpResponse);
            if (!handlerResult.UseDefaultHandling)
            {
                return std::move(handlerResult.Result);
            }
        }

        return ValidateAndExtractResponse(httpResponse);
    }

    Rest::Response HttpClientHelper::Get(
        const std::wstring& uri,
        const HttpClientHelper::HttpRequestHeaders& headers,
        const HttpClientHelper::HttpRequestHeaders& authHeaders) const
    {
        AICLI_LOG(Repo, Info, << "Sending http GET request to: " << Utility::ConvertToUTF8(uri));
        Rest::Request request;
        request.Method = L"GET";
        request.Uri = uri;
        request.Headers = ToRestHeaders(headers);
        request.AuthHeaders = ToRestHeaders(authHeaders);
        EnsureDefaultUserAgent(request);
        return m_restHttpClient->Send(request);
    }

    std::optional<::Json::Value> HttpClientHelper::HandleGet(
        const std::wstring& uri,
        const HttpClientHelper::HttpRequestHeaders& headers,
        const HttpClientHelper::HttpRequestHeaders& authHeaders,
        const HttpResponseHandler& customHandler) const
    {
        Rest::Response httpResponse = Get(uri, headers, authHeaders);

        if (customHandler)
        {
            auto handlerResult = customHandler(httpResponse);
            if (!handlerResult.UseDefaultHandling)
            {
                return std::move(handlerResult.Result);
            }
        }

        return ValidateAndExtractResponse(httpResponse);
    }

    void HttpClientHelper::SetPinningConfiguration(const Certificates::PinningConfiguration& configuration, std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals)
    {
        if (!m_restHttpClient)
        {
            m_restHttpClient = Rest::CreateWinHttpClient();
        }

        m_restHttpClient->SetPinningConfiguration(configuration, std::move(threadGlobals));
    }

    std::optional<::Json::Value> HttpClientHelper::ValidateAndExtractResponse(const Rest::Response& response) const
    {
        return Rest::ValidateAndExtractJsonResponse(response);
    }

}
