// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include <winget/RestHelpers.h>

#include <AppInstallerErrors.h>
#include <AppInstallerDownloader.h>
#include <AppInstallerStrings.h>
#include <winget/WebRequest.h>
#include <winhttp.h>

namespace AppInstaller::Rest
{
    std::wstring GetHeaderValue(const HttpHeaders& headers, std::wstring_view headerName)
    {
        for (const auto& header : headers)
        {
            if (Utility::CaseInsensitiveEquals(header.first, headerName))
            {
                return header.second;
            }
        }

        return {};
    }

    namespace
    {
        constexpr uint32_t s_HttpTooManyRequests = 429;
    }

    namespace Json
    {
        ::Json::Value Parse(std::string_view content)
        {
            ::Json::Value result;
            ::Json::CharReaderBuilder builder;
            std::string errors;
            std::istringstream contentStream{ std::string{ content } };

            THROW_HR_IF(APPINSTALLER_CLI_ERROR_RESTAPI_INTERNAL_ERROR,
                !::Json::parseFromStream(builder, contentStream, &result, &errors));

            return result;
        }

        ::Json::Value Parse(std::wstring_view content)
        {
            return Parse(Utility::ConvertToUTF8(content));
        }

        std::string Serialize(const ::Json::Value& value)
        {
            ::Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            return ::Json::writeString(builder, value);
        }

        const ::Json::Value* GetOptionalValue(const ::Json::Value& value, std::string_view fieldName)
        {
            if (!value.isObject())
            {
                return nullptr;
            }

            const std::string fieldNameString{ fieldName };
            if (!value.isMember(fieldNameString))
            {
                return nullptr;
            }

            return &value[fieldNameString];
        }

        const ::Json::Value& GetRequiredValue(const ::Json::Value& value, std::string_view fieldName)
        {
            const ::Json::Value* result = GetOptionalValue(value, fieldName);
            THROW_HR_IF(APPINSTALLER_CLI_ERROR_RESTSOURCE_INVALID_DATA, !result);
            return *result;
        }

        std::optional<std::string> GetStringValue(const ::Json::Value& value)
        {
            if (!value.isString())
            {
                return {};
            }

            return value.asString();
        }

        std::optional<int> GetIntValue(const ::Json::Value& value)
        {
            if (!value.isInt())
            {
                return {};
            }

            return value.asInt();
        }

        std::optional<uint64_t> GetUInt64Value(const ::Json::Value& value)
        {
            if (!value.isUInt64())
            {
                return {};
            }

            return value.asUInt64();
        }

        std::optional<bool> GetBoolValue(const ::Json::Value& value)
        {
            if (!value.isBool())
            {
                return {};
            }

            return value.asBool();
        }

        std::vector<std::string> GetStringArray(const ::Json::Value& value)
        {
            std::vector<std::string> result;

            if (!value.isArray())
            {
                return result;
            }

            for (const ::Json::Value& element : value)
            {
                std::optional<std::string> elementValue = GetStringValue(element);
                if (elementValue)
                {
                    result.emplace_back(std::move(elementValue.value()));
                }
            }

            return result;
        }
    }

    std::optional<::Json::Value> ValidateAndExtractJsonResponse(const Response& response)
    {
        AICLI_LOG(Repo, Info, << "Response status: " << response.StatusCode);

        switch (response.StatusCode)
        {
        case HTTP_STATUS_OK:
        {
            std::wstring contentType = GetHeaderValue(response.Headers, Utility::Http::Header::ContentType);
            THROW_HR_IF(APPINSTALLER_CLI_ERROR_RESTAPI_UNSUPPORTED_MIME_TYPE,
                contentType.empty() || !contentType._Starts_with(std::wstring{ Utility::Http::MimeType::ApplicationJson }));

            return Json::Parse(response.Body);
        }
        case HTTP_STATUS_NOT_FOUND:
            THROW_HR(APPINSTALLER_CLI_ERROR_RESTAPI_ENDPOINT_NOT_FOUND);

        case HTTP_STATUS_NO_CONTENT:
            return {};

        case HTTP_STATUS_BAD_REQUEST:
            THROW_HR(APPINSTALLER_CLI_ERROR_RESTAPI_INTERNAL_ERROR);

        case s_HttpTooManyRequests:
        case HTTP_STATUS_SERVICE_UNAVAIL:
        {
            std::wstring retryAfter = GetHeaderValue(response.Headers, Utility::Http::Header::RetryAfter);
            THROW_EXCEPTION(AppInstaller::Utility::ServiceUnavailableException(AppInstaller::Utility::GetRetryAfter(retryAfter)));
        }

        default:
            THROW_HR(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, response.StatusCode));
        }
    }
}
