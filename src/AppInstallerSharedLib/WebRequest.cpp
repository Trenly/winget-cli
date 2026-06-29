// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include <winget/WebRequest.h>
#include <Shlwapi.h>

namespace AppInstaller::Utility::Http
{
    namespace
    {
        constexpr DWORD EscapeFlags = URL_ESCAPE_AS_UTF8 | URL_ESCAPE_PERCENT;
        constexpr std::wstring_view AuthorityPrefix = L"//";
        constexpr std::wstring_view EscapedHostSpace = L"%2520";
        constexpr std::wstring_view EncodedSpace = L"%20";

        std::wstring EscapeWithUrlEscape(std::wstring_view value)
        {
            std::wstring valueBuffer{ value };
            DWORD escapedLength = static_cast<DWORD>(valueBuffer.size() + 1);
            std::wstring escapedValue(escapedLength, L'\0');

            HRESULT hr = UrlEscapeW(valueBuffer.c_str(), escapedValue.data(), &escapedLength, EscapeFlags);
            if (hr == E_POINTER)
            {
                escapedValue.resize(escapedLength);
                hr = UrlEscapeW(valueBuffer.c_str(), escapedValue.data(), &escapedLength, EscapeFlags);
            }

            THROW_IF_FAILED(hr);

            escapedValue.resize(escapedLength);
            return escapedValue;
        }

        std::wstring EscapeHostSpaces(std::wstring_view host)
        {
            std::wstring escapedHost;
            escapedHost.reserve(host.size());

            for (wchar_t ch : host)
            {
                if (ch == L' ')
                {
                    escapedHost += L"%20";
                }
                else
                {
                    escapedHost += ch;
                }
            }

            return escapedHost;
        }

        bool TryGetHostRange(std::wstring_view uri, size_t& hostStart, size_t& hostEnd)
        {
            const size_t authorityStartMarker = uri.find(AuthorityPrefix);
            if (authorityStartMarker == std::wstring_view::npos)
            {
                return false;
            }

            const size_t authorityStart = authorityStartMarker + AuthorityPrefix.size();
            size_t authorityEnd = uri.find_first_of(L"/?#", authorityStart);
            if (authorityEnd == std::wstring_view::npos)
            {
                authorityEnd = uri.size();
            }

            hostStart = authorityStart;

            // Skip optional userinfo (`user[:pass]@`) when identifying host boundaries.
            const size_t userInfoDelimiter = uri.rfind(L'@', authorityEnd - 1);
            if (userInfoDelimiter != std::wstring_view::npos && userInfoDelimiter >= authorityStart)
            {
                hostStart = userInfoDelimiter + 1;
            }

            hostEnd = authorityEnd;
            if (hostStart < authorityEnd)
            {
                if (uri[hostStart] == L'[')
                {
                    // IPv6 literal host: keep everything through closing bracket as host.
                    const size_t ipv6Delimiter = uri.find(L']', hostStart + 1);
                    if (ipv6Delimiter != std::wstring_view::npos && ipv6Delimiter < authorityEnd)
                    {
                        hostEnd = ipv6Delimiter + 1;
                    }
                }
                else
                {
                    // Non-IPv6 host: stop before optional `:port`.
                    const size_t portDelimiter = uri.find(L':', hostStart);
                    if (portDelimiter != std::wstring_view::npos && portDelimiter < authorityEnd)
                    {
                        hostEnd = portDelimiter;
                    }
                }
            }

            return hostStart < hostEnd;
        }

        std::pair<std::wstring, size_t> EscapeHostSpacesInUri(std::wstring_view uri)
        {
            size_t hostStart = 0;
            size_t hostEnd = 0;
            if (!TryGetHostRange(uri, hostStart, hostEnd))
            {
                return { std::wstring{ uri }, 0 };
            }

            std::wstring escapedUri{ uri };
            size_t hostSpaceCount = 0;
            for (size_t i = hostStart; i < hostEnd; ++i)
            {
                if (uri[i] == L' ')
                {
                    ++hostSpaceCount;
                }
            }

            if (hostSpaceCount == 0)
            {
                return { escapedUri, 0 };
            }

            escapedUri.replace(hostStart, hostEnd - hostStart, EscapeHostSpaces(uri.substr(hostStart, hostEnd - hostStart)));
            return { escapedUri, hostSpaceCount };
        }

        void NormalizeEscapedHostSpaces(std::wstring& escapedUri, size_t hostSpaceCount)
        {
            if (hostSpaceCount == 0)
            {
                return;
            }

            size_t hostStart = 0;
            size_t hostEnd = 0;
            if (!TryGetHostRange(escapedUri, hostStart, hostEnd))
            {
                return;
            }

            size_t replacements = 0;
            size_t searchPos = hostStart;
            while (replacements < hostSpaceCount && searchPos < hostEnd)
            {
                size_t escapedSpacePos = escapedUri.find(EscapedHostSpace, searchPos);
                if (escapedSpacePos == std::wstring::npos || escapedSpacePos >= hostEnd)
                {
                    break;
                }

                escapedUri.replace(escapedSpacePos, EscapedHostSpace.size(), EncodedSpace);
                hostEnd -= EscapedHostSpace.size() - EncodedSpace.size();
                searchPos = escapedSpacePos + EncodedSpace.size();
                ++replacements;
            }
        }
    }

    std::wstring EscapeUri(std::wstring_view uri)
    {
        // Progressive flow: first escape host spaces in the full URI, then apply UrlEscapeW to the full URI.
        auto [uriWithEscapedHost, hostSpaceCount] = EscapeHostSpacesInUri(uri);
        std::wstring escapedUri = EscapeWithUrlEscape(uriWithEscapedHost);

        // UrlEscapeW escapes `%` to `%25`; restore host `%20` sequences that were intentionally pre-escaped.
        NormalizeEscapedHostSpaces(escapedUri, hostSpaceCount);
        return escapedUri;
    }

    std::wstring EncodeUriComponent(std::wstring_view value)
    {
        return winrt::Windows::Foundation::Uri::EscapeComponent(value).c_str();
    }
}
