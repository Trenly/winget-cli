// -----------------------------------------------------------------------------
// <copyright file="IDSCv3.cs" company="Microsoft Corporation">
//     Copyright (c) Microsoft Corporation. Licensed under the MIT License.
// </copyright>
// -----------------------------------------------------------------------------

namespace Microsoft.Management.Configuration.Processor.DSCv3.Model
{
    using Microsoft.Management.Configuration.Processor.DSCv3.Helpers;
    using Microsoft.Management.Configuration.Processor.Helpers;

    /// <summary>
    /// Interface for interacting with DSC v3.
    /// </summary>
    internal interface IDSCv3
    {
        /// <summary>
        /// Creates the appropriate instance of the DSCv3 interface for the given executable.
        /// </summary>
        /// <param name="processorSettings">The processor settings.</param>
        /// <returns>An object that properly interacts with the specific version of DSC v3.</returns>
        public static IDSCv3 Create(ProcessorSettings processorSettings)
        {
            // Expand as needed to detect the version of dsc.exe and/or its schemas in use.
            return new Schema_2024_04.DSCv3(processorSettings);
        }

        /// <summary>
        /// Gets a single resource by its type name.
        /// </summary>
        /// <param name="resourceType">The type name of the resource.</param>
        /// <returns>A single resource item.</returns>
        public IResourceListItem? GetResourceByType(string resourceType);

        /// <summary>
        /// Tests a configuration unit.
        /// </summary>
        /// <param name="unitInternal">The unit to test.</param>
        /// <returns>A test result.</returns>
        public IResourceTestItem TestResource(ConfigurationUnitInternal unitInternal);

        /// <summary>
        /// Gets a configuration unit settings.
        /// </summary>
        /// <param name="unitInternal">The unit to get.</param>
        /// <returns>A get result.</returns>
        public IResourceGetItem GetResourceSettings(ConfigurationUnitInternal unitInternal);

        /// <summary>
        /// Sets a configuration unit settings.
        /// </summary>
        /// <param name="unitInternal">The unit to set.</param>
        /// <returns>A set result.</returns>
        public IResourceSetItem SetResourceSettings(ConfigurationUnitInternal unitInternal);
    }
}
