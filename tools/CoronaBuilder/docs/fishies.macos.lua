local params =
{
	platform    = 'osx',
	appName     = 'Fishies',
	appVersion  = '1.0',
	dstPath     = '/Users/XXXX/Desktop',
	projectPath = '/Applications/CoronaSDK/SampleCode/Graphics/Fishies',

	-- Optional: Xcode developer tools path.
	-- Defaults to the active Xcode installation (xcode-select -p).
	-- sdkPath = '/Applications/Xcode.app/Contents/Developer',

	-- Optional: override the build number embedded in the binary.
	-- customBuildId = '12345',

	-- ── Signing ────────────────────────────────────────────────────────────────
	--
	-- Option A — Provision file (same as iOS).
	--   CoronaBuilder reads the signing identity from the profile automatically.
	--   Use for Mac App Store distribution.
	--
	-- certificatePath = '/Users/XXXX/Library/MobileDevice/Provisioning Profiles/XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX.provisionprofile',

	-- Option B — Direct identity strings (for Developer ID / self-distribution).
	--   Use the certificate name exactly as shown in Keychain Access.
	--   Required when distributionMethod = 'developer-id' or 'developer-id-dmg'.
	--
	-- appSigningIdentity       = 'Developer ID Application: Your Name (TEAMID)',
	-- installerSigningIdentity = 'Developer ID Installer: Your Name (TEAMID)',

	-- ── Distribution method ────────────────────────────────────────────────────
	--
	-- 'developer'        — (default) standard .app, unsigned or developer-signed
	-- 'app-store'        — Mac App Store .pkg (upload manually via Transporter)
	-- 'developer-id'     — Developer ID signed .pkg for direct distribution
	-- 'developer-id-dmg' — Developer ID signed .dmg for direct distribution
	--
	-- distributionMethod = 'developer',
}

return params
