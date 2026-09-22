// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#include "ErrorInformation.h"
#include "ErrorInformation.g.cpp"

namespace winrt::Microsoft::Management::Deployment::implementation
{
    void ErrorInformation::Initialize(std::unique_ptr<::AppInstaller::Errors::HResultInformation> errorInformation)
    {
        m_errorInformation = std::move(errorInformation);
    }

    int32_t ErrorInformation::Value()
    {
        return static_cast<int32_t>(m_errorInformation->Value());
    }

    hstring ErrorInformation::Symbol()
    {
        return winrt::to_hstring(static_cast<std::string_view>(m_errorInformation->Symbol()));
    }

    hstring ErrorInformation::Description()
    {
        return winrt::to_hstring(static_cast<std::string_view>(m_errorInformation->GetDescription()));
    }
}
