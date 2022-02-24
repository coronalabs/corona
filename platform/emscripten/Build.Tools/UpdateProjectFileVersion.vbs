'------------------------------------------------------------------------------------
' This script replaces the version numbers in a Windows project with arguments
' specified at the command line. Intended to be invoked by an automated build system.
'------------------------------------------------------------------------------------


' Declare member variables.
Dim m_filePath
Dim m_copyrightString
Dim m_majorVersion
Dim m_minorVersion
Dim m_buildNumber
Dim m_revisionNumber
Dim m_fileSystem

' Initialize member variables. (The below functions depend on these.)
m_copyrightString = "Copyright © 2009 - " & Year(Now()) & " Corona Labs Inc."
m_majorVersion = 1
m_minorVersion = 0
m_buildNumber = 0 'DateDiff("d", "01/01/2000", Now())
m_revisionNumber = 0
Set m_fileSystem = CreateObject("Scripting.FileSystemObject")

' Fetch command line arguments for version numbers.
If (WScript.Arguments.Count <> 4) Then
	' Expected arguments were not provided.
	Dim message
	message = _
		"This script updates the version and copyright of 1 Visual Studio project file." & vbCrLf & _
		"The following file types are supported:" & vbCrLf & _
		"- C# Assembly Attributes ""AssemblyInfo.cs""" & vbCrLf & _
		"- C++ Resource File ""*.rc""" & vbCrLf & _
		"- Visual Studio C# Project ""*.vcproj""" & vbCrLf & _
		"- Visual Studio VB.NET Project ""*.vbproj""" & vbCrLf & _
		"- Visual Studio Setup Project ""*.vdproj""" & vbCrLf & _
		"- Visual Studio Extension Manifest ""*.vsixmanifest""" & vbCrLf & _
		"- WiX Installer XML Source ""*.wxs""" & vbCrLf & _
		vbCrLf & _
		"Usage:" & vbCrLf & _
		"UpdateProjectFileVersion [Path\FileName] [MajorVersion] [MinorVersion] [BuildNumber]"
	OutputMessage(message)
	WScript.Quit(1)
End If
m_filePath = WScript.Arguments(0)
m_majorVersion = WScript.Arguments(1)
m_minorVersion = WScript.Arguments(2)
m_buildNumber = WScript.Arguments(3)
ValidateFile m_filePath                             ' Quits if invalid.
ValidateVersion "Major version", m_majorVersion     ' Quits if invalid.
ValidateVersion "Minor version", m_minorVersion     ' Quits if invalid.
ValidateVersion "Build number", m_buildNumber       ' Quits if invalid.

' Update the given file according to its extension.
Dim lowerCaseExtensionName
lowerCaseExtensionName = LCase(m_fileSystem.GetExtensionName(m_filePath))
If (lowerCaseExtensionName = "cs") Then
    UpdateCSharpFile
ElseIf (lowerCaseExtensionName = "rc") Then
	UpdateCppResourceFile
ElseIf (lowerCaseExtensionName = "vdproj") Then
	UpdateSetupProjectFile
ElseIf (lowerCaseExtensionName = "csproj") Then
    UpdateDotNetProjectFile
ElseIf (lowerCaseExtensionName = "vbproj") Then
    UpdateDotNetProjectFile
ElseIf (lowerCaseExtensionName = "vsixmanifest") Then
    UpdateVsixManifestFile
ElseIf (lowerCaseExtensionName = "wxs") Then
    UpdateWixFile
Else
	OutputMessage("Unable to update the given file because it is of an unknown file type.")
	WScript.Quit(1)
End If

' Update was successful.
OutputMessage( _
	"Successfully updated version and copyright information for..." & vbCrLf & _
	"  " & m_filePath & vbCrLf)


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
		'WScript.Echo message
	End If
End Function

' Check if the given file exists.
' If not, then an error message gets outputted and the script exits out.
Function ValidateFile(filePath)
	If (m_fileSystem.FileExists(filePath) = False) Then
		OutputMessage("File not found:" & vbCrLf & filePath)
		WScript.Quit(1)
	End If
