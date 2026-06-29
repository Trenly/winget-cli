// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "RestInformationCache.h"
#include "Rest/Schema/InformationResponseDeserializer.h"
#include <winget/RestHelpers.h>
#include <winget/JsonUtil.h>

namespace AppInstaller::Repository::Rest
{
    namespace
    {
        constexpr std::wstring_view s_EndpointName = L"endpoint"sv;
        constexpr std::wstring_view s_HashName = L"hash"sv;
        constexpr std::wstring_view s_ExpirationName = L"expiration"sv;
        constexpr std::wstring_view s_DataName = L"data"sv;

        // Calculates the hash of values that might change per-call.
        Utility::SHA256::HashBuffer GetHash(const std::optional<std::string>& customHeader, std::string_view caller)
        {
            std::stringstream stream;
            if (customHeader)
            {
                stream << customHeader.value();
            }
            stream << '|' << caller;

            return Utility::SHA256::ComputeHash(stream);
        }

        uint64_t CalculateExpiration(std::chrono::seconds duration)
        {
            // If no expiration information is provided, use 1 minute
            if (!duration.count())
            {
                duration = 60s;
            }

            return Utility::ConvertSystemClockToUnixEpoch(std::chrono::system_clock::now() + duration);
        }
    }

    std::optional<Schema::IRestClient::Information> RestInformationCache::Get(const std::wstring& endpoint, const std::optional<std::string>& customHeader, std::string_view caller)
#ifdef AICLI_DISABLE_TEST_HOOKS
        try
#endif
    {
        LoadCacheView();

        Utility::SHA256::HashBuffer hashValue = GetHash(customHeader, caller);
        CacheItem* item = FindCacheItem(endpoint, hashValue);

        // If we don't find a private match, see if there is a public one.
        if (!item)
        {
            item = FindCacheItem(endpoint, {});
        }

        if (!item)
        {
            return std::nullopt;
        }

        Schema::InformationResponseDeserializer responseDeserializer;
        return responseDeserializer.Deserialize(item->Data);
    }
#ifdef AICLI_DISABLE_TEST_HOOKS
    catch (...)
    {
        LOG_CAUGHT_EXCEPTION_MSG("RestInformationCache::Get exception");
        return std::nullopt;
    }
#endif

    void RestInformationCache::Cache(const std::wstring& endpoint, const std::optional<std::string>& customHeader, std::string_view caller, const Utility::CacheControlPolicy& cacheControl, Json::Value response)
#ifdef AICLI_DISABLE_TEST_HOOKS
        try
#endif
    {
        // If requested, do not cache this response.
        // Since this data is small, treat no-cache as no-store.
        if (cacheControl.NoStore || cacheControl.NoCache)
        {
            return;
        }

        // If not public, we use the header values to differentiate the cache items.
        Utility::SHA256::HashBuffer hashValue;
        if (!cacheControl.Public)
        {
            hashValue = GetHash(customHeader, caller);
        }

        uint64_t expirationEpoch = CalculateExpiration(std::chrono::seconds{ cacheControl.MaxAge });

        // Due to the exchange semantics on the setting stream, we may have to retry storing the value.
        for (int i = 0; i < 10; ++i)
        {
            CacheItem* item = FindCacheItem(endpoint, hashValue);

            if (!item)
            {
                item = &m_cacheView.emplace_back();

                item->Endpoint = endpoint;
                item->Hash = hashValue;
            }

            item->UnixEpochExpiration = expirationEpoch;
            item->Data = response;

            if (StoreCacheView())
            {
                AICLI_LOG(Repo, Verbose, << "RestInformationCache stored information for: " << Utility::ConvertToUTF8(endpoint));
                return;
            }
            else
            {
                // Failed to store due to the cache changing, reload and try again.
                LoadCacheView();
            }
        }

        AICLI_LOG(Repo, Warning, << "RestInformationCache failed to store information cache after 10 attempts.");
    }
#ifdef AICLI_DISABLE_TEST_HOOKS
    CATCH_LOG();
#endif

