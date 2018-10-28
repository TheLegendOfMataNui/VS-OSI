using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.IO;
using System.Threading.Tasks;
using Microsoft.VisualStudio.ProjectSystem;
using Microsoft.VisualStudio.ProjectSystem.Debug;
using Microsoft.VisualStudio.ProjectSystem.Properties;
using Microsoft.VisualStudio.ProjectSystem.VS.Debug;

namespace OSIProject
{
#if true
    [ExportDebugger("OSIDebugger")]
    [AppliesTo(MyUnconfiguredProject.UniqueCapability)]
    public class ScriptDebuggerLaunchProvider : DebugLaunchProviderBase
    {


        [ImportingConstructor]
        public ScriptDebuggerLaunchProvider(ConfiguredProject configuredProject)
            : base(configuredProject)
        {
        }

        [ExportPropertyXamlRuleDefinition("OSIProject, Version=1.0.0.0, Culture=neutral, PublicKeyToken=9be6e469bc4921f1", "XamlRuleToCode:OSIDebugger.xaml", "Project")]
        [AppliesTo(MyUnconfiguredProject.UniqueCapability)]
        private object DebuggerXaml { get { throw new NotImplementedException(); } }

        /// <summary>
        /// Gets project properties that the debugger needs to launch.
        /// </summary>
        [Import]
        private ProjectProperties ProjectProperties { get; set; }

        public override async Task<bool> CanLaunchAsync(DebugLaunchOptions launchOptions)
        {
            return true;
            //var generalProperties = await this.ProjectProperties.GetConfigurationGeneralPropertiesAsync();
            //string startupItem = await generalProperties.StartItem.GetEvaluatedValueAtEndAsync();
            //return !string.IsNullOrEmpty(startupItem);
            var debuggerProperties = await this.ProjectProperties.GetOSIDebuggerPropertiesAsync();
            string gameExecutable = await debuggerProperties.GameExecutable.GetEvaluatedValueAtEndAsync();
            return System.IO.File.Exists(gameExecutable);

            //return true;
        }

        public override async Task<IReadOnlyList<IDebugLaunchSettings>> QueryDebugTargetsAsync(DebugLaunchOptions launchOptions)
        {
            var settings = new DebugLaunchSettings(launchOptions);

            // The properties that are available via DebuggerProperties are determined by the property XAML files in your project.
            //var debuggerProperties = await this.ProjectProperties.GetScriptDebuggerPropertiesAsync();
            //settings.CurrentDirectory = await debuggerProperties.RunWorkingDirectory.GetEvaluatedValueAtEndAsync();

            //string scriptCommand = await debuggerProperties.RunCommand.GetEvaluatedValueAtEndAsync();
            //string scriptArguments = await debuggerProperties.RunCommandArguments.GetEvaluatedValueAtEndAsync();

            var generalProperties = await this.ProjectProperties.GetConfigurationGeneralPropertiesAsync();

            var debuggerProperties = await this.ProjectProperties.GetOSIDebuggerPropertiesAsync();
            settings.CurrentDirectory = System.IO.Path.GetDirectoryName(await debuggerProperties.GameExecutable.GetEvaluatedValueAtEndAsync());
            settings.Executable = await debuggerProperties.GameExecutable.GetEvaluatedValueAtEndAsync();
            settings.Arguments = "";



            //string startupItem = await generalProperties.StartItem.GetEvaluatedValueAtEndAsync();

            /*if ((launchOptions & DebugLaunchOptions.NoDebug) == DebugLaunchOptions.NoDebug)
            {
                // No debug - launch cscript using cmd.exe to introduce a pause at the end
                settings.Executable = Path.Combine(Environment.SystemDirectory, "cmd.exe");
                settings.Arguments = string.Format("/c {0} \"{1}\" {2} & pause", scriptCommand, startupItem, scriptArguments);
            }
            else
            {
                // Debug - launch cscript using the debugger switch //X
                settings.Executable = scriptCommand;
                settings.Arguments = string.Format("\"{0}\" //X {1}", startupItem, scriptArguments);
            }*/

            settings.LaunchOperation = DebugLaunchOperation.CreateProcess;
            settings.LaunchDebugEngineGuid = DebuggerEngines.NativeOnlyEngine;

            return new IDebugLaunchSettings[] { settings };
        }
    }
#else
    [ExportDebugger(ScriptDebugger.SchemaName)]
    [AppliesTo("")] //MyUnconfiguredProject.UniqueCapability
    public class ScriptDebuggerLaunchProvider : DebugLaunchProviderBase
    {
        [ImportingConstructor]
        public ScriptDebuggerLaunchProvider(ConfiguredProject configuredProject)
            : base(configuredProject)
        {
        }

        [ExportPropertyXamlRuleDefinition("OSIProject, Version=1.0.0.0, Culture=neutral, PublicKeyToken=9be6e469bc4921f1", "XamlRuleToCode:OSIDebugger.xaml", "Project")]
        [AppliesTo(MyUnconfiguredProject.UniqueCapability)]
        private object DebuggerXaml { get { throw new NotImplementedException(); } }

        /// <summary>
        /// Gets project properties that the debugger needs to launch.
        /// </summary>
        [Import]
        private ProjectProperties ProjectProperties { get; set; }

        public override async Task<bool> CanLaunchAsync(DebugLaunchOptions launchOptions)
        {
            return true;
            /*var generalProperties = await this.ProjectProperties.GetConfigurationGeneralPropertiesAsync();
            string startupItem = await generalProperties.StartItem.GetEvaluatedValueAtEndAsync();
            return !string.IsNullOrEmpty(startupItem);*/
        }

        public override async Task<IReadOnlyList<IDebugLaunchSettings>> QueryDebugTargetsAsync(DebugLaunchOptions launchOptions)
        {
            var settings = new DebugLaunchSettings(launchOptions);

            // The properties that are available via DebuggerProperties are determined by the property XAML files in your project.
            /*var debuggerProperties = await this.ProjectProperties.GetScriptDebuggerPropertiesAsync();
            settings.CurrentDirectory = await debuggerProperties.RunWorkingDirectory.GetEvaluatedValueAtEndAsync();

            string scriptCommand = await debuggerProperties.RunCommand.GetEvaluatedValueAtEndAsync();
            string scriptArguments = await debuggerProperties.RunCommandArguments.GetEvaluatedValueAtEndAsync();

            var generalProperties = await this.ProjectProperties.GetConfigurationGeneralPropertiesAsync();
            string startupItem = await generalProperties.StartItem.GetEvaluatedValueAtEndAsync();

            if ((launchOptions & DebugLaunchOptions.NoDebug) == DebugLaunchOptions.NoDebug)
            {
                // No debug - launch cscript using cmd.exe to introduce a pause at the end
                settings.Executable = Path.Combine(Environment.SystemDirectory, "cmd.exe");
                settings.Arguments = string.Format("/c {0} \"{1}\" {2} & pause", scriptCommand, startupItem, scriptArguments);
            }
            else
            {
                // Debug - launch cscript using the debugger switch //X
                settings.Executable = scriptCommand;
                settings.Arguments = string.Format("\"{0}\" //X {1}", startupItem, scriptArguments);
            }*/

            settings.LaunchOperation = DebugLaunchOperation.CreateProcess;
            settings.LaunchDebugEngineGuid = DebuggerEngines.ScriptEngine;

            return new IDebugLaunchSettings[] { settings };
        }
    }
#endif
}

