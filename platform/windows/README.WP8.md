
**IMPORTANT:** This is obsolete guide

## Prerequisites:
You must install the following before building Corona Labs' WP8 software.
- Visual Studio 2013 Pro
  * Make sure install feature:  "Windows Phone 8.0 SDK"
- SandCastleHelpFileBuilder
  * Installer can be found at:  main-tachyon\tools\SandCastleHelpFileBuilder
  * Only needed to build the "Corona.Cards.Phone8.0.sln" solution.
  * Used to generate HTML/HelpFile documentation from XML code comments.


How to build WP8 CoronaCards app template:
- Double click on "Corona.AppTemplate.Phone8.0.sln" file.
  * Solution file is used to build and debug Corona WP8 app template.
  * Intended for quick testing without having to build the CoronaCards Visual Studio extension.
  * Native C/C++/CX code can be found in project:
    > Corona.Component.Library
  * .NET code can be found in projects:
    > Corona.DotNet.Library
    > Corona.Controls.DotNet.Library
      * Needs to be a separate library to work in Visual Studio's UI designer.
  * .NET Silverlight application project:
    > Corona.AppTemplate.Phone8.0
  * How to test a Corona project:
    1) Go to the "Solution Explorer" panel in Visual Studio.
    2) Expand the "Corona.AppTemplate.Phone8.0" project to reveal the "Assets\Corona" directory.
    3) Drag and drop a Corona project to "Assets\Corona" as documented here...
       > https://docs.coronalabs.com/daily/coronacards/wp8/portapp.html#copying-project-files
    (Note: This folder is not "hg ignored". So don't check in files committed here.)


How to build the WP8 CoronaCards Visual Studio extension:
- Double click on "Corona.Cards.Phone8.0.sln" file.
  * Make sure you have Internet access. Needed to do digital signing.
  * Do a "Release|Mixed Platforms" Rebuild.
  * Outputs "CoronaCards.WindowsPhone.zip" to:
    > .\platform\windows\bin\CoronaCards
