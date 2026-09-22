// -----------------------------------------------------------------------------
// <copyright file="ErrorInformationInterop.cs" company="Microsoft Corporation">
//     Copyright (c) Microsoft Corporation. Licensed under the MIT License.
// </copyright>
// -----------------------------------------------------------------------------

namespace AppInstallerCLIE2ETests.Interop
{
    using System.Linq;
    using Microsoft.Management.Deployment;
    using Microsoft.Management.Deployment.Projection;
    using NUnit.Framework;

    /// <summary>
    /// Test error information interop, which provides parity with the `winget error` command.
    /// </summary>
    [TestFixtureSource(typeof(InstanceInitializersSource), nameof(InstanceInitializersSource.InProcess), Category = nameof(InstanceInitializersSource.InProcess))]
    [TestFixtureSource(typeof(InstanceInitializersSource), nameof(InstanceInitializersSource.OutOfProcess), Category = nameof(InstanceInitializersSource.OutOfProcess))]
    public class ErrorInformationInterop : BaseInterop
    {
        private const int InternalError = unchecked((int)0x8A150001);
        private const string InternalErrorSymbol = "APPINSTALLER_CLI_ERROR_INTERNAL_ERROR";
        private const int UnknownWinGetError = unchecked((int)0x8A15FFFF);
        private const int InvalidArg = unchecked((int)0x80070057);

        private ErrorInformationProvider errorInformationProvider;

        /// <summary>
        /// Initializes a new instance of the <see cref="ErrorInformationInterop"/> class.
        /// </summary>
        /// <param name="initializer">Initializer.</param>
        public ErrorInformationInterop(IInstanceInitializer initializer)
            : base(initializer)
        {
        }

        /// <summary>
        /// Set up.
        /// </summary>
        [SetUp]
        public void SetUp()
        {
            this.errorInformationProvider = this.TestFactory.CreateErrorInformationProvider();
        }

        /// <summary>
        /// Verifies that a known Windows Package Manager error returns its symbol and a description.
        /// </summary>
        [Test]
        public void GetErrorInformation_KnownError()
        {
            var errorInformation = this.errorInformationProvider.GetErrorInformation(InternalError);

            Assert.That(errorInformation, Is.Not.Null);
            Assert.That(errorInformation.Value, Is.EqualTo(InternalError));
            Assert.That(errorInformation.Symbol, Is.EqualTo(InternalErrorSymbol));
            Assert.That(errorInformation.Description, Is.Not.Empty);
        }

        /// <summary>
        /// Verifies that an unknown error in the Windows Package Manager facility still returns a result.
        /// </summary>
        [Test]
        public void GetErrorInformation_UnknownWinGetError()
        {
            var errorInformation = this.errorInformationProvider.GetErrorInformation(UnknownWinGetError);

            Assert.That(errorInformation, Is.Not.Null);
            Assert.That(errorInformation.Value, Is.EqualTo(UnknownWinGetError));
            Assert.That(errorInformation.Symbol, Is.Empty);
            Assert.That(errorInformation.Description, Is.Not.Empty);
        }

        /// <summary>
        /// Verifies that a system error returns the system provided description.
        /// </summary>
        [Test]
        public void GetErrorInformation_SystemError()
        {
            var errorInformation = this.errorInformationProvider.GetErrorInformation(InvalidArg);

            Assert.That(errorInformation, Is.Not.Null);
            Assert.That(errorInformation.Value, Is.EqualTo(InvalidArg));
            Assert.That(errorInformation.Symbol, Is.Empty);
            Assert.That(errorInformation.Description, Is.Not.Empty);
        }

        /// <summary>
        /// Verifies that searching for an exact symbol returns that error first.
        /// </summary>
        [Test]
        public void FindErrorInformation_BySymbol()
        {
            var results = this.errorInformationProvider.FindErrorInformation(InternalErrorSymbol);

            Assert.That(results, Is.Not.Null);
            Assert.That(results.Count, Is.GreaterThan(0));
            Assert.That(results[0].Value, Is.EqualTo(InternalError));
            Assert.That(results[0].Symbol, Is.EqualTo(InternalErrorSymbol));
        }

        /// <summary>
        /// Verifies that a partial symbol search returns multiple matching errors.
        /// </summary>
        [Test]
        public void FindErrorInformation_BySubstring()
        {
            var results = this.errorInformationProvider.FindErrorInformation("INSTALLER");

            Assert.That(results, Is.Not.Null);
            Assert.That(results.Count, Is.GreaterThan(1));
            Assert.That(results.All(result => !string.IsNullOrEmpty(result.Symbol)), Is.True);
        }

        /// <summary>
        /// Verifies that input is trimmed before searching, matching the `winget error` behavior.
        /// </summary>
        [Test]
        public void FindErrorInformation_TrimsInput()
        {
            var results = this.errorInformationProvider.FindErrorInformation($"  {InternalErrorSymbol}  ");

            Assert.That(results, Is.Not.Null);
            Assert.That(results.Count, Is.GreaterThan(0));
            Assert.That(results[0].Value, Is.EqualTo(InternalError));
        }

        /// <summary>
        /// Verifies that a search with no matches returns an empty collection.
        /// </summary>
        [Test]
        public void FindErrorInformation_NoMatches()
        {
            var results = this.errorInformationProvider.FindErrorInformation("ThisTextShouldNotMatchAnyWinGetError");

            Assert.That(results, Is.Not.Null);
            Assert.That(results.Count, Is.EqualTo(0));
        }

        /// <summary>
        /// Verifies that all Windows Package Manager errors are returned with symbols and descriptions.
        /// </summary>
        [Test]
        public void GetAllErrorInformation()
        {
            var results = this.errorInformationProvider.GetAllErrorInformation();

            Assert.That(results, Is.Not.Null);
            Assert.That(results.Count, Is.GreaterThan(0));
            Assert.That(results.All(result => !string.IsNullOrEmpty(result.Symbol)), Is.True);
            Assert.That(results.All(result => !string.IsNullOrEmpty(result.Description)), Is.True);
            Assert.That(results.Any(result => result.Value == InternalError), Is.True);
        }
    }
}
