// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "Rest/Schema/1_0/Json/ManifestDeserializer.h"

namespace AppInstaller::Repository::Rest::Schema::V1_1::Json
{
    // Manifest Deserializer.
    struct ManifestDeserializer : public V1_0::Json::ManifestDeserializer
    {
        std::vector<Manifest::AppsAndFeaturesEntry> DeserializeAppsAndFeaturesEntries(const ::Json::Value& entries) const override;

        std::optional<Manifest::ManifestLocalization> DeserializeLocale(const ::Json::Value& localeJsonObject) const override;

    protected:
        std::optional<Manifest::ManifestInstaller> DeserializeInstaller(const ::Json::Value& installerJsonObject) const override;

        Manifest::InstallerTypeEnum ConvertToInstallerType(std::string_view in) const override;

        virtual Manifest::ExpectedReturnCodeEnum ConvertToExpectedReturnCodeEnum(std::string_view in) const;

        virtual Manifest::ManifestInstaller::ExpectedReturnCodeInfo DeserializeExpectedReturnCodeInfo(const ::Json::Value& expectedReturnCodeJsonObject) const;

        Manifest::ManifestVer GetManifestVersion() const override;
    };
}
