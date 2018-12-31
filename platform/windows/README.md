
# Building for Windows Desktop

## Prerequisites:
You must install the following before building Corona Labs' Win32 software.
- Visual Studio 2013 Pro
- WiX Toolset v3.10  (Needed to build the install program)
  * Installer can be found at:  corona\tools\Wix


## How to build "Corona Simulator":
- Double click on "Corona.Simulator.sln" file.
  * Solution file is used to build and debug Corona Simulator software.
  * Outputs to:

        .\platform\windows\bin\Corona
  
  * Pre-build step will also build the following:

        Corona.Shell.exe  (Runs a Corona project in Win32 desktop app mode.)
        Corona.OutputViewer.exe  (Logs output from the simulator.)
        Win32 desktop app template  (Needed by the simulator to do local Win32 builds.)

## How to build "Corona Shell" and "Corona Win32 app template":
- Double click on "Corona.Shell.sln" file.
  * Used to build:

        Corona.Shell.exe
        Win32 desktop app template
        CoronaLabs.Corona.Native.dll

  * Intended to quickly debug Corona while running under Win32 desktop app mode.
  * This build does not use the Rtt_AUTHORING_SIMULATOR definition.
  * Outputs "Corona.Shell.exe" to:

        .\platform\windows\bin\Corona

  * Outputs Win32 app template to:

        .\platform\windows\bin\AppTemplates\Win32

###  How to build "Corona Install Program":
- This requires `platform/windows/Build.Tools/CoronaLabsInc.pfx` to be in place to code sign the distribution
- Double click on the "Corona.SDK.sln" file.
  * Used to build the Corona install program.
  * Outputs MSI install program to:

        .\platform\windows\bin\Corona.SDK.Installer
  
  * Outputs Win32 Corona Enterprise files to:
  
        .\platform\windows\bin\Corona.Enterprise
  
  * When building a "Release" version:
      * Prebuild step will build clean release version of:
      * Corona Simulator, Corona Shell, Win32 app template, Win32 Corona Enterprise libraries.
        This is what Jenkins build server uses.
  * When building a "Debug" version:
      * Prebuild step will *not* build Corona Simulator or other Win32 apps.
        Instead, assumes Win32 apps are already built to the "bin" directory.
        Intended for fast testing of different installation program features.
