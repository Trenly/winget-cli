// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "AppInstallerStrings.h"
#include "winget/Rest.h"
#include <winget/WebRequest.h>
#include <winhttp.h>

namespace AppInstaller::Rest
{
    bool HasQueryString(const std::wstring& uri)
    {
        return uri.find(Utility::Http::Uri::QueryParamMarker) != std::wstring::npos;
    }

    std::wstring GetRestAPIBaseUri(std::string uri)
    {
        // Trim
        if (!uri.empty())
        {
            uri = AppInstaller::Utility::Trim(uri);

            // Remove trailing forward slash
            if (uri.back() == '/')
            {
                uri.pop_back();
            }
        }

        return Utility::Http::EscapeUri(Utility::ConvertToUTF16(uri));
    }

    bool IsValidUri(const std::wstring& restApiUri)
    {
        if (restApiUri.find_first_of(L" \t\r\n") != std::wstring::npos)
        {
            return false;
        }

        URL_COMPONENTS uriComponents{};
        uriComponents.dwStructSize = sizeof(uriComponents);
        uriComponents.dwHostNameLength = static_cast<DWORD>(-1);
        uriComponents.dwUrlPathLength = static_cast<DWORD>(-1);
        uriComponents.dwExtraInfoLength = static_cast<DWORD>(-1);
        return WinHttpCrackUrl(restApiUri.c_str(), 0, 0, &uriComponents) != FALSE;
    }

    std::wstring AppendPathToUri(const std::wstring& restApiUri, const std::wstring& path)
    {
        std::wstring result = restApiUri;

        if (result.empty() || result.back() != Utility::Http::Uri::PathDelimiter)
        {
            result += Utility::Http::Uri::PathDelimiter;
        }

        std::wstring cleanPath = path;
        while (!cleanPath.empty() && cleanPath.front() == Utility::Http::Uri::PathDelimiter)
        {
            cleanPath.erase(cleanPath.begin());
        }

        return result + Utility::Http::EncodeUriComponent(cleanPath);
    }

    std::wstring MakeQueryParam(std::string_view queryName, const std::string& queryValue)
    {
        std::string queryParam;
        queryParam.append(queryName).append("=").append(queryValue);

        return Utility::ConvertToUTF16(queryParam);
    }

    std::wstring AppendQueryParamsToUri(const std::wstring& uri, const std::map<std::string_view, std::string>& queryParameters)
    {
        if (queryParameters.empty())
        {
            return uri;
        }

        std::wstring result = uri;
        result += HasQueryString(uri) ? Utility::Http::Uri::QueryParamDelimiter : Utility::Http::Uri::QueryParamMarker;

        bool first = true;
        for (const auto& pair : queryParameters)
        {
            if (!first)
            {
                result += Utility::Http::Uri::QueryParamDelimiter;
            }

            std::wstring queryName = Utility::Http::EncodeUriComponent(Utility::ConvertToUTF16(std::string{ pair.first }));
            std::wstring queryValue = Utility::Http::EncodeUriComponent(Utility::ConvertToUTF16(pair.second));
            result += queryName + L'=' + queryValue;
            first = false;
        }

        return result;
    }

    std::vector<std::string> GetUniqueItems(const std::vector<std::string>& list)
    {
        std::set<std::string> set;
        for (const auto& item : list)
        {
            set.emplace(item);
        }

        std::vector<std::string> result{ set.begin(), set.end() };
        return result;
    }
}
