local params =
{
	platform    = 'win32',
	appName     = 'Fishies',
	appVersion  = '1.0',
	dstPath     = 'C:\\Users\\XXXX\\Desktop',
	projectPath = 'C:\\CoronaSDK\\SampleCode\\Graphics\\Fishies',

	-- Optional: name of the output .exe file.
	-- Defaults to appName + '.exe'.
	-- exeFileName = 'Fishies.exe',

	-- Optional: Windows VERSIONINFO resource fields embedded in the .exe.
	-- companyName    = 'My Company',
	-- copyright      = 'Copyright (C) 2024 My Company',
	-- appDescription = 'A Solar2D sample application',
	-- versionString  = '1.0.0.0',

	-- Optional: override the build number embedded in the binary.
	-- customBuildId = '12345',
}

return params
