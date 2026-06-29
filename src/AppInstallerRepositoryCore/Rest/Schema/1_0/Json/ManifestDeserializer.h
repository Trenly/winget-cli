// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <winget/Manifest.h>
#include <json/json.h>
#include <winget/JsonUtil.h>

namespace AppInstaller::Repository::Rest::Schema::V1_0::Json
{
    // Manifest Deserializer.
    struct ManifestDeserializer
    {
        // Gets the manifest from the given json object received from a REST request
        std::vector<Manifest::Manifest> Deserialize(const ::Json::Value& responseJsonObject) const;

        // Gets the manifest from the given json Data field
        std::vector<Manifest::Manifest> DeserializeData(const ::Json::Value& dataJsonObject) const;

        // Deserializes the AppsAndFeaturesEntries node, returning the set of values below it.
        virtual std::vector<Manifest::AppsAndFeaturesEntry> DeserializeAppsAndFeaturesEntries(const ::Json::Value& entries) const;

        // Deserializes the locale; requires that the PackageLocale be set to return an object.
        virtual std::optional<Manifest::ManifestLocalization> DeserializeLocale(const ::Json::Value& localeJsonObject) const;

        // Deserializes the locale; requires that the PackageLocale be set to return an object.
        virtual std::optional<Manifest::InstallationMetadataInfo> DeserializeInstallationMetadata(const ::Json::Value& installationMetadataJsonObject) const;

    protected:

        template <Manifest::Localization L>
        inline void TryParseStringLocaleField(Manifest::ManifestLocalization& manifestLocale, const ::Json::Value& localeJsonObject, std::string_view localeJsonFieldName) const
        {
            auto value = AppInstaller::JSON::GetRawStringValueFromJsonNode(localeJsonObject, localeJsonFieldName);

            if (AppInstaller::JSON::IsValidNonEmptyStringValue(value))
            {
                manifestLocale.Add<L>(value.value());
            }
        }

        virtual std::optional<Manifest::ManifestInstaller> DeserializeInstaller(const ::Json::Value& installerJsonObject) const;

        virtual std::map<Manifest::InstallerSwitchType, Manifest::string_t> DeserializeInstallerSwitches(const ::Json::Value& installerSwitchesJsonObject) const;

        std::optional<Manifest::DependencyList> DeserializeDependency(const ::Json::Value& dependenciesJsonObject) const;

        virtual Manifest::InstallerTypeEnum ConvertToInstallerType(std::string_view in) const;

        virtual Manifest::UpdateBehaviorEnum ConvertToUpdateBehavior(std::string_view in) const;

        std::vector<Manifest::string_t> ConvertToManifestStringArray(const std::vector<std::string>& values) const;

        virtual Manifest::ManifestVer GetManifestVersion() const;
    };
}
