--*********************************************************************************************
-- ====================================================================
-- Corona AndroidValidation.lua
-- ====================================================================
--
-- File: AndroidValidation.lua
--
-- Version 1.0
--
-- Copyright (C) 2011 ANSCA Inc. All Rights Reserved.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of 
-- this software and associated documentation files (the "Software"), to deal in the 
-- Software without restriction, including without limitation the rights to use, copy, 
-- modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
-- and to permit persons to whom the Software is furnished to do so, subject to the 
-- following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in all copies 
-- or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
-- PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
-- FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
-- OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
-- DEALINGS IN THE SOFTWARE.
--
-- Published changes made to this software and associated documentation and module files (the
-- "Software") may be used and distributed by ANSCA, Inc. without notification. Modifications
-- made to this software and associated documentation and module files may or may not become
-- part of an official software release. All modifications made to the software will be
-- licensed under these same terms and conditions.
--*********************************************************************************************

require('lpeg')
require('re')
require('lfs')

-- The grammar for Android/Java package names.
-- According to what I've found, Android package names follow exactly
-- the same rules as Java pacakage names with the extra condition that
-- you must have two parts (one.two) to help ensure uniquiness. 
-- Also, through experimentation, it appears that $ is not allowed.
-- http://reedmanit.com/wordpress/?p=17
--[[
Rules for Java Identifiers
1) Must start with a letter, ($), or connecting character (_)
2) Can not start with a number
3) No limits to the amount of characters for a identifier
4) You can not use a Java Keyword
5) Case sensitive – IDENTIFER is different from identifier

http://greeenjava.blogspot.com/2009/06/java-identifierrules-and-convention.html

http://www.uni-ulm.de/admin/doku/javaspec/javaspec_11.html
PackageName =
	Identifier 
