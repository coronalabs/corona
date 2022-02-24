'------------------------------------------------------------------------------------
' This script creates a directory containing the contents of a VSIX Extension SDK.
' The created directory is intended to be under an "SDKReferenceDirectoryRoot"
' folder so that a project can reference the SDK without installing it.
'------------------------------------------------------------------------------------

Option Explicit


' Create a file system member variable object.
Dim m_fileSystem
Set m_fileSystem = CreateObject("Scripting.FileSystemObject")

' Fetch command line arguments for paths to VSIX file and output directory.
If (WScript.Arguments.Count <> 2) Then
	' Expected arguments were not provided.
	Dim message
	message = _
		"This script creates the directory structure and copies the contents of a VSIX Extension SDK." & vbCrLf & _
		vbCrLf & _
		"Usage:" & vbCrLf & _
		"CreateSDKReferenceDirectory [VSIXDirectory] [SDKReferenceDirectoryRoot]"
	OutputMessage(message)
	WScript.Quit(1)
End If
Dim inputDirectoryPath
Dim rootOutputDirectoryPath
inputDirectoryPath = WScript.Arguments(0)
rootOutputDirectoryPath = WScript.Arguments(1)

' Do not continue if the given input directory does not exist.
If (m_fileSystem.FolderExists(inputDirectoryPath) = False) Then
	OutputMessage("Directory not found:" & vbCrLf & inputDirectoryPath)
	WScript.Quit(1)
End If

' Fetch a path to the VSIX manifest file.
Dim vsixManifestFilePath
vsixManifestFilePath = m_fileSystem.BuildPath(inputDirectoryPath, "extension.vsixmanifest")
If (m_fileSystem.FileExists(vsixManifestFilePath) = False) Then
	OutputMessage("File not found:" & vbCrLf & vsixManifestFilePath)
	WScript.Quit(1)
End If

' Load the VSIX manifest XML file.
Dim xmlDocument
Set xmlDocument = CreateObject("Microsoft.XMLDOM")
xmlDocument.Async = "False"
xmlDocument.Load(vsixManifestFilePath)
If (xmlDocument.ParseError.ErrorCode <> 0) Then
	OutputMessage(xmlDocument.ParseError.Reason)
	WScript.Quit(1)
End If

' Fetch the "PackageManifest" XML element.
Dim xmlPackageManifestElement
Set xmlPackageManifestElement = xmlDocument.documentElement
If (IsNull(xmlPackageManifestElement)) Then
	WScript.Quit(1)
ElseIf (xmlPackageManifestElement.NodeName <> "PackageManifest") Then
	WScript.Quit(1)
End If

' Fetch Platform and SDK information from the XML.
Dim platformName
Dim platformVersionString
Dim sdkName
Dim sdkVersionString
Dim xmlPackageChildElement
For Each xmlPackageChildElement In xmlPackageManifestElement.ChildNodes
    If (xmlPackageChildElement.NodeName = "Installation") Then
        Dim xmlElement
        For Each xmlElement In xmlPackageChildElement.ChildNodes
            If (xmlElement.NodeName = "InstallationTarget") Then
                platformName = xmlElement.GetAttribute("TargetPlatformIdentifier")
                platformVersionString = xmlElement.GetAttribute("TargetPlatformVersion")
                sdkName = xmlElement.GetAttribute("SdkName")
                sdkVersionString = xmlElement.GetAttribute("SdkVersion")
                Exit For
            End If
        Next
        Exit For
    End If
Next
If (IsNull(platformName) Or IsNull(platformVersionString) Or IsNull(sdkName) Or IsNull(sdkVersionString)) Then
	OutputMessage( _
            "The 'extension.vsixmanifest' file is missing required Extension SDK parameters" & _
            " 'TargetPlatformIdentifier', 'TargetPlatformVersion', 'SdkName', 'SdkVersion'.")
	WScript.Quit(1)
End If

' Build the SDK reference directory path to be outputted below.
Dim sdkOutputDirectoryPath
sdkOutputDirectoryPath = m_fileSystem.BuildPath( _
        rootOutputDirectoryPath, _
        platformName & "\" & platformVersionString & "\ExtensionSDKs\" & sdkName & "\" & sdkVersionString)

' Delete the previously outputted SDK reference directory, if it exists.
If (m_fileSystem.FolderExists(sdkOutputDirectoryPath)) Then
    m_fileSystem.DeleteFolder sdkOutputDirectoryPath, True
End If

' Create the SDK reference directory structure.
CreateFolderTree(sdkOutputDirectoryPath)

' Copy the input directory's contents to the newly created SDK reference directory.
m_fileSystem.CopyFolder m_fileSystem.BuildPath(inputDirectoryPath, "*"), sdkOutputDirectoryPath & "\"
m_fileSystem.CopyFile m_fileSystem.BuildPath(inputDirectoryPath, "*.*"), sdkOutputDirectoryPath & "\"

' The SDK reference directory copy was a success.
OutputMessage("Copied '" & inputDirectoryPath & "' to '" & sdkOutputDirectoryPath & "'." & vbCrLf)


'------------------------------------------------------------------------------------
' General Helper Functions
'------------------------------------------------------------------------------------

' Displays the given message to the user.
' Outputs the message to the command line if script was ran via cscript.exe.
' Outputs the message via a message box if script was ran via wscript.exe.
Function OutputMessage(message)
	If (m_fileSystem.GetFileName(LCase(WScript.FullName)) = "wscript.exe") Then
		MsgBox message
	Else
		WScript.StdOut.WriteLine(message)
	End If
End Function

' Create the entire directory tree for the given path.
Function CreateFolderTree(path)
    If (Len(path) > 0) Then
        If (m_fileSystem.FolderExists(path) = False) Then
            CreateFolderTree(m_fileSystem.GetParentFolderName(path))
            m_fileSystem.CreateFolder(path)
        End If
    End If
End Function