End Function

' Checks if the given "versionNumber" argument is a valid version number.
' If not, then an error message gets outputted and the script exits out.
Function ValidateVersion(versionName, versionNumber)
	' Check if the version is numeric.
	If (IsNumeric(versionNumber) = False) Then
		OutputMessage(versionName & " must be numeric.")
		WScript.Quit(1)
	End If

	' Check if the version is out of range.
	If (versionNumber < 0) Then
		OutputMessage(versionName & " cannot be less than 0.")
		WScript.Quit(1)
	End If
End Function

' Determines if the given file is encoded in UTF-16.
' This is determined by checking the first 2 bytes in the file for a Windows BOM signature.
Function IsFileUtf16(filePath)
	Dim fileObject
	
	' Set up error handling.
	On Error Resume Next
	
	' Open the file for reading.
	Err.Clear
	Set fileObject = m_fileSystem.OpenTextFile(filePath, 1, False)
	If (Err.Number <> 0) Then
		OutputMessage(Err.Description)
		WScript.Quit(1)
	End If
	
    ' Read the BOM signature in the file. It is little endian UTF-16 if the signature is 0xFFFE.
    IsFileUtf16 = False
    If ((Asc(fileObject.read(1)) = 255) And (Asc(fileObject.read(2)) = 254)) Then
        IsFileUtf16 = True
    End If

	' Close the file.
	fileObject.Close
End Function

' Writes all of the strings in the given collection to the specified file.
' This script will quit out if an error has been encountered during the write.
Function WriteToFile(filePath, stringCollection, isUtf16)
	Dim fileObject
    Dim fileFormatType
	Dim index
	
	' Set up error handling.
	On Error Resume Next
	Err.Clear
	
	' Open the file for writing.
    fileFormatType = 0
    If (isUtf16) Then
        fileFormatType = -1
    End If
	Set fileObject = m_fileSystem.OpenTextFile(filePath, 2, True, fileFormatType)
	If (Err.Number <> 0) Then
		OutputMessage(Err.Description)
		WScript.Quit(1)
	End If
	
	' Write each string in the collection to file.
	For index = 0 To (stringCollection.Count() - 1)
		fileObject.Write stringCollection.GetString(index)
		If (Err.Number <> 0) Then
			OutputMessage(Err.Description)
			fileObject.Close
			WScript.Quit(1)
		End If
	Next
	
	' Close the file.
	fileObject.Close
End Function

'------------------------------------------------------------------------------------
' Version Updating Functions
'------------------------------------------------------------------------------------

