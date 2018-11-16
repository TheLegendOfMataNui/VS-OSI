# VS-OSI
Adds Visual Studio support for working with OSI bytecode (scripts used by the SAGE game engine).

Goals
-----
 - [ ] OSI Assembly language support for `.osa` files
   - [X] Syntax coloring
   - [X] Code folding
   - [X] Code prediction
   - [ ] Instruction signature help
 - [ ] OSI Projects for compiling OSI Assembly into OSI using [SAGE-JS](https://github.com/TheLegendOfMataNui/sage-js)
   - [X] SAGE-JS MSBuild task (Installed to a well-known path by the installer)
   - [X] Launch the game when the run action is activated
   - [ ] Don't rebuild unchanged sources
 - [ ] An actual debug engine for debugging the bytecode live
   - [ ] Attach and detatch properly
   - [ ] Pipe game debug output to VS Output Window
   - [ ] Breakpoints
   - [ ] Step one instruction
   - [ ] Browse & modify VM state

Project Overview
----------------

`Installer.iss` - An installer script that is compiled by [Inno Setup](http://www.jrsoftware.org/isinfo.php) into the `Output/` directory upon build. Requires Inno Setup to be installed. An installer is necessary for installing the MSBuild task.

`OSIProject.Language` - The details of OSI stuff. `OSIAssembly.cs` for OSI Assembly (lexing, tokens, and keywords), and maybe later an `OSIFile.cs` for the actual packed `.osi` format.

`OSIProject.Language.Test` - A few poorly done unit tests for `OSIProject.Language`.

`OSIProject.ProjectTemplate` - The template for creating new Visual Studio OSI Projects.

`OSIProject.ProjectType` - The Visual Studio VSIX extension for OSI Projects, language support, and all the rest of the editor stuff.
<br/>
See `OSIAssemblyLanguageService.cs` for the text editor support for OSI Assembly.
<br/>
See `OSADeploy.cs` for what will eventually be the copy-to-game deploy task.
<br/>
See `ScriptDebuggerLaunchProvider.cs` for the debugger launching stuff.

`OSIProject.Tasks` - The library that holds the MSBuild tasks for compiling OSI material.

`OSIProject.DebugServer` - The SAGE Hookmod that hooks the game's OSI virtual machine and provides debugging capabilities.

`OSIProject.DebugInterop` - The .NET classes for attaching to a debug server.