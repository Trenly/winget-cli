// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "TestCommon.h"
#include "TestRestRequestHandler.h"
#include "TestCertificates.h"
#include <AppInstallerErrors.h>
#include <AppInstallerRuntime.h>
#include <AppInstallerStrings.h>
#include <winget/Certificates.h>
#include <winget/HttpClientHelper.h>
#include <winget/JsonUtil.h>

using namespace AppInstaller::Http;
using namespace AppInstaller::Runtime;
using namespace AppInstaller::Utility;
using namespace AppInstaller::Certificates;

TEST_CASE("ExtractJsonResponse_UnsupportedMimeType", "[RestSource][RestSearch]")
{
    HttpClientHelper helper{ GetTestRestRequestHandler(HTTP_STATUS_OK, L"", std::wstring{ AppInstaller::Utility::Http::MimeType::TextPlain }) };
    REQUIRE_THROWS_HR(helper.HandleGet(L"https://testUri"), APPINSTALLER_CLI_ERROR_RESTAPI_UNSUPPORTED_MIME_TYPE);
}

TEST_CASE("ValidateAndExtractResponse_ServiceUnavailable", "[RestSource]")
{
    HttpClientHelper helper{ GetTestRestRequestHandler(HTTP_STATUS_SERVICE_UNAVAIL) };
    REQUIRE_THROWS_HR(helper.HandleGet(L"https://testUri"), APPINSTALLER_CLI_ERROR_SERVICE_UNAVAILABLE);
}

TEST_CASE("ValidateAndExtractResponse_NotFound", "[RestSource]")
{
    HttpClientHelper helper{ GetTestRestRequestHandler(HTTP_STATUS_NOT_FOUND) };
    REQUIRE_THROWS_HR(helper.HandleGet(L"https://testUri"), APPINSTALLER_CLI_ERROR_RESTAPI_ENDPOINT_NOT_FOUND);
}

TEST_CASE("EnsureDefaultUserAgent", "[RestSource]")
{
    HttpClientHelper helper{ GetTestRestRequestHandler([](const AppInstaller::Rest::Request& request)
        {
            auto itr = request.Headers.find(std::wstring{ AppInstaller::Utility::Http::Header::UserAgent });
            if (itr != request.Headers.end() &&
                itr->second.find(ConvertToUTF16(GetClientVersion())) != std::wstring::npos &&
                itr->second.find(ConvertToUTF16(GetPackageVersion())) != std::wstring::npos)
            {
                return HTTP_STATUS_OK;
            }
            else
            {
                return HTTP_STATUS_BAD_REQUEST;
            }
        }) };

    SECTION("GET")
    {
        REQUIRE_NOTHROW(helper.HandleGet(L"https://testUri"));
    }
    SECTION("POST")
    {
        REQUIRE_NOTHROW(helper.HandlePost(L"https://testUri", {}));
    }
}

TEST_CASE("HttpClientHelper_PinningConfiguration", "[RestSource][uses-test-certificates]")
{
    // Create a pinning chain with test certs that won't match any real server
    TestCommon::TestCertificateChain testChain;
    PinningChain chain;
    auto chainElement = chain.Root();
    chainElement->LoadCertificate(testChain.Root().View()).SetPinning(PinningVerificationType::PublicKey);
    chainElement = chainElement.Next();
    chainElement->LoadCertificate(testChain.Intermediate2().View()).SetPinning(PinningVerificationType::Subject | PinningVerificationType::Issuer);
    chainElement = chainElement.Next();
    chainElement->LoadCertificate(testChain.Leaf2().View()).SetPinning(PinningVerificationType::Subject | PinningVerificationType::Issuer);

    PinningConfiguration config;
    config.AddChain(chain);

    HttpClientHelper helper;
    helper.SetPinningConfiguration(config);

    REQUIRE_THROWS_HR(helper.HandleGet(L"https://github.com"), APPINSTALLER_CLI_ERROR_PINNED_CERTIFICATE_MISMATCH);
}

TEST_CASE("HttpClientHelper_CallerCharacters", "[RestSource]")
{
    HttpClientHelper::HttpRequestHeaders headers;
    headers.emplace(std::wstring{ AppInstaller::Utility::Http::Header::UserAgent }, AppInstaller::JSON::GetUtilityString(AppInstaller::Runtime::GetUserAgent("\xe6\xb5\x8b\xe8\xaf\x95")));

    HttpClientHelper helper;
    REQUIRE_THROWS_HR(helper.HandleGet(L"https://github.com", headers), APPINSTALLER_CLI_ERROR_RESTAPI_UNSUPPORTED_MIME_TYPE);
}