' Changes the version number and copyright year in a C++ project's resource file.
' Member variable "m_filePath" must be set to the path and file name of the *.rc file to update.
Function UpdateCppResourceFile()
	Dim updatedTextCollection
	Dim updatedText
	Dim lineOfText
	Dim stringKey
	Dim fileObject
    Dim fileFormatType
	Dim index
	Dim wasLineUpdated
    Dim isUtf16
	
	On Error Resume Next
	
    ' Determine if the given file is ASCII or UTF-16 encoded.
    isUtf16 = IsFileUtf16(m_filePath)

	' Read the given resource file.
	Err.Clear
    fileFormatType = 0
    If (isUtf16) Then
        fileFormatType = -1
    End If
	Set fileObject = m_fileSystem.OpenTextFile(m_filePath, 1, False, fileFormatType)
	If (Err.Number <> 0) Then
		OutputMessage(Err.Description)
		WScript.Quit(1)
	End If
	Set updatedTextCollection = New StringCollection
	Do Until fileObject.AtEndOfStream
		' Read the next line of text from file.
		lineOfText = fileObject.ReadLine
		wasLineUpdated = False
		
		' Update line if it contains a file version numeric field.
		If (wasLineUpdated = False) Then
			stringKey = "FILEVERSION"
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						" " & BuildVersionStringWithSeparator(",")
				wasLineUpdated = True
			End If
		End If
		
		' Update line if it contains a file version string.
		If (wasLineUpdated = False) Then
			stringKey = """FileVersion"""
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						", """ & BuildVersionStringWithSeparator(".") & """"
				wasLineUpdated = True
			End If
		End If
		
		' Update line if it contains a product version string.
		If (wasLineUpdated = False) Then
			stringKey = """ProductVersion"""
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						", """ & BuildVersionStringWithSeparator(".") & """"
				wasLineUpdated = True
			End If
		End If
		
		' Update line if it contains a copyright string.
		If (wasLineUpdated = False) Then
			stringKey = """LegalCopyright"""
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						", """ & m_copyrightString & """"
				wasLineUpdated = True
			End If
		End If
		
		' The read line does not need to change. Keep it as is.
		If (wasLineUpdated = False) Then
			updatedText = lineOfText
		End If
		
		' Add the text to the collection to be written to file later.
		updatedTextCollection.Add(updatedText & vbCrLf)
	Loop
	fileObject.Close
	
	' Overwrite the given file with the new updates.
	WriteToFile m_filePath, updatedTextCollection, isUtf16
End Function

' Changes the version number, copyright string, and GUIDs in a Visual Studio Setup project file.
' Member variable "m_filePath" must be set to the path and file name of the *.vdproj file to update.
Function UpdateSetupProjectFile()
	Dim updatedTextCollection
	Dim updatedText
	Dim lineOfText
	Dim stringKey
	Dim fileObject
    Dim fileFormatType
	Dim typeLib
	Dim index
	Dim isReadingProductSection
	Dim wasLineUpdated
    Dim isUtf16
	
	On Error Resume Next
	
	' Initialize variables.
	Set updatedTextCollection = New StringCollection
	isReadingProductSection = False
	
    ' Determine if the given file is ASCII or UTF-16 encoded.
    isUtf16 = IsFileUtf16(m_filePath)

	' Read the given file.
	Err.Clear
    fileFormatType = 0
    If (isUtf16) Then
        fileFormatType = -1
    End If
	Set fileObject = m_fileSystem.OpenTextFile(m_filePath, 1, False, fileFormatType)
	If (Err.Number <> 0) Then
		OutputMessage(Err.Description)
		WScript.Quit(1)
	End If
	Do Until fileObject.AtEndOfStream
		' Read the next line of text from file.
		lineOfText = fileObject.ReadLine
		wasLineUpdated = False
		
		' Update the read line if applicable.
		If (isReadingProductSection) Then
			' Check if the Product section of the file has ended.
			If (Right(lineOfText, 1) = "}") Then
				isReadingProductSection = False
			End If
			If (isReadingProductSection) Then
				' Update line if it contains a version string.
				If (wasLineUpdated = False) Then
					stringKey = """ProductVersion"""
					index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
					If (index > 0) Then
						updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
								" = ""8:" & m_majorVersion & "." & m_minorVersion & "." & m_buildNumber & _
								""""
						wasLineUpdated = True
					End If
				End If
				
				' Update line if it contains a product code GUID.
				If (wasLineUpdated = False) Then
					stringKey = """ProductCode"""
					index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
					If (index > 0) Then
						Set typeLib = CreateObject("Scriptlet.TypeLib")
						updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
								" = ""8:" & Left(typeLib.Guid, 38) & """"
						wasLineUpdated = True
					End If
				End If
				
				' Update line if it contains a package code GUID.
				If (wasLineUpdated = False) Then
					stringKey = """PackageCode"""
					index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
					If (index > 0) Then
						Set typeLib = CreateObject("Scriptlet.TypeLib")
						updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
								" = ""8:" & Left(typeLib.Guid, 38) & """"
						wasLineUpdated = True
					End If
				End If
			End If
		Else
			' Check if we're now reading the "Product { }" section of the file.
			' This section contains the version information for the setup project.
			index = InStr(1, lineOfText, """Product""", vbBinaryCompare)
			If (index > 0) Then
				isReadingProductSection = True
			End If
		End If
		
		' The read line does not need to change. Keep it as is.
		If (wasLineUpdated = False) Then
			updatedText = lineOfText
		End If
		
		' Add the text to the collection to be written to file later.
		updatedTextCollection.Add(updatedText & vbCrLf)
	Loop
	fileObject.Close
	
	' Overwrite the given file with the new updates.
	WriteToFile m_filePath, updatedTextCollection, isUtf16
