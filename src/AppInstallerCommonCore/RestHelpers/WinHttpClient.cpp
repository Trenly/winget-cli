// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "IHttpClient.h"

#include <AppInstallerErrors.h>
#include <AppInstallerRuntime.h>
#include <AppInstallerStrings.h>
#include <winget/WebRequest.h>
#include <winget/NetworkSettings.h>
#include <winhttp.h>

namespace AppInstaller::Rest
{
    using unique_winhttp_hinternet = wil::unique_any<HINTERNET, decltype(&::WinHttpCloseHandle), ::WinHttpCloseHandle>;

    namespace
    {
        bool ContainsHeader(const HttpHeaders& headers, std::wstring headerName)
        {
            for (const auto& header : headers)
            {
                if (Utility::CaseInsensitiveEquals(header.first, headerName))
                {
                    return true;
                }
            }

            return false;
        }

        void AddHeader(HINTERNET requestHandle, const std::wstring& name, const std::wstring& value)
        {
            std::wstring headerLine = name + L": " + value;
            THROW_LAST_ERROR_IF(!WinHttpAddRequestHeaders(
                requestHandle,
                headerLine.c_str(),
                static_cast<DWORD>(headerLine.length()),
                WINHTTP_ADDREQ_FLAG_ADD));
        }

        std::wstring GetHeader(HINTERNET requestHandle, DWORD headerInfoLevel)
        {
            DWORD size = 0;

            if (!WinHttpQueryHeaders(requestHandle, headerInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &size, WINHTTP_NO_HEADER_INDEX))
            {
                DWORD error = GetLastError();
                if (error == ERROR_WINHTTP_HEADER_NOT_FOUND)
                {
                    return {};
                }
                else if (error != ERROR_INSUFFICIENT_BUFFER)
                {
                    THROW_HR(HRESULT_FROM_WIN32(error));
                }

                THROW_HR_IF(HRESULT_FROM_WIN32(error), size == 0);
            }

            std::vector<wchar_t> value((size / sizeof(wchar_t)) + 1, L'\0');
            THROW_LAST_ERROR_IF(!WinHttpQueryHeaders(requestHandle, headerInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, value.data(), &size, WINHTTP_NO_HEADER_INDEX));
            return std::wstring{ value.data() };
        }

        HttpHeaders ParseRawHeaders(HINTERNET requestHandle)
        {
            HttpHeaders result;
            std::wstring rawHeaders = GetHeader(requestHandle, WINHTTP_QUERY_RAW_HEADERS_CRLF);
            std::wistringstream rawHeadersStream(rawHeaders);
            std::wstring line;

            while (std::getline(rawHeadersStream, line))
            {
                if (!line.empty() && line.back() == L'\r')
                {
                    line.pop_back();
                }

                auto separator = line.find(L':');
                if (separator != std::wstring::npos)
                {
                    std::wstring key = line.substr(0, separator);
                    std::wstring value = Utility::Trim(line.substr(separator + 1));
                    result.emplace(std::move(key), std::move(value));
                }
            }

            return result;
        }

        std::string ReadResponseBody(HINTERNET requestHandle)
        {
            std::string result;
            while (true)
            {
                DWORD bytesAvailable = 0;
                THROW_LAST_ERROR_IF(!WinHttpQueryDataAvailable(requestHandle, &bytesAvailable));
                if (bytesAvailable == 0)
                {
                    break;
                }

                std::string buffer(bytesAvailable, '\0');
                DWORD bytesRead = 0;
                THROW_LAST_ERROR_IF(!WinHttpReadData(requestHandle, buffer.data(), bytesAvailable, &bytesRead));
                buffer.resize(bytesRead);
                result.append(buffer);
            }

            return result;
        }

