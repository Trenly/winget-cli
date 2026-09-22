// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"
#pragma warning( push )
#pragma warning ( disable : 4467 6388)
// 6388 Allow CreateInstance.
#include <wil\cppwinrt_wrl.h>
// 4467 Allow use of uuid attribute for com object creation.
#include "ErrorInformationProvider.h"
#pragma warning( pop )
#include "ErrorInformationProvider.g.cpp"
#include "ErrorInformation.h"
#include "Helpers.h"
#include <AppInstallerErrors.h>
#include <AppInstallerStrings.h>

namespace winrt::Microsoft::Management::Deployment::implementation
{
    namespace
    {
        winrt::Microsoft::Management::Deployment::ErrorInformation CreateErrorInformation(std::unique_ptr<::AppInstaller::Errors::HResultInformation> information)
        {
            auto errorInformation = winrt::make_self<wil::details::module_count_wrapper<winrt::Microsoft::Management::Deployment::implementation::ErrorInformation>>();
            errorInformation->Initialize(std::move(information));
            return *errorInformation;
        }

        winrt::Windows::Foundation::Collections::IVectorView<winrt::Microsoft::Management::Deployment::ErrorInformation> CreateErrorInformationView(
            std::vector<std::unique_ptr<::AppInstaller::Errors::HResultInformation>> information)
        {
            auto result = winrt::single_threaded_vector<winrt::Microsoft::Management::Deployment::ErrorInformation>();

            for (auto& entry : information)
            {
                result.Append(CreateErrorInformation(std::move(entry)));
            }

            return result.GetView();
        }
    }

    winrt::Microsoft::Management::Deployment::ErrorInformation ErrorInformationProvider::GetErrorInformation(int32_t error)
    {
        return CreateErrorInformation(::AppInstaller::Errors::HResultInformation::Find(static_cast<HRESULT>(error)));
    }

    winrt::Windows::Foundation::Collections::IVectorView<winrt::Microsoft::Management::Deployment::ErrorInformation> ErrorInformationProvider::FindErrorInformation(hstring const& searchText)
    {
        std::string search = ::AppInstaller::Utility::ConvertToUTF8(searchText);
        return CreateErrorInformationView(::AppInstaller::Errors::HResultInformation::Find(::AppInstaller::Utility::Trim(search)));
    }

    winrt::Windows::Foundation::Collections::IVectorView<winrt::Microsoft::Management::Deployment::ErrorInformation> ErrorInformationProvider::GetAllErrorInformation()
    {
        return CreateErrorInformationView(::AppInstaller::Errors::GetWinGetErrors());
    }

    CoCreatableMicrosoftManagementDeploymentClass(ErrorInformationProvider);
}
