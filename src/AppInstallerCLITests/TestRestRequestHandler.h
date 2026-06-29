// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <winhttp.h>
#include <winget/WebRequest.h>
#include <winget/RestHelpers.h>

struct TestRestRequestHandler : public AppInstaller::Rest::IHttpClient
{
    using Request = AppInstaller::Rest::Request;
    using Response = AppInstaller::Rest::Response;
    using Handler = std::function<Response(const Request&)>;

    TestRestRequestHandler(Handler handler);

    Response Send(const Request& request) const override;
    Response Get(const std::wstring& uri, const AppInstaller::Rest::HttpHeaders& headers = {}, const AppInstaller::Rest::HttpHeaders& authHeaders = {}) const override;
    Response Post(const std::wstring& uri, const ::Json::Value& body, const AppInstaller::Rest::HttpHeaders& headers = {}, const AppInstaller::Rest::HttpHeaders& authHeaders = {}) const override;

private:
    Handler m_handler;
};

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    const uint32_t statusCode, const std::wstring& sampleResponseString = {}, const std::wstring& mimeType = std::wstring{ AppInstaller::Utility::Http::MimeType::ApplicationJson });

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    std::function<uint32_t(const AppInstaller::Rest::Request& request)> handler);

std::shared_ptr<TestRestRequestHandler> GetHeaderVerificationHandler(
    const uint32_t statusCode, const std::wstring& sampleResponseString, const std::pair<std::wstring, std::wstring>& header, uint32_t statusCodeOnFailure = HTTP_STATUS_BAD_REQUEST);
