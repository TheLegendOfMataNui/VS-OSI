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
   - [X] SAGE-JS MSBuild task (!! installation method needs help: the Project definition requires the path to the embedded `OSIProject.Tasks.dll` to be in the registry before it will work!!)
   - [ ] [WIP] Post-build deploy task: Copy the output .osi files to the game directory
   - [ ] [WIP] Launch the game when the run action is activated
   - [ ] Maybe an actual debug engine for stepping through the bytecode live? (Would require a hookmod)
     - [ ] Attach and detatch properly
     - [ ] Pipe game debug output to VS Output Window
     - [ ] Breakpoints
     - [ ] Step one instruction
     - [ ] Browse & modify VM state

Project Overview
----------------

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

`OSIProject.DebugServer` - The SAGE Hookmod that hooks the game's OSI virtual machine an provides debugging capabilities.