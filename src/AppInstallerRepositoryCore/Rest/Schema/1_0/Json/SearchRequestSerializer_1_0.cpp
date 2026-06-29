// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "Rest/Schema/IRestClient.h"
#include "SearchRequestSerializer.h"
#include <winget/JsonUtil.h>
#include "Rest/Schema/CommonRestConstants.h"

namespace AppInstaller::Repository::Rest::Schema::V1_0::Json
{
    namespace
    {
        // Search request constants
        constexpr std::string_view Query = "Query"sv;
        constexpr std::string_view Filters = "Filters"sv;
        constexpr std::string_view Inclusions = "Inclusions"sv;
        constexpr std::string_view MaximumResults = "MaximumResults"sv;
        constexpr std::string_view RequestMatch = "RequestMatch"sv;
        constexpr std::string_view KeyWord = "KeyWord"sv;
        constexpr std::string_view MatchType = "MatchType"sv;
        constexpr std::string_view PackageMatchField = "PackageMatchField"sv;
        constexpr std::string_view FetchAllManifests = "FetchAllManifests"sv;

        std::optional<std::string_view> ConvertMatchTypeToString(AppInstaller::Repository::MatchType type)
        {
            // Match types supported by Rest API schema.
            switch (type)
            {
            case MatchType::Exact:
                return "Exact"sv;
            case MatchType::CaseInsensitive:
                return "CaseInsensitive"sv;
            case MatchType::StartsWith:
                return "StartsWith"sv;
            case MatchType::Substring:
                return "Substring"sv;
            case MatchType::Wildcard:
                return "Wildcard"sv;
            case MatchType::Fuzzy:
                return "Fuzzy"sv;
            case MatchType::FuzzySubstring:
                return "FuzzySubstring"sv;
            }

            return {};
        }
    }

    ::Json::Value SearchRequestSerializer::Serialize(const SearchRequest& searchRequest) const
    {
        std::optional<::Json::Value> result = SerializeSearchRequest(searchRequest);

        THROW_HR_IF(APPINSTALLER_CLI_ERROR_RESTAPI_INTERNAL_ERROR, !result);

        return result.value();
    }

    std::optional<::Json::Value> SearchRequestSerializer::SerializeSearchRequest(const SearchRequest& searchRequest) const
    {
        try
        {
            ::Json::Value json_body{ ::Json::objectValue };
            if (searchRequest.MaximumResults > 0)
            {
                json_body[std::string{ MaximumResults }] = searchRequest.MaximumResults;
            }

            if (searchRequest.IsForEverything())
            {
                json_body[std::string{ FetchAllManifests }] = true;
                return json_body;
            }

            if (searchRequest.Query)
            {
                auto& requestMatch = searchRequest.Query.value();
                std::optional<::Json::Value> requestMatchJson = GetRequestMatchJsonObject(requestMatch);
                if (requestMatchJson)
                {
                    json_body[std::string{ Query }] = std::move(requestMatchJson.value());
                }
            }

            if (!searchRequest.Filters.empty())
            {
                ::Json::Value filters{ ::Json::arrayValue };
                for (auto& filter : searchRequest.Filters)
                {
                    std::optional<::Json::Value> jsonObject = GetPackageMatchFilterJsonObject(filter);

                    if (jsonObject)
                    {
                        filters.append(std::move(jsonObject.value()));
                    }
                }

                json_body[std::string{ Filters }] = std::move(filters);
            }

            if (!searchRequest.Inclusions.empty())
            {
                ::Json::Value inclusions{ ::Json::arrayValue };
                for (auto& inclusion : searchRequest.Inclusions)
                {
                    std::optional<::Json::Value> jsonObject = GetPackageMatchFilterJsonObject(inclusion);

                    if (jsonObject)
                    {
                        inclusions.append(std::move(jsonObject.value()));
                    }
                }

                json_body[std::string{ Inclusions }] = std::move(inclusions);
            }

            return json_body;
        }
        catch (const std::exception& e)
        {
            AICLI_LOG(Repo, Error, << "Error occurred while serializing search request. Reason: " << e.what());
        }
        catch (...)
        {
            AICLI_LOG(Repo, Error, << "Error occurred while serializing search request");
        }

        return {};
    }

    std::optional<::Json::Value> SearchRequestSerializer::GetPackageMatchFilterJsonObject(const PackageMatchFilter& packageMatchFilter) const
    {
        ::Json::Value filter{ ::Json::objectValue };
        std::optional<std::string_view> matchField = ConvertPackageMatchFieldToString(packageMatchFilter.Field);

        if (!matchField)
        {
            AICLI_LOG(Repo, Warning, << "Skipping unsupported package match field: " << packageMatchFilter.Field);
            return {};
        }

        filter[std::string{ PackageMatchField }] = std::string{ matchField.value() };
        std::optional<::Json::Value> requestMatchJson = GetRequestMatchJsonObject(packageMatchFilter);

        if (!requestMatchJson)
        {
            AICLI_LOG(Repo, Warning, << "Skipping unsupported request match object.");
            return {};
        }

        filter[std::string{ RequestMatch }] = std::move(requestMatchJson.value());
        return filter;
    }

    std::optional<::Json::Value> SearchRequestSerializer::GetRequestMatchJsonObject(const AppInstaller::Repository::RequestMatch& requestMatch) const
    {
        ::Json::Value match{ ::Json::objectValue };
        match[std::string{ KeyWord }] = requestMatch.Value;

        std::optional<std::string_view> matchType = ConvertMatchTypeToString(requestMatch.Type);
        if (!matchType)
        {
            AICLI_LOG(Repo, Warning, << "Skipping unsupported match type: " << requestMatch.Type);
            return {};
        }

        match[std::string{ MatchType }] = std::string{ matchType.value() };
        return match;
    }

    std::optional<std::string_view> SearchRequestSerializer::ConvertPackageMatchFieldToString(AppInstaller::Repository::PackageMatchField field) const
    {
        // Match fields supported by Rest API schema.
        switch (field)
        {
        case PackageMatchField::Command:
            return "Command"sv;
        case PackageMatchField::Id:
            return "PackageIdentifier"sv;
        case PackageMatchField::Moniker:
            return "Moniker"sv;
        case PackageMatchField::Name:
            return "PackageName"sv;
        case PackageMatchField::Tag:
            return "Tag"sv;
        case PackageMatchField::PackageFamilyName:
            return "PackageFamilyName"sv;
        case PackageMatchField::ProductCode:
            return "ProductCode"sv;
        case PackageMatchField::NormalizedNameAndPublisher:
            return "NormalizedPackageNameAndPublisher"sv;
        }

        return {};
    }
}
