/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

namespace OSIProject
{
    using System;
    using System.ComponentModel;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio.Shell;

    /// <summary>
    /// This class implements the package exposed by this assembly.
    /// </summary>
    /// <remarks>
    /// This package is required if you want to define adds custom commands (ctmenu)
    /// or localized resources for the strings that appear in the New Project and Open Project dialogs.
    /// Creating project extensions or project types does not actually require a VSPackage.
    /// </remarks>
    [PackageRegistration(UseManagedResourcesOnly = true)]
    [Description("A project type for compiling and debugging OSI scripts for the Saffire Advanced Game Engine (SAGE)")]
    [Guid(VsPackage.PackageGuid)]
    [ProvideAutoLoad(VsPackage.ProjectTypeGuid)]
    [ProvideBindingPath]
    public sealed class VsPackage : Package
    {
        /// <summary>
        /// The GUID for this package.
        /// </summary>
        public const string PackageGuid = "4d9c8c38-bb6f-4d1b-99fb-e4444a5d811f";

        /// <summary>
        /// The GUID for this project type.  It is unique with the project file extension and
        /// appears under the VS registry hive's Projects key.
        /// </summary>
        public const string ProjectTypeGuid = "a13e84ac-da16-43b6-93ad-4c72a8404633";

        /// <summary>
        /// The file extension of this project type.  No preceding period.
        /// </summary>
        public const string ProjectExtension = "osiproj";

        /// <summary>
        /// The default namespace this project compiles with, so that manifest
        /// resource names can be calculated for embedded resources.
        /// </summary>
        internal const string DefaultNamespace = "OSIProject";

        protected override void Initialize()
        {
            // Write the DLL filename to the registry for later use by the MSBuild task
            try
            {
                Microsoft.Win32.Registry.SetValue(@"HKEY_CURRENT_USER\Software\Litestone\VSOSI", "TaskDLL", System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetAssembly(typeof(VsPackage)).Location) + @"\OSIProject.Tasks.dll");

            }
            catch (Exception ex)
            {

            }
            base.Initialize();
        }
    }
}
