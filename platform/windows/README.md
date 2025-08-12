# Building for Windows Desktop

## Prerequisites:

You must install the following before building Corona Labs' Win32 software.

- [Visual Studio 2019 Community Edition](https://aka.ms/vs/16/release/vs_community.exe)
  - Select Native Development workload
  - Add individual components:
    - C++ Windows XP Support for VS 2017 (v141) tools [Deprecated]
    - C++ MFC for v141 build tools (x86 & x64)

## How to build "Corona Simulator":

- Double click on "Corona.Simulator.sln" file.

  - Solution file is used to build and debug Corona Simulator software.
  - Outputs to:

        .\platform\windows\bin\Corona
