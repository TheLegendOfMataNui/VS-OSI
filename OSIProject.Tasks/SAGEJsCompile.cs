using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
//using System.Threading.Tasks;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace OSIProject
{
    public class SAGEJsCompile : Task
    {
        [Required]
        public ITaskItem[] SourceFiles { get; set; }

        [Required]
        public ITaskItem OutputDirectory { get; set; }

        /*[Required]
        public ITaskItem ProjectDirectory { get; set; }*/

        [Output]
        public ITaskItem[] OutputFiles { get; private set; }

        public override bool Execute()
        {
            //Log.LogWarning("HEY!!!! " + SourceFiles.Length);
            if (!System.IO.Directory.Exists(OutputDirectory.ItemSpec))
                System.IO.Directory.CreateDirectory(OutputDirectory.ItemSpec);
            foreach (ITaskItem sourceFile in SourceFiles)
            {
                Log.LogMessage(MessageImportance.High, "Assembling file '" + sourceFile.ItemSpec + "'...");
                string resultFilename = Assemble(sourceFile.GetMetadata("FullPath"));
                Log.LogMessage(MessageImportance.High, sourceFile.ItemSpec + " -> " + resultFilename);
            }
            return !Log.HasLoggedErrors;
        }

        private string Assemble(string sourceFilename)
        {
            string resultFilename = System.IO.Path.Combine(OutputDirectory.GetMetadata("FullPath"), System.IO.Path.ChangeExtension(System.IO.Path.GetFileName(sourceFilename), ".osi"));
            //Log.LogMessage(MessageImportance.High, "Output dir: '" + OutputDirectory.GetMetadata("FullPath") + "', combined: '" + resultFilename);

            System.Diagnostics.Process sageJS = new System.Diagnostics.Process();
            sageJS.StartInfo.FileName = FindExePath("sage-js.cmd");
            sageJS.StartInfo.Arguments = "res:osi:asm:a \"" + sourceFilename + "\" \"" + resultFilename + "\"";

            //Log.LogWarning("Command: '" + sageJS.StartInfo.FileName + " " + sageJS.StartInfo.Arguments + "'");

            sageJS.StartInfo.UseShellExecute = false;
            sageJS.StartInfo.RedirectStandardOutput = true;
            sageJS.StartInfo.RedirectStandardError = true;
            sageJS.OutputDataReceived += SageJS_OutputDataReceived;
            sageJS.ErrorDataReceived += SageJS_ErrorDataReceived;

            sageJS.Start();
            sageJS.WaitForExit();

            return resultFilename;
        }

        private void SageJS_ErrorDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            Log.LogError(" > " + e.Data);
        }

        private void SageJS_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            Log.LogMessage(MessageImportance.High, " > " + e.Data);
        }

        /// <summary>
        /// (from http://csharptest.net/526/how-to-search-the-environments-path-for-an-exe-or-dll/index.html)
        /// Expands environment variables and, if unqualified, locates the exe in the working directory
        /// or the evironment's path.
        /// </summary>
        /// <param name="exe">The name of the executable file</param>
        /// <returns>The fully-qualified path to the file</returns>
        /// <exception cref="System.IO.FileNotFoundException">Raised when the exe was not found</exception>
        public static string FindExePath(string exe)
        {
            exe = Environment.ExpandEnvironmentVariables(exe);
            if (!File.Exists(exe))
            {
                if (Path.GetDirectoryName(exe) == String.Empty)
                {
                    foreach (string test in (Environment.GetEnvironmentVariable("PATH") ?? "").Split(';'))
                    {
                        string path = test.Trim();
                        if (!String.IsNullOrEmpty(path) && File.Exists(path = Path.Combine(path, exe)))
                            return Path.GetFullPath(path);
                    }
                }
                throw new FileNotFoundException(new FileNotFoundException().Message, exe);
            }
            return Path.GetFullPath(exe);
        }
    }
}