        void SetMethodHeaders(Request& request, const std::wstring& method)
        {
            request.Method = method;
            if (!ContainsHeader(request.Headers, std::wstring{ Utility::Http::Header::ContentType }))
            {
                request.Headers.emplace(std::wstring{ Utility::Http::Header::ContentType }, std::wstring{ Utility::Http::MimeType::ApplicationJson });
            }
        }
    }

    WinHttpClient::WinHttpClient(
        std::optional<Certificates::PinningConfiguration> pinningConfiguration,
        std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals) :
        m_pinningConfiguration(std::move(pinningConfiguration)),
        m_threadGlobals(std::move(threadGlobals))
    {
    }

    Response WinHttpClient::Send(const Request& request) const
    {
        URL_COMPONENTS uriComponents{};
        uriComponents.dwStructSize = sizeof(uriComponents);
        uriComponents.dwHostNameLength = static_cast<DWORD>(-1);
        uriComponents.dwUrlPathLength = static_cast<DWORD>(-1);
        uriComponents.dwExtraInfoLength = static_cast<DWORD>(-1);

        THROW_HR_IF(APPINSTALLER_CLI_ERROR_INVALID_CL_ARGUMENTS,
            !WinHttpCrackUrl(request.Uri.c_str(), 0, 0, &uriComponents));

        std::wstring host =
            (uriComponents.lpszHostName && uriComponents.dwHostNameLength > 0) ?
            std::wstring{ uriComponents.lpszHostName, uriComponents.dwHostNameLength } :
            std::wstring{};

        std::wstring path =
            (uriComponents.lpszUrlPath && uriComponents.dwUrlPathLength > 0) ?
            std::wstring{ uriComponents.lpszUrlPath, uriComponents.dwUrlPathLength } :
            std::wstring{};
        if (uriComponents.dwExtraInfoLength > 0)
        {
            if (uriComponents.lpszExtraInfo)
            {
                path += std::wstring{ uriComponents.lpszExtraInfo, uriComponents.dwExtraInfoLength };
            }
        }
        if (path.empty())
        {
            path = L"/";
        }

        const auto& proxyUri = Settings::Network().GetProxyUri();
        DWORD accessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
        std::wstring proxyUriWide;
        if (proxyUri)
        {
            proxyUriWide = Utility::ConvertToUTF16(proxyUri.value());
            accessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            AICLI_LOG(Repo, Info, << "Setting proxy for REST helper WinHTTP client to " << proxyUri.value());
        }
        else
        {
            AICLI_LOG(Repo, Info, << "REST helper WinHTTP client does not use proxy");
        }

        unique_winhttp_hinternet session{ WinHttpOpen(
            nullptr,
            accessType,
            proxyUri ? proxyUriWide.c_str() : WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0) };
        THROW_LAST_ERROR_IF(!session);

        unique_winhttp_hinternet connect{ WinHttpConnect(session.get(), host.c_str(), uriComponents.nPort, 0) };
        THROW_LAST_ERROR_IF(!connect);

        DWORD flags = uriComponents.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
        unique_winhttp_hinternet httpRequest{ WinHttpOpenRequest(
            connect.get(),
            request.Method.c_str(),
            path.c_str(),
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            flags) };
        THROW_LAST_ERROR_IF(!httpRequest);

        for (const auto& header : request.Headers)
        {
            AddHeader(httpRequest.get(), header.first, header.second);
        }

        if (!ContainsHeader(request.Headers, std::wstring{ Utility::Http::Header::UserAgent }))
        {
            AddHeader(httpRequest.get(), std::wstring{ Utility::Http::Header::UserAgent }, Utility::ConvertToUTF16(AppInstaller::Runtime::GetDefaultUserAgent()));
        }

        std::ostringstream logStream;
        logStream << Utility::ConvertToUTF8(request.Method) << " " << Utility::ConvertToUTF8(request.Uri) << "\n";
        for (const auto& header : request.Headers)
        {
            logStream << Utility::ConvertToUTF8(header.first) << ": " << Utility::ConvertToUTF8(header.second) << "\n";
        }
        AICLI_LOG(Repo, Verbose, << "Http request details:\n" << logStream.str());

        for (const auto& header : request.AuthHeaders)
        {
            AddHeader(httpRequest.get(), header.first, header.second);
        }

        const std::string& body = request.Body.value_or(std::string{});
        DWORD bodySize = static_cast<DWORD>(body.size());
        THROW_LAST_ERROR_IF(!WinHttpSendRequest(
            httpRequest.get(),
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            bodySize ? const_cast<char*>(body.data()) : WINHTTP_NO_REQUEST_DATA,
            bodySize,
            bodySize,
            0));

        THROW_LAST_ERROR_IF(!WinHttpReceiveResponse(httpRequest.get(), nullptr));

        if (m_pinningConfiguration)
        {
            auto previousThreadGlobals = m_threadGlobals ? m_threadGlobals->SetForCurrentThread() : nullptr;
            UNREFERENCED_PARAMETER(previousThreadGlobals);

            wil::unique_cert_context certContext;
            DWORD bufferSize = sizeof(PCCERT_CONTEXT);
            THROW_IF_WIN32_BOOL_FALSE(WinHttpQueryOption(httpRequest.get(), WINHTTP_OPTION_SERVER_CERT_CONTEXT, &certContext, &bufferSize));
            THROW_HR_IF(APPINSTALLER_CLI_ERROR_PINNED_CERTIFICATE_MISMATCH, !m_pinningConfiguration->Validate(certContext.get()));
        }

        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        THROW_LAST_ERROR_IF(!WinHttpQueryHeaders(httpRequest.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX));

        Response response;
        response.StatusCode = statusCode;
        response.Headers = ParseRawHeaders(httpRequest.get());
        response.Body = ReadResponseBody(httpRequest.get());

        AICLI_LOG(Repo, Info, << "Response status: " << response.StatusCode);
        std::ostringstream responseLog;
        responseLog << "Status: " << response.StatusCode << "\n";
        for (const auto& header : response.Headers)
        {
            responseLog << Utility::ConvertToUTF8(header.first) << ": " << Utility::ConvertToUTF8(header.second) << "\n";
        }
        responseLog << "\n" << response.Body;
        AICLI_LOG_LARGE_STRING(Repo, Verbose, << "Response details:",
            responseLog.str());

        return response;
    }

