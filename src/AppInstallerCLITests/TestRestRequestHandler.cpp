// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include "TestRestRequestHandler.h"
#include <AppInstallerStrings.h>
#include <winget/JsonUtil.h>

using namespace AppInstaller::Rest;

namespace
{
    bool ContainsHeader(const HttpHeaders& headers, const std::wstring& name, const std::wstring& value)
    {
        for (const auto& header : headers)
        {
            if (AppInstaller::Utility::CaseInsensitiveEquals(header.first, name) &&
                AppInstaller::Utility::CaseInsensitiveEquals(header.second, value))
            {
                return true;
            }
        }

        return false;
    }
}

TestRestRequestHandler::TestRestRequestHandler(Handler handler) : m_handler(std::move(handler))
{
}

TestRestRequestHandler::Response TestRestRequestHandler::Send(const Request& request) const
{
    return m_handler(request);
}

TestRestRequestHandler::Response TestRestRequestHandler::Get(const std::wstring& uri, const HttpHeaders& headers, const HttpHeaders& authHeaders) const
{
    Request request;
    request.Method = winrt::Windows::Web::Http::HttpMethod::Get().Method();
    request.Uri = uri;
    request.Headers = headers;
    request.AuthHeaders = authHeaders;
    return Send(request);
}

TestRestRequestHandler::Response TestRestRequestHandler::Post(const std::wstring& uri, const ::Json::Value& body, const HttpHeaders& headers, const HttpHeaders& authHeaders) const
{
    Request request;
    request.Method = winrt::Windows::Web::Http::HttpMethod::Post().Method();
    request.Uri = uri;
    request.Headers = headers;
    request.AuthHeaders = authHeaders;
    request.Body = AppInstaller::Rest::Json::Serialize(body);
    return Send(request);
}

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    const uint32_t statusCode, const std::wstring& sampleResponseString, const std::wstring& mimeType)
{
    return std::make_shared<TestRestRequestHandler>(
        [statusCode, sampleResponseString, mimeType](const Request&) -> Response
        {
            Response response;
            response.StatusCode = statusCode;
            if (!sampleResponseString.empty())
            {
                response.Body = AppInstaller::Utility::ConvertToUTF8(sampleResponseString);
            }

            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::ContentType }, mimeType);
            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::CacheControl }, L"no-store");
            return response;
        });
}

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    std::function<uint32_t(const Request& request)> handler)
{
    return std::make_shared<TestRestRequestHandler>(
        [handler = std::move(handler)](const Request& request) -> Response
        {
            Response response;
            response.StatusCode = handler(request);
            response.Body = "{}";
            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::ContentType }, std::wstring{ AppInstaller::Utility::Http::MimeType::ApplicationJson });
            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::CacheControl }, L"no-store");
            return response;
        });
}

std::shared_ptr<TestRestRequestHandler> GetHeaderVerificationHandler(
    const uint32_t statusCode, const std::wstring& sampleResponseString, const std::pair<std::wstring, std::wstring>& header, uint32_t statusCodeOnFailure)
{
    return std::make_shared<TestRestRequestHandler>(
        [statusCode, sampleResponseString, header, statusCodeOnFailure](const Request& request) -> Response
        {
            Response response;

            if ((!ContainsHeader(request.Headers, header.first, header.second)) &&
                (!ContainsHeader(request.AuthHeaders, header.first, header.second)))
            {
                response.Body = "Expected header not found";
                response.StatusCode = statusCodeOnFailure;
                return response;
            }

            if (!sampleResponseString.empty())
            {
                response.Body = AppInstaller::Utility::ConvertToUTF8(sampleResponseString);
            }

            response.StatusCode = statusCode;
            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::ContentType }, std::wstring{ AppInstaller::Utility::Http::MimeType::ApplicationJson });
            response.Headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::CacheControl }, L"no-store");
            return response;
        });
}
