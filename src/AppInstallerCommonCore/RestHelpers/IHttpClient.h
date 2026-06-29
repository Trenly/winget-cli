// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <winget/RestHelpers.h>

namespace AppInstaller::Rest
{
    struct WinHttpClient : public IHttpClient
    {
        WinHttpClient(
            std::optional<Certificates::PinningConfiguration> pinningConfiguration = {},
            std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals = {});

        Response Send(const Request& request) const override;
        Response Get(const HttpString& uri, const HttpHeaders& headers = {}, const HttpHeaders& authHeaders = {}) const override;
        Response Post(const HttpString& uri, const ::Json::Value& body, const HttpHeaders& headers = {}, const HttpHeaders& authHeaders = {}) const override;
        void SetPinningConfiguration(
            const Certificates::PinningConfiguration& pinningConfiguration,
            std::shared_ptr<ThreadLocalStorage::ThreadGlobals> threadGlobals) override;

    private:
        mutable std::optional<Certificates::PinningConfiguration> m_pinningConfiguration;
        mutable std::shared_ptr<ThreadLocalStorage::ThreadGlobals> m_threadGlobals;
    };
}
