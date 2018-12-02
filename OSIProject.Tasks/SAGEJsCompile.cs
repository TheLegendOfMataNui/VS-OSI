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
        /*[Required]
        public ITaskItem[] SourceFiles { get; set; }*/

        [Required]
        public ITaskItem SourceDirectory { get; set; }

        [Required]
        public ITaskItem OutputDirectory { get; set; }

        [Required]
        public ITaskItem OutputFilename { get; set; }

        /*[Required]
        public ITaskItem ProjectDirectory { get; set; }*/

        [Output]
        public ITaskItem[] OutputFiles { get; private set; }

        public override bool Execute()
        {
            //Log.LogWarning("HEY!!!! " + SourceFiles.Length);
            if (!System.IO.Directory.Exists(OutputDirectory.ItemSpec))
                System.IO.Directory.CreateDirectory(OutputDirectory.ItemSpec);
            /*foreach (ITaskItem sourceFile in SourceFiles)
            {
                Log.LogMessage(MessageImportance.High, "Assembling file '" + sourceFile.ItemSpec + "'...");
                bool success = false;
                string resultFilename = Assemble(sourceFile.GetMetadata("FullPath"), out success);
                if (success)
                {
                    Log.LogMessage(MessageImportance.High, sourceFile.ItemSpec + " -> " + resultFilename);
                }
                else
                {

                }
            }*/

            bool success = false;
            /*List<string> sourceFiles = new List<string>();
            foreach (ITaskItem task in SourceFiles)
            {
                sourceFiles.Add(task.GetMetadata("FullPath"));
            }
            string resultFilename = Assemble(sourceFiles.ToArray(), out success);*/
            string resultFilename = Assemble(new string[] { SourceDirectory.GetMetadata("FullPath") }, out success);
            if (success)
            {
                Log.LogMessage(MessageImportance.High, " -> " + resultFilename);
            }
            else
            {

            }

            return !Log.HasLoggedErrors;
        }

        private static string CurrentFilename = "";
        private string Assemble(string[] sourceFilenames, out bool success)
        {
            CurrentFilename = "(Multiple Files)";
            string resultFilename = System.IO.Path.Combine(OutputDirectory.GetMetadata("FullPath"), OutputFilename.ItemSpec);
            //Log.LogMessage(MessageImportance.High, "Output dir: '" + OutputDirectory.GetMetadata("FullPath") + "', combined: '" + resultFilename);

            System.Diagnostics.Process sageJS = new System.Diagnostics.Process();
            sageJS.StartInfo.FileName = FindExePath("sage-js.cmd");
            sageJS.StartInfo.Arguments = "res:osi:asm:sa \"" + resultFilename + "\"";// \"" + resultFilename + "\"";

            foreach (string filename in sourceFilenames)
                sageJS.StartInfo.Arguments += " \"" + filename + "\"";

            Log.LogWarning("Command: '" + sageJS.StartInfo.FileName + " " + sageJS.StartInfo.Arguments + "'");

            sageJS.StartInfo.UseShellExecute = false;
            sageJS.StartInfo.RedirectStandardOutput = true;
            sageJS.StartInfo.RedirectStandardError = true;
            sageJS.OutputDataReceived += SageJS_OutputDataReceived;
            sageJS.ErrorDataReceived += SageJS_ErrorDataReceived;

            sageJS.Start();
            sageJS.BeginErrorReadLine();
            sageJS.BeginOutputReadLine();
            sageJS.WaitForExit();

            int returnCode = sageJS.ExitCode;
            if (returnCode != 0)
            {
                Log.LogError("OSI Assembly Failure!");
            }
            success = returnCode == 0;
            return resultFilename;
        }

        private void SageJS_ErrorDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            string output = e?.Data?.Trim();
            if (output?.Length > 0)
            {
                if (output.StartsWith("at"))
                    return;

                string[] parts = output.Split(new char[] { ':' }, 2);
                if (parts.Length > 1)
                {
                    string exceptionName = parts[0].Trim();
                    string exceptionMessage = parts[1].Trim();
                    if (exceptionMessage.Contains('@'))
                    {
                        string[] location = exceptionMessage.Substring(exceptionMessage.IndexOf('@') + 1).Split(':');
                        int line = Int32.Parse(location[0].Trim());
                        int column = Int32.Parse(location[1].Trim());
                        Log.LogError(exceptionName, "ASM", "", CurrentFilename, line, column, line, column, exceptionMessage.Substring(0, exceptionMessage.IndexOf('@')));
                        return;
                    }
                }
                Log.LogError(e.Data);
            }
        }

        private void SageJS_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            if (e?.Data?.Trim().Length > 0)
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
