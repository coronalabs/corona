
# Building for Windows Desktop

## Prerequisites:
You must install the following before building Corona Labs' Win32 software.
- Visual Studio 2019 Community Edition
  * Make sure to install C++ components for 140 toolset, as well as 141 CLI, ATL, MFC and XP Tools
- WiX Toolset v3.10  (Needed to build the install program)
  * Installer can be found at:  corona\tools\Wix


## How to build "Corona Simulator":
- Double click on "Corona.Simulator.sln" file.
  * Solution file is used to build and debug Corona Simulator software.
  * Outputs to:

        .\platform\windows\bin\Corona
  