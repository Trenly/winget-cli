// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "Rest/Schema/1_1/Json/ManifestDeserializer.h"

namespace AppInstaller::Repository::Rest::Schema::V1_4::Json
{
    // Manifest Deserializer.
    struct ManifestDeserializer : public V1_1::Json::ManifestDeserializer
    {
        std::optional<Manifest::ManifestLocalization> DeserializeLocale(const ::Json::Value& localeJsonObject) const override;

        std::optional<Manifest::InstallationMetadataInfo> DeserializeInstallationMetadata(const ::Json::Value& installationMetadataJsonObject) const override;

    protected:
        std::optional<Manifest::ManifestInstaller> DeserializeInstaller(const ::Json::Value& installerJsonObject) const override;

        Manifest::InstallerTypeEnum ConvertToInstallerType(std::string_view in) const override;

        Manifest::ExpectedReturnCodeEnum ConvertToExpectedReturnCodeEnum(std::string_view in) const override;

        Manifest::ManifestInstaller::ExpectedReturnCodeInfo DeserializeExpectedReturnCodeInfo(const ::Json::Value& expectedReturnCodeJsonObject) const override;

        Manifest::ManifestVer GetManifestVersion() const override;
    };
}