    Response WinHttpClient::Get(const HttpString& uri, const HttpHeaders& headers, const HttpHeaders& authHeaders) const
    {
        AICLI_LOG(Repo, Info, << "Sending http GET request to: " << Utility::ConvertToUTF8(uri));

        Request request;
        request.Uri = uri;
        request.Headers = headers;
        request.AuthHeaders = authHeaders;
        SetMethodHeaders(request, L"GET");
        return Send(request);
    }

    Response WinHttpClient::Post(const HttpString& uri, const ::Json::Value& body, const HttpHeaders& headers, const HttpHeaders& authHeaders) const
    {
        AICLI_LOG(Repo, Info, << "Sending http POST request to: " << Utility::ConvertToUTF8(uri));

        Request request;
        request.Method = L"POST";
        request.Uri = uri;
        request.Headers = headers;
        request.AuthHeaders = authHeaders;
        SetMethodHeaders(request, L"POST");
        request.Body = Json::Serialize(body);
        return Send(request);
    }

    void WinHttpClient::SetPinningConfiguration(
        const Certificates::PinningConfiguration& pinningConfiguration,
        std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals)
    {
        m_pinningConfiguration = pinningConfiguration;
        m_threadGlobals = std::move(threadGlobals);
    }

    std::unique_ptr<IHttpClient> CreateWinHttpClient(
        std::optional<Certificates::PinningConfiguration> pinningConfiguration,
        std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals)
    {
        return std::make_unique<WinHttpClient>(std::move(pinningConfiguration), std::move(threadGlobals));
    }
}
