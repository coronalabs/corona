//
// pch.h
// Header for standard system include files.
//

#pragma once


#include <windows.h>
#include <collection.h>


/// <summary>
///  <para>Compiler warning code indicating that a "///" XML code comment is malformed.</para>
///  <para>
///   This constant is expected to be used by a "#pragma warning disable" macro to disable this warning
///   when #including headers that do not use XML comments.
///  </para>
/// </summary>
#define Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE 4635

/// <summary>
///  <para>Disables compiler warning caused by "///" code comments that are not using Microsoft's XML comment format.</para>
///  <para>This macro is excepted to be used before #including core Rtt headers and 3rd party headers.</para>
///  <para>You must use the Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END macro after using this macro.</para>
/// </summary>
#define Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN \
	__pragma(warning(push)) \
	__pragma(warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE))

/// <summary>
///  <para>Re-enables compiler warnings for malformed "///" XML code comments.</para>
///  <para>This macro is expected to be used after the Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN macro.</para>
/// </summary>
#define Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END __pragma(warning(pop))