    void RestInformationCache::LoadCacheView()
    {
        std::unique_ptr<std::istream> stream = m_settingsStream.Get();
        m_cacheView.clear();

        if (!stream)
        {
            return;
        }

        Json::Value cacheValue = AppInstaller::Rest::Json::Parse(Utility::ReadEntireStream(*stream));

        if (!cacheValue.isArray())
        {
            AICLI_LOG(Repo, Warning, << "RestInformationCache value was not an array.");
            return;
        }

        for (const Json::Value& cacheItemValue : cacheValue)
        {
            if (!cacheItemValue.isObject())
            {
                AICLI_LOG(Repo, Warning, << "RestInformationCache cache item was not an object.");
                continue;
            }

            std::optional<uint64_t> expiration = AppInstaller::Rest::Json::GetUInt64Value(cacheItemValue[Utility::ConvertToUTF8(s_ExpirationName)]);
            if (!expiration)
            {
                AICLI_LOG(Repo, Warning, << "RestInformationCache cache item missing expiration.");
                continue;
            }

            if (std::chrono::system_clock::now() > Utility::ConvertUnixEpochToSystemClock(expiration.value()))
            {
                AICLI_LOG(Repo, Verbose, << "RestInformationCache cache item has expired.");
                continue;
            }

            std::optional<std::string> endpoint = AppInstaller::Rest::Json::GetStringValue(cacheItemValue[Utility::ConvertToUTF8(s_EndpointName)]);
            if (!JSON::IsValidNonEmptyStringValue(endpoint))
            {
                AICLI_LOG(Repo, Warning, << "RestInformationCache cache item missing endpoint.");
                continue;
            }

            CacheItem cacheItem;
            cacheItem.Endpoint = Utility::ConvertToUTF16(endpoint.value());
            cacheItem.UnixEpochExpiration = expiration.value();

            std::optional<std::string> hash = AppInstaller::Rest::Json::GetStringValue(cacheItemValue[Utility::ConvertToUTF8(s_HashName)]);
            if (JSON::IsValidNonEmptyStringValue(hash))
            {
                cacheItem.Hash = Utility::SHA256::ConvertToBytes(Utility::ConvertToUTF16(hash.value()));
            }

            const Json::Value& dataValue = cacheItemValue[Utility::ConvertToUTF8(s_DataName)];
            if (dataValue.isNull())
            {
                AICLI_LOG(Repo, Warning, << "RestInformationCache cache item missing data.");
                continue;
            }

            cacheItem.Data = dataValue;

            m_cacheView.emplace_back(std::move(cacheItem));
        }
    }

    RestInformationCache::CacheItem* RestInformationCache::FindCacheItem(const std::wstring& endpoint, const Utility::SHA256::HashBuffer& hash)
    {
        for (CacheItem& item : m_cacheView)
        {
            if (item.Endpoint == endpoint &&
                Utility::SHA256::AreEqual(item.Hash, hash))
            {
                return &item;
            }
        }

        return nullptr;
    }

    [[nodiscard]] bool RestInformationCache::StoreCacheView()
    {
        Json::Value cacheValue{ Json::arrayValue };

        for (const CacheItem& item : m_cacheView)
        {
            Json::Value cacheItemValue{ Json::objectValue };
            cacheItemValue[Utility::ConvertToUTF8(s_EndpointName)] = Utility::ConvertToUTF8(item.Endpoint);
            cacheItemValue[Utility::ConvertToUTF8(s_HashName)] = Utility::ConvertToHexString(item.Hash);
            cacheItemValue[Utility::ConvertToUTF8(s_ExpirationName)] = Json::UInt64{ item.UnixEpochExpiration };
            cacheItemValue[Utility::ConvertToUTF8(s_DataName)] = item.Data;

            cacheValue.append(std::move(cacheItemValue));
        }

        std::string stream = AppInstaller::Rest::Json::Serialize(cacheValue);

        return m_settingsStream.Set(std::move(stream));
    }
}
