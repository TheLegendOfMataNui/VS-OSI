using System;
using System.ComponentModel.Composition;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.ProjectSystem;
using Microsoft.VisualStudio.ProjectSystem.Build;
using Microsoft.VisualStudio.Shell.Interop;

namespace OSIProject
{
    [Export(typeof(IDeployProvider))]
    [AppliesTo(MyUnconfiguredProject.UniqueCapability)]
    internal class OSADeploy : IDeployProvider
    {
        private ConfiguredProject configuredProject;

        [ImportingConstructor]
        public OSADeploy(ConfiguredProject configuredProject)
        {
            this.configuredProject = configuredProject;
            //this.IVsHierarchies = new OrderPrecedenceImportCollection<IVsHierarchy>(projectCapabilityCheckProvider: unconfiguredProject);
        }

        /*/// <summary>
        /// Gets or sets IVsHierarchies.
        /// </summary>
        [ImportMany(ExportContractNames.VsTypes.IVsHierarchy)]
        private OrderPrecedenceImportCollection<IVsHierarchy> IVsHierarchies { get; set; }

        /// <summary>
        /// Gets the IVsHierarchy instance for the project being built.
        /// </summary>
        internal IVsHierarchy VsHierarchy
        {
            get
            {
                return this.IVsHierarchies.First().Value;
            }
        }*/

        

        /// <summary>
        /// Provides access to the project's properties.
        /// </summary>
        [Import]
        private ProjectProperties Properties { get; set; }

        public async Task DeployAsync(CancellationToken cancellationToken, TextWriter outputPaneWriter)
        {
            /*string gameDirectory = System.IO.Path.GetDirectoryName(await (await Properties.GetOSIDebuggerPropertiesAsync()).GameExecutable.GetEvaluatedValueAtEndAsync());
            //string outputDirectory = System.IO.Path.Combine(await (await Properties.GetConfigurationGeneralPropertiesAsync()).)

            foreach (IProjectItem item in await configuredProject.Services.SourceItems.GetItemsAsync())
            {
                await outputPaneWriter.WriteLineAsync(item.EvaluatedIncludeAsFullPath);
            }

            await outputPaneWriter.WriteLineAsync("Copying output to '" + gameDirectory + "'...");*/

            // Add your custom deploy code here. Write informational output to the outputPaneWriter.
            await outputPaneWriter.WriteAsync("Hello Deploy!!!");
        }

        public bool IsDeploySupported
        {
            get { return true; }
        }

        public void Commit()
        {
        }

        public void Rollback()
        {
        }
    }
}