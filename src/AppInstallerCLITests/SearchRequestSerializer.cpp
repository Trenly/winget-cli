// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include "TestRestRequestHandler.h"
#include <AppInstallerErrors.h>
#include <Rest/Schema/1_0/Json/SearchRequestSerializer.h>
#include <Rest/Schema/1_1/Json/SearchRequestSerializer.h>

using namespace TestCommon;
using namespace AppInstaller::Repository;
using namespace AppInstaller::Repository::Rest::Schema;

TEST_CASE("SearchRequestSerializer_InclusionsFilters", "[RestSource]")
{
    SearchRequest searchRequest;
    searchRequest.Inclusions.emplace_back(PackageMatchFilter(PackageMatchField::Id, MatchType::Substring, "Foo.Bar"));
    searchRequest.Inclusions.emplace_back(PackageMatchFilter(PackageMatchField::Name, MatchType::Substring, "Foo"));
    searchRequest.Filters.emplace_back(PackageMatchFilter(PackageMatchField::Moniker, MatchType::Exact, "FooBar"));
    searchRequest.MaximumResults = 10;

    V1_0::Json::SearchRequestSerializer serializer;
    Json::Value actual = serializer.Serialize(searchRequest);

    REQUIRE(!actual.isNull());
    REQUIRE(!actual.isMember("FetchAllManifests"));
    REQUIRE(actual["MaximumResults"].asInt() == static_cast<int>(searchRequest.MaximumResults));

    // Inclusions
    const auto& inclusions = actual["Inclusions"];
    REQUIRE(inclusions.size() == 2);
    REQUIRE(inclusions[0]["PackageMatchField"].asString() == "PackageIdentifier");
    REQUIRE(inclusions[1]["PackageMatchField"].asString() == "PackageName");
    const auto& requestMatch = inclusions[0]["RequestMatch"];
    REQUIRE(!requestMatch.isNull());
    REQUIRE(requestMatch["KeyWord"].asString() == "Foo.Bar");
    REQUIRE(requestMatch["MatchType"].asString() == "Substring");

    // Filters
    const auto& filters = actual["Filters"];
    REQUIRE(filters.size() == 1);
    REQUIRE(filters[0]["PackageMatchField"].asString() == "Moniker");
    const auto& requestMatchFilter = filters[0]["RequestMatch"];
    REQUIRE(!requestMatchFilter.isNull());
    REQUIRE(requestMatchFilter["KeyWord"].asString() == "FooBar");
    REQUIRE(requestMatchFilter["MatchType"].asString() == "Exact");
}

TEST_CASE("SearchRequestSerializer_Query", "[RestSource]")
{
    SearchRequest searchRequest;
    searchRequest.Query = RequestMatch(MatchType::Substring, "Foo.Bar");

    V1_0::Json::SearchRequestSerializer serializer;
    Json::Value actual = serializer.Serialize(std::move(searchRequest));

    REQUIRE(!actual.isNull());
    const auto& query = actual["Query"];
    REQUIRE(query["KeyWord"].asString() == "Foo.Bar");
    REQUIRE(query["MatchType"].asString() == "Substring");
}

TEST_CASE("SearchRequestSerializer_FetchAllManifests", "[RestSource]")
{
    V1_0::Json::SearchRequestSerializer serializer;
    Json::Value actual = serializer.Serialize({});

    REQUIRE(!actual.isNull());
    REQUIRE(actual["FetchAllManifests"].asBool());
}

TEST_CASE("SearchRequestSerializer_NewFields", "[RestSource]")
{
    SearchRequest searchRequest;
    searchRequest.Inclusions.emplace_back(PackageMatchFilter(PackageMatchField::Id, MatchType::Substring, "Foo.Bar"));
    searchRequest.Inclusions.emplace_back(PackageMatchFilter(PackageMatchField::Name, MatchType::Substring, "Foo"));
    searchRequest.Filters.emplace_back(PackageMatchFilter(PackageMatchField::Market, MatchType::Exact, "FooBar"));

    V1_0::Json::SearchRequestSerializer serializerV1_0;
    Json::Value actual_1_0 = serializerV1_0.Serialize(searchRequest);
    REQUIRE(!actual_1_0.isNull());
    REQUIRE(actual_1_0["Filters"].size() == 0);

    V1_1::Json::SearchRequestSerializer serializerV1_1;
    Json::Value actual_1_1 = serializerV1_1.Serialize(searchRequest);
    REQUIRE(!actual_1_1.isNull());
    REQUIRE(actual_1_1["Filters"].size() == 1);
}
