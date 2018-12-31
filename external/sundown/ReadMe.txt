
Version Date from GitHub:
   November 29th, 2012

Downloaded From:
   https://github.com/vmg/sundown

Summary:
Sundown is a fast Markdown-to-HTML converter written in C which has no 3rd party library dependencies.

Used By:
- The Windows Corona Simulator to display server markdown responses via its HtmlMessageDialog.

Modifications Made:
Sundown's source files and header files have been moved from their "src" and "html" subdirectories to a single "sundown" directory by Joshua Quick at Corona Labs. The reason is because the source files use common names such as "html.h" and "stack.h" and we want to avoid naming collision when including headers. So, the recommended way to include headers from sundown is like this...
   #include "sundown/html.h"
