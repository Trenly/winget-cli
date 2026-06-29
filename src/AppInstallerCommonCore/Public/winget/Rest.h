// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace AppInstaller::Rest
{
    std::wstring GetRestAPIBaseUri(std::string restApiUri);

    bool IsValidUri(const std::wstring& restApiUri);

    std::wstring AppendPathToUri(const std::wstring& restApiUri, const std::wstring& path);

    std::wstring MakeQueryParam(std::string_view queryName, const std::string& queryValue);

    std::wstring AppendQueryParamsToUri(const std::wstring& uri, const std::map<std::string_view, std::string>& queryParameters);

    std::vector<std::string> GetUniqueItems(const std::vector<std::string>& list);
}
