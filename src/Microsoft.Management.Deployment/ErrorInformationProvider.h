// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "ErrorInformationProvider.g.h"
#include "Public/ComClsids.h"
#include <winget/ModuleCountBase.h>

namespace winrt::Microsoft::Management::Deployment::implementation
{
    [uuid(WINGET_OUTOFPROC_COM_CLSID_ErrorInformationProvider)]
    struct ErrorInformationProvider : ErrorInformationProviderT<ErrorInformationProvider>
    {
        ErrorInformationProvider() = default;

        winrt::Microsoft::Management::Deployment::ErrorInformation GetErrorInformation(int32_t error);
        winrt::Windows::Foundation::Collections::IVectorView<winrt::Microsoft::Management::Deployment::ErrorInformation> FindErrorInformation(hstring const& searchText);
        winrt::Windows::Foundation::Collections::IVectorView<winrt::Microsoft::Management::Deployment::ErrorInformation> GetAllErrorInformation();
    };
}

#if !defined(INCLUDE_ONLY_INTERFACE_METHODS)
namespace winrt::Microsoft::Management::Deployment::factory_implementation
{
    struct ErrorInformationProvider : ErrorInformationProviderT<ErrorInformationProvider, implementation::ErrorInformationProvider>, AppInstaller::WinRT::ModuleCountBase
    {
    };
}
#endif
