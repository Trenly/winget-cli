// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <string>
#include <string_view>

namespace AppInstaller::Utility::Http
{
    namespace Uri
    {
        inline constexpr wchar_t PathDelimiter = L'/';
        inline constexpr wchar_t QueryParamMarker = L'?';
        inline constexpr wchar_t QueryParamDelimiter = L'&';
    }

    namespace Header
    {
        inline constexpr std::wstring_view Authorization = L"Authorization";
        inline constexpr std::wstring_view CacheControl = L"Cache-Control";
        inline constexpr std::wstring_view ContentType = L"Content-Type";
        inline constexpr std::wstring_view RetryAfter = L"Retry-After";
        inline constexpr std::wstring_view UserAgent = L"User-Agent";
    }

    namespace MimeType
    {
        inline constexpr std::wstring_view ApplicationJson = L"application/json";
        inline constexpr std::wstring_view TextPlain = L"text/plain";
    }

    std::wstring EscapeUri(std::wstring_view uri);

    std::wstring EncodeUriComponent(std::wstring_view value);
}