End Function

' Changes the version number and copyright string in a C# file. Typically an "AssemblyInfo.cs" file.
' Member variable "m_filePath" must be set to the path and file name of the *.vsixmanifest file to update.
Function UpdateCSharpFile()
	Dim updatedTextCollection
    Dim updatedText
    Dim lineOfText
	Dim fileObject
    Dim fileFormatType
    Dim index
    Dim wasLineUpdated
    Dim isUtf16
	
	On Error Resume Next
	
    ' Determine if the given file is ASCII or UTF-16 encoded.
    isUtf16 = IsFileUtf16(m_filePath)

	' Read the given C# file.
	Err.Clear
    fileFormatType = 0
    If (isUtf16) Then
        fileFormatType = -1
    End If
	Set fileObject = m_fileSystem.OpenTextFile(m_filePath, 1, False, fileFormatType)
	If (Err.Number <> 0) Then
		OutputMessage(Err.Description)
		WScript.Quit(1)
	End If
	Set updatedTextCollection = New StringCollection
	Do Until fileObject.AtEndOfStream
		' Read the next line of text from file.
		lineOfText = fileObject.ReadLine
		wasLineUpdated = False
		
		' Update line if it contains an "AssemblyVersion" attribute.
		If (wasLineUpdated = False) Then
			stringKey = "AssemblyVersion("
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						"""" & BuildVersionStringWithSeparator(".") & """)]"
				wasLineUpdated = True
			End If
		End If
		
		' Update line if it contains an "AssemblyFileVersion" attribute.
		If (wasLineUpdated = False) Then
			stringKey = "AssemblyFileVersion("
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						"""" & BuildVersionStringWithSeparator(".") & """)]"
				wasLineUpdated = True
			End If
		End If

		' Update line if it contains an "AssemblyCopyright" attribute.
		If (wasLineUpdated = False) Then
			stringKey = "AssemblyCopyright("
			index = InStr(1, lineOfText, stringKey, vbBinaryCompare)
			If (index > 0) Then
				updatedText = Mid(lineOfText, 1, (index + Len(stringKey)) - 1) & _
						"""" & m_copyrightString & """)]"
				wasLineUpdated = True
			End If
		End If

        ' If no text replacement was made up above, then keep the line as is.
		If (wasLineUpdated = False) Then
			updatedText = lineOfText
		End If
		
		' Add the text to the collection to be written to file later.
		updatedTextCollection.Add(updatedText & vbCrLf)
    Loop
	fileObject.Close
	
	' Overwrite the given file with the new updates.
	WriteToFile m_filePath, updatedTextCollection, isUtf16
End Function

