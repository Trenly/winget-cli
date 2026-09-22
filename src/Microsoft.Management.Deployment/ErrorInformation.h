// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "ErrorInformation.g.h"
#include <AppInstallerErrors.h>

#include <memory>

namespace winrt::Microsoft::Management::Deployment::implementation
{
    struct ErrorInformation : ErrorInformationT<ErrorInformation>
    {
        ErrorInformation() = default;

#if !defined(INCLUDE_ONLY_INTERFACE_METHODS)
        void Initialize(std::unique_ptr<::AppInstaller::Errors::HResultInformation> errorInformation);
#endif

        int32_t Value();
        hstring Symbol();
        hstring Description();

#if !defined(INCLUDE_ONLY_INTERFACE_METHODS)
    private:
        std::unique_ptr<::AppInstaller::Errors::HResultInformation> m_errorInformation;
#endif
    };
}