|	PackageName `.' Identifier
;
--]]
--[[
Implementation notes:

	JavaReservedWord: Just a list of reserved words.

	JaveReservedToken ends with ! characters to prevent matching from picking up words that start with reserved words but continue on, e.g. floaty is a valid token, but float is reserved.

	BaseIdentifier: Java rules allow for the letters, underscore and dollar to start a identifier. It may then be followed by additional letters/underscore/dollars and numbers.

	Identifer - I had trouble with, but this seems to work. First disallow reserved tokens and then immediately follow by the valid tokens.

	PackageIdentifier ends with a !. to make sure the match doesn't allow for stray bad characters following the valid package name.
--]]
local JavaReservedWordString =[[
	JavaReservedWord 	<- 'abstract'
		/ 'assert'
		/ 'boolean'
		/ 'break'
		/ 'byte'
		/ 'case'
		/ 'catch'
		/ 'char'
		/ 'class'
		/ 'const'
		/ 'continue'
		/ 'default'
		/ 'do'
		/ 'double'
		/ 'else'
		/ 'enum'
		/ 'extends'
		/ 'false'
		/ 'final'
		/ 'finally'
		/ 'float'
		/ 'for'
		/ 'goto'
		/ 'if'
		/ 'implements'
		/ 'import'
		/ 'instanceof'
		/ 'int'
		/ 'interface'
		/ 'long'
		/ 'native'
		/ 'new'
		/ 'null'
		/ 'package'
		/ 'private'
		/ 'protected'
		/ 'public'
		/ 'return'
		/ 'short'
		/ 'static'
		/ 'strictfp'
		/ 'super'
		/ 'switch'
		/ 'synchronized'
		/ 'this'
		/ 'throw'
		/ 'throws'
		/ 'transient'
		/ 'true'
		/ 'try'
		/ 'void'
		/ 'volatile'
		/ 'while'
]]

local packageIdentifierGrammar = re.compile([[
	PackageIdentifer 	<- { <Identifier> (<Dot><Identifier>)+ !. }
	Dot 	<- [.]
	Identifier 	<- !<JavaReservedToken> <BaseIdentifier>
	BaseIdentifier 	<- [A-Za-z][A-Za-z0-9_]*
	JavaReservedToken <- <JavaReservedWord>![A-Za-z0-9_]+
]] .. JavaReservedWordString)

-- Same as the packageIdentifier but captures just the basename (up to the first dot)
-- Bug: 4524. We don't want to apply the Java rules to the file extension.
-- Since we already run code through the packageIdentifierGrammar, we can simply this to
-- return just the base file name without applying all the Java package name rules.
-- For multiple dots, Android seems to look only up to the first period.
-- We might want to open up the character set even more.
local baseFileNameIdentifierGrammar = re.compile([[
	PackageIdentifer 	<- { <Identifier> } (<Dot><Identifier>)+ !.
	Dot 	<- [.]
	Identifier 	<- [a-zA-Z0-9_]+
]])

local componentPackageIdentifierGrammar = re.compile([[
	PackageIdentifer 	<- { <Identifier> (<Dot><Identifier>)* !. }
	Dot 	<- [.]
	Identifier 	<- !<JavaReservedToken> <BaseIdentifier>
	BaseIdentifier 	<- [A-Za-z][A-Za-z0-9_]*
	JavaReservedToken <- <JavaReservedWord>![A-Za-z0-9_]+
]] .. JavaReservedWordString)


--[=[ Debug string
local combined_string = [[
	PackageIdentifer 	<- { <Identifier> (<Dot><Identifier>)+ !. }
	Dot 	<- [.]
	Identifier 	<- !<JavaReservedToken> <BaseIdentifier>
	BaseIdentifier 	<- [A-Za-z][A-Za-z0-9_]*
	JavaReservedToken <- <JavaReservedWord>![A-Za-z0-9_]+
]]  .. JavaReservedWordString

print(combined_string)
--]=]


function ValidateAndroidPackageName(packagename)
	if not packagename then
		return false
	end

	local result = packageIdentifierGrammar:match(packagename)
	if result then
		return true
	else
		return false
	end
end

-- Similar to ValidateAndroidPackageName, but works for incomplete package names,
-- for example, if we are looking at just a single word without the dots, e.g.
-- 'explosion' in 'explosion.3gp'. 
-- If there are dots, this will validate like ValidateAndroidPackageName.
function ValidateAndroidPackageComponent(packagename)
	if not packagename then
		return false
	end

	local result = componentPackageIdentifierGrammar:match(packagename)
	if result then
		return true
	else
		return false
	end
end


--[==[ Grammar Unit Tests
print("Performing Grammar Unit Tests")
validIdentifiers = 
{
	'Simple123',
	'aName123__',
}	
invalidIdentifiers = 
{
	'1aName',
	'&aName',
	':)smileyName',
	'-creativeName',
	'________', -- This used to be legal, but we needed to change this to disallow 'foo._'
	'_123', -- This used to be legal, but we needed to change this to disallow 'foo._'
}

validJavaIdentifiers = 
{
	'This',
	'tryif',
	'voiD',
	'WHILEtrue',
	'floaty',
}	

invalidJavaIdentifiers = 
{
	'static',
	'super',
	'while',
	'enum',
}

validPackageNames = 
{
	'com.Simple123',
	'net.aName123__',
	'zig.zag.zog.A________',
	'Apublic.Bprotected',
	'publicA.protectedB',
	'ApublicA.AprotectedB',
	
}

invalidPackageNames = 
{
	'simple123',
	'public.aName123__',
	'123something.other',
	'foo.protected',
	'zig.zag.',
	'',
	'dig..dug',
	'..',
	'..foo.bar',
	'foo.bar._123', -- This used to be legal, but we needed to change this to disallow 'foo._'
	'Abar._',  -- This used to be legal, but we needed to change this to disallow 'foo._'
	'zig.zag.zog.________', -- This used to be legal, but we needed to change this to disallow 'foo._'
}	

-- Test base identifiers without Java keyword restrictions
local baseIdentifierGrammar = re.compile([[
	BaseIdentifier 	<- [A-Za-z][A-Za-z0-9_]*
]])

for i,v in ipairs(validIdentifiers) do
	local ret_val = baseIdentifierGrammar:match(validIdentifiers[i])
	assert(ret_val, "Entry: " .. v .. " at index: " .. i .. " should have returned a match")
end

for i,v in ipairs(invalidIdentifiers) do
	local ret_val = baseIdentifierGrammar:match(invalidIdentifiers[i])
	assert(nil == ret_val, "Entry: " .. v .. " at index: " .. i .. " should NOT have returned a match")
end

-- Test just identifiers
local identifierGrammar = re.compile([[
	Identifier 	<- !<JavaReservedToken> <BaseIdentifier>
	BaseIdentifier 	<- [A-Za-z_][A-Za-z0-9_]*
	JavaReservedToken <- <JavaReservedWord>![A-Za-z0-9_]+
]] .. JavaReservedWordString)

for i,v in ipairs(validJavaIdentifiers) do
	local ret_val = identifierGrammar:match(validJavaIdentifiers[i])
	assert(ret_val, "Entry: " .. v .. " at index: " .. i .. " should have returned a match")
end

for i,v in ipairs(invalidJavaIdentifiers) do
	local ret_val = identifierGrammar:match(invalidJavaIdentifiers[i])
	assert(nil == ret_val, "Entry: " .. v .. " at index: " .. i .. " should NOT have returned a match")
end

-- Fullblown test
for i,v in ipairs(validPackageNames) do
	local ret_val = packageIdentifierGrammar:match(validPackageNames[i])
	assert(ret_val, "Entry: " .. v .. " at index: " .. i .. " should have returned a match")
end

for i,v in ipairs(invalidPackageNames) do
	local ret_val = packageIdentifierGrammar:match(invalidPackageNames[i])
	assert(nil == ret_val, "Entry: " .. v .. " at index: " .. i .. " should NOT have returned a match")
end

print("Passed all Grammar Unit Tests")

--]==]

-- http://lua-users.org/wiki/StringRecipes
function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.ends(String,End)
return End=='' or string.sub(String,-string.len(End))==End
end

-- Files that go in Res are different than files that go in Assets. 
-- Res files must obey Java package rules.
-- Returns true if a Res file.
function IsResFile(file)
	local lowercase_file = string.lower(file)
	if string.ends( lowercase_file, '.ogg' )
		or string.ends( lowercase_file, '.mp3' )
		or string.ends( lowercase_file, '.wav' )
		or string.ends( lowercase_file, '.aiff' )
		or string.ends( lowercase_file, '.aif' )
		or string.ends( lowercase_file, '.mp4' )
		or string.ends( lowercase_file, '.3gp' )
	then
		return true
	else
		return false
	end
end

function IsLuaFile(file)
	if string.ends( string.lower(file), '.lua' )
	then
		return true
	else
		return false
	end
end


-- Builds up an array of files in directories.
-- Passing true for shouldrecurse will traverse subdirectories.
-- The list gets appended to listOfCollectedFiles.
-- listOfCollectedFiles is returned for convenience
-- Each element in the array is a table with the following keys:
-- baseName - Just the file name (with the extension)
-- absolutePath - The entire path with the baseName 
function GetListOfFilesFromDirectory(path, shouldrecurse, listOfCollectedFiles)
	if nil == listOfCollectedFiles then
		listOfCollectedFiles = {}
	end

	for file in lfs.dir(path) do
		if file ~= "." and file ~= ".." then
			-- Ignore files that start with .
			if not string.starts(file, '.') then
				local f = path..'/'..file
				local attr = lfs.attributes (f)
				if attr.mode == "directory" then
					-- recurse
					if shouldrecurse then
						GetListOfFilesFromDirectory(f, shouldrecurse, listOfCollectedFiles)
					end
				else
					table.insert( listOfCollectedFiles, { baseName=file, absolutePath=f } )
				end
			end
		end
	end

	-- return for convenience
	return listOfCollectedFiles
end

-- Validates Res files for Java keywords
function ValidateFilesForJavaKeywordsInDirectory(path)
	local badfiles = {}
	local foundbad = false

	for file in lfs.dir(path) do
		if file ~= "." and file ~= ".." then
			-- Ignore files that start with .
			if not string.starts(file, '.') and IsResFile(file) then
				local f = path..'/'..file
--				print ("\t "..f)
				local attr = lfs.attributes (f)
--				assert (type(attr) == "table")
				if attr.mode == "directory" then
					-- TODO: How do we handle subdirectories since they are officially unsupported? We may need to validate the path for Java reserved words.
					ValidateFilesForJavaKeywordsInDirectory (f)
				else
--[[
					for name, value in pairs(attr) do
						print (name, value)
					end
--]]
					-- This function should work for our purposes
					-- But first we must chop off the trailing extension because Android
					-- doesn't count the file extension. (Android only looks up to the first period.)
					local basename_with_no_ext = baseFileNameIdentifierGrammar:match(file)
					if not ValidateAndroidPackageComponent(basename_with_no_ext) then
						table.insert(badfiles, f)
						foundbad = true
					end
				end
			end
		end
	end

	--[[ for debugging
	for name, value in pairs(badfiles) do
		print("bad:", name, value)
	end
	--]]
	local isvalid = not foundbad

	return isvalid, badfiles
end

-- Validates Res files for duplicate basenames (up to first period)
-- This function does not handle subdirectories because subdirs are not supported.
function ValidateFilesForDuplicateBaseNamesInDirectory(path)
	local badfiles = {}
	local foundbad = false
	local filemap = {}
	local list_of_files = {}

	list_of_files = GetListOfFilesFromDirectory(path, true)

	for i,v in ipairs(list_of_files) do
		
		if IsResFile(v.baseName) then
			local basename_with_no_ext = baseFileNameIdentifierGrammar:match(v.baseName)
			if basename_with_no_ext then
				if nil == filemap[basename_with_no_ext] then
					-- Put the basename as a key in a table so we can look up if there is a duplicate in subsequent passes
					-- The value is the full path of the file so we can report it if need be.
					filemap[basename_with_no_ext] = v.absolutePath
					
				else
					foundbad = true
					table.insert(badfiles, v.absolutePath)
					-- Put both parties in the error table to report unless it has already been listed
					local alreadylisted = false
					for i2, v2 in ipairs(badfiles) do
						if v2 == filemap[basename_with_no_ext] then
							alreadylisted = true
							break
						end
					end
					if not alreadylisted then
						table.insert(badfiles, filemap[basename_with_no_ext])
					end
				end
			end
		end
	end

	local isvalid = not foundbad

	return isvalid, badfiles
end


-- Validates files for being in subdirectories when they shouldn't be.
-- It is assumed Res files and .lua files can't be in subdirectories
-- This function does not handle subdirectories because subdirs are not supported.
function ValidateFilesForForbiddenSubdirectories(path, is_type_to_be_filtered_function)
	local badfiles = {}
	local foundbad = false
	local listofsubdirfiles = {}

	for file in lfs.dir(path) do
		if file ~= "." and file ~= ".." then
			-- Ignore files that start with .
			if not string.starts(file, '.') then
				local f = path..'/'..file
				local attr = lfs.attributes (f)
				if attr.mode == "directory" then
					GetListOfFilesFromDirectory(f, true, listofsubdirfiles)
				end
			end
		end
	end


	for i,v in ipairs(listofsubdirfiles) do
		if is_type_to_be_filtered_function(v.baseName) then
			table.insert(badfiles, v.absolutePath)
			foundbad = true
		end
	end

	local isvalid = not foundbad

	return isvalid, badfiles
end


function ValidateResFilesForForbiddenSubdirectories(path)
	return ValidateFilesForForbiddenSubdirectories(path, IsResFile)
end

function ValidateLuaFilesForForbiddenSubdirectories(path)
	return ValidateFilesForForbiddenSubdirectories(path, IsLuaFile)
end


-- Temporary workaround for iOS App Names until we fix the
-- problem. Current indications are we need to change
-- something in PHP on the server to fix this.
-- Note the dash (-) at the end of the identifier. 
-- It is used to allow things like 'X-ray'. It must be the last character
-- because there is no way to escape with the re module, and it would
-- be interpreted as a range elsewhere.
local restrictedASCIIIdentifierGrammar =  re.compile([[
	PackageIdentifer 	<-  <BaseIdentifier>+ !.
	BaseIdentifier 	<- [A-Za-z0-9!@#*+=~,.|;_' -]
]] )


function ValidateRestrictedASCIIName(packagename)
	if not packagename then
		return false
	end

	local result = restrictedASCIIIdentifierGrammar:match(packagename)
	if result then
		return true
	else
		return false
	end
end

--[==[ Grammar Unit Tests
validASCIIIdentifiers = 
{
	'Hello',
	'Hello World',
	'_______', -- This used to be legal, but we needed to change this to disallow 'foo._',
	'_123', -- This used to be legal, but we needed to change this to disallow 'foo._'
	"Steve's Apps",
	"X-Ray",
}

invalidASCIIIdentifiers = 
{
	'&aName',
	':)smileyName',
	'-$creativeName',
	'-#bad space Name',
	'asd&ads',
	'"asd :',
}

for i,v in ipairs(validASCIIIdentifiers) do
	local ret_val = restrictedASCIIIdentifierGrammar:match(validASCIIIdentifiers[i])
	assert(ret_val, "Entry: " .. v .. " at index: " .. i .. " should have returned a match")
end

for i,v in ipairs(invalidASCIIIdentifiers) do
	local ret_val = restrictedASCIIIdentifierGrammar:match(invalidASCIIIdentifiers[i])
	assert(nil == ret_val, "Entry: " .. v .. " at index: " .. i .. " should NOT have returned a match")
end

print("Passed all Grammar Unit Tests")

--]==]