' Changes the version number in a Visual Studio Extension manifest file.
' Member variable "m_filePath" must be set to the path and file name of the file to update.
Function UpdateVsixManifestFile()
    Dim xmlDocument
    Dim xmlPackageManifestNode
    Dim xmlElement
    Dim stringValue
    Dim index
    Dim wasUpdated
	
	On Error Resume Next
	
    ' Load the XML file.
    Set xmlDocument = CreateObject("Microsoft.XMLDOM")
    xmlDocument.Async = "False"
    xmlDocument.Load(m_filePath)
	If (xmlDocument.ParseError.ErrorCode <> 0) Then
		OutputMessage(xmlDocument.ParseError.Reason)
		WScript.Quit(1)
	End If

    ' Fetch the "PackageManifest" XML node.
    Set xmlPackageManifestNode = xmlDocument.documentElement
    If (IsNull(xmlPackageManifestNode)) Then
		WScript.Quit(1)
    ElseIf (xmlPackageManifestNode.NodeName <> "PackageManifest") Then
		WScript.Quit(1)
    End If

    ' Traverse the XML child nodes and update all Corona related version numbers.
    wasUpdated = False
    For Each xmlPackageChildNode In xmlPackageManifestNode.ChildNodes
        If (xmlPackageChildNode.NodeName = "Metadata") Then
            ' Update the version number under the "Metadata\Identity" section.
            For Each xmlElement In xmlPackageChildNode.ChildNodes
                If (xmlElement.NodeName = "Identity") Then
                    xmlElement.SetAttribute "Version", BuildVersionStringWithSeparator(".")
                    wasUpdated = True
                End If
            Next
        ElseIf (xmlPackageChildNode.NodeName = "Installation") Then
            ' Update the extension SDK's version number, if it has one. We only do this if:
            ' - An attribute of "SdkName" exists, meaning this is a VSIX file for an Extension SDK.
            ' - The "SdkName" contains the word "Corona".
            For Each xmlElement In xmlPackageChildNode.ChildNodes
                If (xmlElement.NodeName = "InstallationTarget") Then
                    stringValue = xmlElement.GetAttribute("SdkName")
                    If (IsNull(stringValue) = False) Then
			            index = InStr(1, stringValue, "Corona", vbBinaryCompare)
                        If (index > 0) Then
                            xmlElement.SetAttribute "SdkVersion", BuildVersionStringWithSeparator(".")
                            wasUpdated = True
                        End If
                    End If
                End If
            Next
        ElseIf (xmlPackageChildNode.NodeName = "Dependencies") Then
            For Each xmlElement In xmlPackageChildNode.ChildNodes
                If (xmlElement.NodeName = "Dependency") Then
                    stringValue = xmlElement.GetAttribute("Id")
                    If (IsNull(stringValue) = False) Then
			            index = InStr(1, stringValue, "Corona", vbBinaryCompare)
                        If (index > 0) Then
                            xmlElement.SetAttribute "Version", BuildVersionStringWithSeparator(".")
                            wasUpdated = True
                        End If
                    End If
                End If
            Next
        End If
    Next

    ' Overwrite the file if the XML was updated up above.
    If (wasUpdated) Then
        xmlDocument.Save m_filePath
    End If
End Function

' Updates a "Corona Framework" reference in a Visual Studio *.csproj or *.vbproj file.
' Intended to be used on Corona App Project Templates.
' Member variable "m_filePath" must be set to the path and file name of the file to update.
Function UpdateDotNetProjectFile()
    Dim xmlDocument
    Dim xmlProjectNode
    Dim xmlProjectChildNode
    Dim xmlElement
    Dim stringArray
    Dim stringValue
    Dim index
    Dim wasUpdated
	
	On Error Resume Next
	
    ' Load the XML file.
    Set xmlDocument = CreateObject("Microsoft.XMLDOM")
    xmlDocument.Async = "False"
    xmlDocument.Load(m_filePath)
	If (xmlDocument.ParseError.ErrorCode <> 0) Then
		OutputMessage(xmlDocument.ParseError.Reason)
		WScript.Quit(1)
	End If

    ' Fetch the "Project" XML node.
    Set xmlProjectNode = xmlDocument.documentElement
    If (IsNull(xmlProjectNode)) Then
		WScript.Quit(1)
    ElseIf (xmlProjectNode.NodeName <> "Project") Then
		WScript.Quit(1)
    End If

    ' Traverse the XML child nodes and update all Corona related version numbers.
    wasUpdated = False
    For Each xmlProjectChildNode In xmlProjectNode.ChildNodes
        If (xmlProjectChildNode.NodeName = "ItemGroup") Then
            For Each xmlElement In xmlProjectChildNode.ChildNodes
                If (xmlElement.NodeName = "SDKReference") Then
                    ' If referencing a "Corona Framework" SDK, then update its version number.
                    stringValue = xmlElement.GetAttribute("Include")
                    If (IsNull(stringValue) = False) Then
                        If (InStr(1, stringValue, "Corona", vbBinaryCompare) > 0) Then
                            ' Traverse the attribute's comma separated strings and update the version section.
                            stringArray = Split(stringValue, ",", -1, vbBinaryCompare)
                            stringValue = ""
                            For index = 0 To UBound(stringArray)
                                If (index > 0) Then
                                    stringValue = stringValue & ","
                                End If
                                If (InStr(1, stringArray(index), "Version", vbBinaryCompare) > 0) Then
                                    stringValue = stringValue & " Version=" & BuildVersionStringWithSeparator(".")
                                Else
                                    stringValue = stringValue & stringArray(index)
                                End If
                            Next
                            xmlElement.SetAttribute "Include", stringValue
                            wasUpdated = True

                            ' Strip off the version number from the reference's name if present.
                            For Each xmlChildElement In xmlElement.ChildNodes
                                If (xmlChildElement.NodeName = "Name") Then
                                    stringValue = xmlChildElement.text
                                    index = InStr(1, stringValue, " %28", vbBinaryCompare)
                                    If (index > 1) Then
                                        xmlChildElement.text = Mid(stringValue, 1, index - 1)
                                    End If
                                End If
                            Next
                        End If
                    End If
                End If
            Next
        End If
    Next

    ' Overwrite the file if the XML was updated up above.
    If (wasUpdated) Then
        xmlDocument.Save m_filePath
    End If
