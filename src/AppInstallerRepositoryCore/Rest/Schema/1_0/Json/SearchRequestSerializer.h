// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <json/json.h>
#include "Rest/Schema/IRestClient.h"

namespace AppInstaller::Repository::Rest::Schema::V1_0::Json
{
    // Search Result Serializer.
    struct SearchRequestSerializer
    {
        ::Json::Value Serialize(const SearchRequest& searchRequest) const;

    protected:
        std::optional<::Json::Value> SerializeSearchRequest(const SearchRequest& searchRequest) const;

        std::optional<::Json::Value> GetRequestMatchJsonObject(const AppInstaller::Repository::RequestMatch& requestMatch) const;

        std::optional<::Json::Value> GetPackageMatchFilterJsonObject(const PackageMatchFilter& packageMatchFilter) const;

        virtual std::optional<std::string_view> ConvertPackageMatchFieldToString(AppInstaller::Repository::PackageMatchField field) const;
    };
}
