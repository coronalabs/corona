echo "building Angle projects and nuget packages..."
rm *.nupkg
echo "building WinRT Angle..."
Start-Process .\winrt_build.bat -wait
Write-NuGetPackage .\angle_winrt.autopkg
echo "building WP8 Angle..."
Start-Process .\wp8_build.bat -wait
Write-NuGetPackage .\angle_wp8.autopkg