End Function

' Updates an XML <Product> tag's "Version" attribute in a WiX installer file.
' Member variable "m_filePath" must be set to the path and file name of the file to update.
Function UpdateWixFile()
    Dim xmlDocument
    Dim xmlWixNode
    Dim xmlElement
    Dim wasUpdated
	
	On Error Resume Next
	
    ' Load the XML file.
    Set xmlDocument = CreateObject("Microsoft.XMLDOM")
    xmlDocument.Async = "False"
    xmlDocument.Load(m_filePath)
	If (xmlDocument.ParseError.ErrorCode <> 0) Then
		OutputMessage(xmlDocument.ParseError.Reason)
		WScript.Quit(1)
	End If

    ' Fetch the "Wix" XML node.
    Set xmlWixNode = xmlDocument.documentElement
    If (IsNull(xmlWixNode)) Then
		WScript.Quit(1)
    ElseIf (xmlWixNode.NodeName <> "Wix") Then
		WScript.Quit(1)
    End If

    ' Find the "Product" XML child node and update its version attribute.
    wasUpdated = False
    For Each xmlElement In xmlWixNode.ChildNodes
        If (xmlElement.NodeName = "Product") Then
            xmlElement.SetAttribute "Version", m_majorVersion & "." & m_minorVersion & "." & m_buildNumber
            wasUpdated = True
        End If
    Next

    ' Overwrite the file if the XML was updated up above.
    If (wasUpdated) Then
        xmlDocument.Save m_filePath
    End If
End Function

' Returns a version string with the given separator string.
' The argument is expected to be "." or ",".
' Version numbers are taken from member variables.
Function BuildVersionStringWithSeparator(separatorString)
	BuildVersionStringWithSeparator = _
				m_majorVersion & separatorString & _
				m_minorVersion & separatorString & _
				m_buildNumber & separatorString & _
				m_revisionNumber
End Function


'------------------------------------------------------------------------------------
' Classes
'------------------------------------------------------------------------------------

' Stores a collection of strings.
Class StringCollection
	Dim m_array 	    ' The array that stores the strings.
	Dim m_itemCount	    ' The number of strings in the collection.

	' Constructor. Initializes member variables.
	Private Sub Class_Initialize()
		m_itemCount = 0
		ReDim m_array(64)
	End Sub
	
	' Adds the given string to the end of the collection.
	Public Sub Add(ByVal string)
		If (m_itemCount > UBound(m_array)) Then
			ReDim Preserve m_array(UBound(m_array) + 64)
		End If
		m_array(m_itemCount) = string
		m_itemCount = m_itemCount + 1
	End Sub
	
	' Gets the number of strings contained in this collection.
	Public Function Count()
		Count = m_itemCount
	End Function
	
	' Fetches a string from this collection by its 0 based index.
	Public Function GetString(index)
		GetString = m_array(index)
	End Function
End Class
