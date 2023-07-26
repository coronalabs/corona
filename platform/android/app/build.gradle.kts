import com.android.ide.common.util.toPathString
import com.beust.klaxon.JsonArray
import com.beust.klaxon.JsonObject
import com.beust.klaxon.Klaxon
import com.beust.klaxon.Parser
import com.beust.klaxon.lookup
import org.apache.commons.io.output.ByteArrayOutputStream
import org.apache.tools.ant.filters.StringInputStream

plugins {
    id("com.android.application")
}

//<editor-fold desc="Utility Variables Setup" defaultstate="collapsed">

val coronaResourcesDir: String? by project
val coronaDstDir: String? by project
val coronaTmpDir: String? by project
val coronaAppFileName: String? by project
val coronaAppPackage = project.findProperty("coronaAppPackage") as? String ?: "com.corona.app"
val coronaKeystore: String? by project
val coronaKeystorePassword: String? by project
val coronaKeyAlias: String? by project
val coronaKeyAliasPassword: String? by project
val configureCoronaPlugins: String? by project
val coronaBuild: String? by project
val (dailyBuildYear, dailyBuildRevision) = coronaBuild?.split('.') ?: listOf(null, null)
val coronaBuildData: String? by project
val coronaExpansionFileName: String? by project
val coronaCustomHome: String? by project
val coronaTargetStore = project.findProperty("coronaTargetStore") as? String ?: "none"
val isLiveBuild = project.findProperty("coronaLiveBuild") == "YES"
val isExpansionFileRequired = !coronaExpansionFileName.isNullOrEmpty() && !isLiveBuild
val coronaSrcDir = project.findProperty("coronaSrcDir") as? String
        ?: if (file("$rootDir/../test/assets2").exists()) {
            "$rootDir/../test/assets2"
        } else {
            "$rootDir/../Corona"
        }
val coronaBuiltFromSource = file("CMakeLists.txt").exists() && file("../sdk").exists()

val windows = System.getProperty("os.name").toLowerCase().contains("windows")
val linux = System.getProperty("os.name").toLowerCase().contains("linux")
val shortOsName = if (windows) "win" else if (linux) "linux" else "mac"

val nativeDir = if (windows) {
    val resourceDir = coronaResourcesDir?.let { file("$it/../Native/").absolutePath }?.takeIf { file(it).exists() }
    (resourceDir ?: "${System.getenv("CORONA_PATH")}/Native").replace("\\", "/")
} else if (linux) {
    "$coronaResourcesDir/Native"
} else {
    val resourceDir = coronaResourcesDir?.let { file("$it/../../../Native/").absolutePath }?.takeIf { file(it).exists() }
    resourceDir ?: "${System.getenv("HOME")}/Library/Application Support/Corona/Native/"
}

val coronaPlugins = file("$buildDir/corona-plugins")
val luaCmd = "$nativeDir/Corona/$shortOsName/bin/lua"
val isSimulatorBuild = coronaTmpDir != null

fun checkCoronaNativeInstallation() {
    if (file("$nativeDir/Corona/android/resource/android-template.zip").exists())
        return
    if (windows) {
        throw InvalidUserDataException("Corona Native was not set-up properly. Re-install Corona to fix this issue.")
    } else {
        val setupNativeApp = File("/Applications").listFiles { f ->
            f.isDirectory && f.name.startsWith("Corona")
        }.maxOrNull()?.let {
            "${it.absolutePath}/Native/Setup Corona Native.app"
        } ?: "Native/Setup Corona Native.app"
        throw InvalidUserDataException("Corona Native was not set-up properly. Launch '$setupNativeApp'.")
    }
}

val buildToolsDir = "$projectDir/buildTools".takeIf { file(it).exists() }
        ?: "$projectDir/../template".takeIf { file(it).exists() } ?: {
            checkCoronaNativeInstallation()
            copy {
                from(zipTree("$nativeDir/Corona/android/resource/android-template.zip"))
                into("$buildDir/intermediates/corona-build-tools")
            }
            "$buildDir/intermediates/corona-build-tools/template/app/buildTools"
        }()

val generatedPluginsOutput = "$buildDir/generated/corona_plugins"
val generatedPluginAssetsDir = "$generatedPluginsOutput/assets"
val generatedPluginNativeLibsDir = "$generatedPluginsOutput/native"
val generatedBuildIdPath = "$generatedPluginsOutput/build"
val generatedPluginMegaJar = "$generatedPluginsOutput/plugins.jar"
val generatedMainIconsAndBannersDir = "$buildDir/generated/corona_icons"

val parsedBuildProperties: JsonObject = run {
    coronaTmpDir?.let { srcDir ->
        file("$srcDir/build.properties").takeIf { it.exists() }?.let { f ->
            return@run Parser.default().parse(f.absolutePath) as JsonObject
        }
    }

    val buildSettingsFile = file("$coronaSrcDir/build.settings")
    if (!buildSettingsFile.exists()) {
        return@run JsonObject(mapOf("buildSettings" to JsonObject(), "packageName" to coronaAppPackage, "targetedAppStore" to coronaTargetStore))
    }

    val output = ByteArrayOutputStream()
    val execResult = exec {
        setWorkingDir("$nativeDir/Corona/$shortOsName/bin")
        commandLine(luaCmd,
                "-e",
                "package.path='$nativeDir/Corona/shared/resource/?.lua;'..package.path",
                "-e",
                """
                        dofile('${buildSettingsFile.path.replace("\\", "\\\\")}')
                        if type(settings) == 'table' then
                            print(require('json').encode(settings))
                        else
                            print('{}')
                        end
                    """.trimIndent()
        )
        errorOutput = output
        standardOutput = output
        isIgnoreExitValue = true
    }
    if (execResult.exitValue != 0) {
        throw InvalidUserDataException("Build.settings file could not be parsed: ${output.toString().replace(luaCmd, "")}")
    }
    val parsedBuildSettingsFile = Parser.default().parse(StringBuilder(output.toString())) as? JsonObject
    return@run JsonObject(mapOf("buildSettings" to parsedBuildSettingsFile, "packageName" to coronaAppPackage, "targetedAppStore" to coronaTargetStore))
}

extra["minSdkVersion"] = parsedBuildProperties.lookup<Any?>("buildSettings.android.minSdkVersion").firstOrNull()?.toString()?.toIntOrNull()
        ?: 15

val coronaBuilder = if (windows) {
    "$nativeDir/Corona/win/bin/CoronaBuilder.exe"
} else if (linux) {
    "$coronaResourcesDir/../Solar2DBuilder"
} else {
    "$nativeDir/Corona/$shortOsName/bin/CoronaBuilder.app/Contents/MacOS/CoronaBuilder"
}

val coronaVersionName =
        parsedBuildProperties.lookup<Any?>("buildSettings.android.versionName").firstOrNull()?.toString()
                ?: project.findProperty("coronaVersionName") as? String ?: "1.0"

val coronaVersionCode: Int =
        parsedBuildProperties.lookup<Any?>("buildSettings.android.versionCode").firstOrNull()?.toString()?.toIntOrNull()
                ?: (project.findProperty("coronaVersionCode") as? String)?.toIntOrNull() ?: 1

val androidDestPluginPlatform = if (coronaTargetStore.equals("amazon", ignoreCase = true)) {
    "android-kindle"
} else if (coronaTargetStore.equals("samsung", ignoreCase = true)) {
    "android-nongoogle"
} else {
    "android"
}

val coronaAndroidPluginsCache = file(if (windows) {
    if (coronaCustomHome.isNullOrEmpty()) {
        "${System.getenv("APPDATA")}/Corona Labs/Corona Simulator/build cache/$androidDestPluginPlatform"
    } else {
        "$coronaCustomHome/build cache/$androidDestPluginPlatform"
    }
} else {
    "${System.getenv("HOME")}/Library/Application Support/Corona/build cache/$androidDestPluginPlatform"
})

val pluginDisabledMetadata = mutableSetOf<String>()
val pluginDisabledDependencies = mutableSetOf<String>()
val pluginDisabledJar = mutableSetOf<String>()
val pluginDisabledNative = mutableSetOf<String>()
val pluginDisabledResources = mutableSetOf<String>()
val pluginDisabledAssets = mutableSetOf<String>()
val coronaProcessedScripts = mutableSetOf<File>()
val excludePluginMap = mapOf(
        "metadata" to pluginDisabledMetadata
        , "dependencies" to pluginDisabledDependencies
        , "jars" to pluginDisabledJar
        , "native" to pluginDisabledNative
        , "resources" to pluginDisabledResources
        , "assets" to pluginDisabledAssets
)
extra["excludeMap"] = excludePluginMap

if (configureCoronaPlugins == "YES") {
    downloadAndProcessCoronaPlugins()
}

//</editor-fold>

android {
    lintOptions {
        isCheckReleaseBuilds = true
    }
    compileSdk = 33
    defaultConfig {
        applicationId = coronaAppPackage
        targetSdk = 33
        minSdk = (extra["minSdkVersion"] as Int)
        versionCode = coronaVersionCode
        versionName = coronaVersionName
        multiDexEnabled = true
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility  = JavaVersion.VERSION_11
    }
    coronaKeystore?.let { keystore ->
        signingConfigs {
            create("release") {
                storeFile = file(keystore)
                storePassword = coronaKeystorePassword
                keyAlias = coronaKeyAlias
                keyPassword = coronaKeyAliasPassword
            }
        }
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android.txt"), "proguard-rules.pro")
            coronaKeystore?.let { signingConfig = signingConfigs.getByName("release") }
        }
    }

    applicationVariants.all {
        generateBuildConfigProvider!!.configure {
            enabled = false
        }
    }
    testOptions {
        testVariants.all {
            generateBuildConfigProvider!!.configure {
                enabled = false
            }
        }
    }
    val mainSourceSet = sourceSets["main"]
    val pluginJniLibs = file(coronaPlugins).walk().maxDepth(2).filter { it.name == "jniLibs" }.toSet()
    mainSourceSet.jniLibs.srcDirs(pluginJniLibs)
    mainSourceSet.jniLibs.srcDir(generatedPluginNativeLibsDir)
    mainSourceSet.res.srcDir(generatedMainIconsAndBannersDir)
    mainSourceSet.assets.srcDir(generatedPluginAssetsDir)
    file("$generatedPluginsOutput/resourceDirectories.json").takeIf { it.exists() }?.let {
        val resourceDirs: List<String>? = Klaxon().parseArray(it)
        resourceDirs?.forEach { res ->
            mainSourceSet.res.srcDir(res)
        }
    }
    val extraPackages = mutableListOf(coronaAppPackage)
    file("$generatedPluginsOutput/resourcePackages.txt").takeIf { it.exists() }?.let {
        extraPackages += it.readText().trim()
    }
    aaptOptions {
        additionalParameters("--extra-packages", extraPackages.filter { it.isNotBlank() }.joinToString(":"))
    }
    if (isExpansionFileRequired) {
        assetPacks.add(":preloadedAssets")
    }

    parsedBuildProperties.lookup<JsonArray<JsonObject>>("buildSettings.android.onDemandResources").firstOrNull()?.forEach {
        it["tag"].let { tag ->
            assetPacks.add(":pda-$tag")
        }
    }

    // This is dirty hack because Android Assets refuse to copy assets which start with . or _
    if (!isExpansionFileRequired) {
        android.applicationVariants.all {
            mergeAssetsProvider!!.configure {
                val mergeTask = this
                doLast {
                    copy {
                        from(generatedPluginAssetsDir) {
                            include(".corona-plugins/**")
                        }
                        into(mergeTask.outputDir)
                    }
                }
            }
        }
    }
}

//<editor-fold desc="Packaging Corona App" defaultstate="collapsed">
val apkFilesSet = mutableSetOf<String>()
file("$buildDir/intermediates/corona_manifest_gen/CopyToApk.txt").takeIf { it.exists() }?.readLines()?.forEach {
    apkFilesSet.add(it.trim())
}
if (!isSimulatorBuild) {
    parsedBuildProperties.lookup<JsonArray<String>>("buildSettings.android.apkFiles").firstOrNull()?.forEach {
        apkFilesSet.add(it.trim())
    }
}
if (apkFilesSet.isNotEmpty()) {
    val generatedApkFiles = "$buildDir/generated/apkFiles"
    val coronaCopyApkFiles = tasks.create<Copy>("coronaCopyApkFiles") {
        description = "Creates new resource directory with raw APK files"
        into(generatedApkFiles)
        from(coronaSrcDir) {
            apkFilesSet.forEach { include(it) }
        }
        doFirst {
            delete(generatedApkFiles)
        }
    }

    android.applicationVariants.all {
        preBuildProvider.configure {
            dependsOn(coronaCopyApkFiles)
        }
        android.sourceSets[name].resources.srcDirs(generatedApkFiles)
    }
}

fun processPluginGradleScripts() {
    fileTree(coronaPlugins) {
        include("**/corona.gradle", "**/corona.gradle.kts")
    }.forEach {
        if (coronaProcessedScripts.contains(it))
            return@forEach
        coronaProcessedScripts.add(it)

        val pluginName = it.relativeTo(coronaPlugins).toPathString().segments.first()
        try {
            apply(from = it)
            // Exclude plugin from following lists
            if (project.extra.has("excludeCoronaPlugin")) {
                when (val exclude = project.extra["excludeCoronaPlugin"]) {
                    true -> excludePluginMap.forEach { kv -> kv.value.add(pluginName) }
                    is String -> if (excludePluginMap.containsKey(exclude)) excludePluginMap[exclude]?.add(pluginName)
                    is Iterable<*> -> exclude.forEach { e -> if (excludePluginMap.containsKey(e)) excludePluginMap[e]?.add(pluginName) }
                }
            }
            project.extra["excludeCoronaPlugin"] = null
            // Undo exclude
            if (project.extra.has("includeCoronaPlugin")) {
                when (val include = project.extra["includeCoronaPlugin"]) {
                    is String -> if (excludePluginMap.containsKey(include)) excludePluginMap[include]?.remove(pluginName)
                    is Iterable<*> -> include.forEach { e -> if (excludePluginMap.containsKey(e)) excludePluginMap[e]?.remove(pluginName) }
                }
            }
            project.extra["includeCoronaPlugin"] = null
        } catch (ex: Throwable) {
            logger.error("ERROR: configuring '$pluginName' failed!")
            throw(ex)
        }
    }
    fileTree("$coronaSrcDir/AndroidResources") {
        include("**/corona.gradle", "**/corona.gradle.kts")
    }.forEach {
        try {
            apply(from = it)
        } catch (ex: Throwable) {
            logger.error("ERROR: executing configuration from '${it.relativeTo(file(coronaSrcDir)).path}' failed!")
            throw(ex)
        }
    }
}
processPluginGradleScripts()


fun coronaAssetsCopySpec(spec: CopySpec) {
    with(spec) {
        file("$coronaTmpDir/excludesfile.properties").takeIf { it.exists() }?.readLines()?.forEach {
            exclude(it)
        }
        parsedBuildProperties.lookup<JsonArray<JsonObject>>("buildSettings.android.onDemandResources").firstOrNull()?.forEach {
            it["resource"].let { res ->
                exclude("$res")
                exclude("$res/**")
            }
        }
        if (!isSimulatorBuild) {
            // use build.settings properties only if this is not simulator build
            parsedBuildProperties.lookup<JsonArray<String>>("buildSettings.excludeFiles.all").firstOrNull()?.forEach {
                exclude("**/$it")
            }
            parsedBuildProperties.lookup<JsonArray<String>>("buildSettings.excludeFiles.android").firstOrNull()?.forEach {
                exclude("**/$it")
            }
        }
        exclude("**/Icon\r")
        exclude("AndroidResources/**")
        if (isLiveBuild) {
            into("corona_live_build_app_")
        } else {
            exclude("**/*.lua", "build.settings")
        }
    }
}

android.applicationVariants.all {
    val baseName = this.baseName.toLowerCase()
    val isRelease = (baseName == "release")
    val generatedAssetsDir = "$buildDir/generated/corona_assets/$baseName"
    val compiledLuaArchive = "$buildDir/intermediates/compiled_lua_archive/$baseName/resource.car"

    val compileLuaTask = tasks.create("compileLua${baseName.capitalize()}") {
        description = "If required, compiles Lua and archives it into resource.car"
        val luac = "$nativeDir/Corona/$shortOsName/bin/luac"

        val srcLuaFiles = fileTree(coronaSrcDir) {
            include("**/*.lua")
        }
        val pluginLuaFiles = fileTree(coronaPlugins) {
            include("**/*.lua")
            exclude("*/metadata.lua")
        }

        inputs.files(srcLuaFiles + pluginLuaFiles)
        outputs.file(compiledLuaArchive)
        doLast {
            val rootFile = file(coronaSrcDir)
            val compiledDir = "$buildDir/intermediates/compiled_lua/$baseName"
            delete(compiledDir)
            mkdir(compiledDir)
            val luaFiles = if (isSimulatorBuild) {
                pluginLuaFiles
            } else {
                srcLuaFiles + pluginLuaFiles
            }
            val outputsList = luaFiles.map {
                val compiled = when {
                    it.canonicalPath.startsWith(rootFile.canonicalPath) -> it.relativeTo(rootFile)
                            .toPathString()
                            .segments
                            .joinToString(".")
                            .replaceAfterLast(".", "lu")
                    it.canonicalPath.startsWith(coronaPlugins.canonicalPath) -> it.relativeTo(coronaPlugins)
                            .toPathString()
                            .segments.drop(1)
                            .joinToString(".")
                            .replaceAfterLast(".", "lu")
                            .removePrefix("lua.")
                            .removePrefix("lua_51.")
                    else -> throw InvalidUserDataException("Unknown location of Lua file '$it'!")
                }
                exec {
                    val additionalArguments = mutableListOf<String>()
                    if (isRelease) {
                        additionalArguments += "-s"
                    }
                    commandLine(luac, *additionalArguments.toTypedArray(), "-o", "$compiledDir/$compiled", "--", it)
                }
                compiled
            } + if (isSimulatorBuild) {
                fileTree(coronaTmpDir!!) {
                    include("*.lu")
                }
            } else {
                files().asFileTree
            }
            val buildId = file(generatedBuildIdPath)
                    .takeIf { it.exists() }?.let {
                        file(generatedBuildIdPath).readText().trim()
                    } ?: coronaBuild ?: "unknown"
            val metadataLuaStr = """
                if not application or type( application ) ~= "table" then
                    application = {}
                end
                application.metadata = {
                    appName = "$coronaAppFileName",
                    appVersion = "$coronaVersionName",
                    appPackageId = "${android.defaultConfig.applicationId}",
                    mode = "$baseName",
                    build = "$buildId",
                    coronaBuild = "$coronaBuild",
                }
            """.trimIndent()
            val metadataConfig = file("$buildDir/tmp/config.$baseName.lua")
            val metadataCompiled = file("$buildDir/tmp/config.$baseName.lu")
            mkdir(metadataConfig.parent)
            metadataConfig.writeText(metadataLuaStr)
            val configEntries = outputsList.filter { file(it).name == "config.lu" } + metadataConfig
            exec {
                workingDir = file(compiledDir)
                commandLine(luac, "-s", "-o", metadataCompiled, "--", *configEntries.toTypedArray())
            }
            copy {
                from(metadataCompiled)
                into(compiledDir)
                rename { "config.lu" }
            }
            delete(metadataConfig)
            val toArchive = outputsList.filter { file(it).name != "config.lu" } + "config.lu"
            mkdir(file(compiledLuaArchive).parent)
            exec {
                workingDir = file(compiledDir)
                standardInput = StringInputStream(toArchive.joinToString("\n"))
                commandLine(coronaBuilder, "car", "-f", "-", compiledLuaArchive)
            }
        }
    }

    val taskCopyResources = tasks.create<Copy>("packageCoronaApp${baseName.capitalize()}") {
        description = "Copies all resources and compiled Lua to the project"

        dependsOn(compileLuaTask)

        into(generatedAssetsDir)
        from("$coronaTmpDir/output/assets")
        from(compiledLuaArchive)

        if (!isExpansionFileRequired) {
            from(coronaSrcDir) {
                coronaAssetsCopySpec(this)
            }
        }


        doFirst {
            delete(generatedAssetsDir)
            mkdir(generatedAssetsDir)
        }
        doFirst {
            if (!file(coronaSrcDir).isDirectory) {
                throw InvalidUserDataException("Unable to find Solar2D project (for example platform/test/assets2/main.lua)!")
            }
        }
    }

    mergeAssetsProvider!!.configure {
        dependsOn(taskCopyResources)
    }
    android.sourceSets[name].assets.srcDirs(generatedAssetsDir)
}

fun downloadAndProcessCoronaPlugins(reDownloadPlugins: Boolean = true) {

    val luaVerbosityPlug = if (!logger.isLifecycleEnabled) {
        arrayOf("-e", "printError=print;print=function()end")
    } else {
        arrayOf()
    }

    // Download plugins
    logger.lifecycle("Authorizing plugins")
    if (reDownloadPlugins) {

        delete(coronaPlugins)
        file(coronaPlugins).mkdirs()

        val buildDataStr = coronaBuildData?.let { file(it).readText() } ?: run {
            val fakeBuildData = (parsedBuildProperties.obj("buildSettings") ?: JsonObject())
            fakeBuildData["targetAppStore"] = coronaTargetStore
            fakeBuildData["dailyBuildYear"] = dailyBuildYear
            fakeBuildData["dailyBuildRevision"] = dailyBuildRevision
            fakeBuildData["appName"] = coronaAppFileName ?: "Corona App"
            fakeBuildData.toJsonString()
        }
        val buildParams = JsonObject(mapOf(
                "appName" to coronaAppFileName,
                "appPackage" to coronaAppPackage,
                "build" to dailyBuildRevision,
                "buildData" to buildDataStr,
                "modernPlatform" to "android",
                "platform" to "android",
                "pluginPlatform" to androidDestPluginPlatform,
                "destinationDirectory" to coronaPlugins.absolutePath
        )).toJsonString()
        val builderInput = file("$buildDir/tmp/builderInput.json")
        builderInput.parentFile.mkdirs()
        builderInput.writeText(buildParams)
        val builderOutput = ByteArrayOutputStream()
        val execResult = exec {
            commandLine(coronaBuilder, "plugins", "download", "--android-offline-plugins", "builderInput=${builderInput.absolutePath}")
            standardOutput = builderOutput
            errorOutput = builderOutput
            isIgnoreExitValue = true
        }
        if (execResult.exitValue != 0) {
            logger.error("Error while fetching plugins: $builderOutput")
            throw InvalidPluginException("Error while fetching plugins: $builderOutput")
        }
        if((System.getenv("DEBUG_BUILD_PROCESS")?.toIntOrNull() ?: 0) > 0 ) {
            logger.lifecycle("Plugins collector output: $builderOutput")
        }
        logger.lifecycle("Unpacking plugins")
        fileTree(coronaPlugins) {
            include("*/data.tgz")
        }.forEach { data ->
            copy {
                from(tarTree(resources.gzip(data)))
                into(data.parent)
            }
            data.delete()
        }
    }
    processPluginGradleScripts()

    // Collect Assets
    logger.lifecycle("Collecting plugin assets")
    run {
        delete(generatedPluginAssetsDir)
        copy {
            from(coronaPlugins) {
                include("*/resources/assets/**/*")
                pluginDisabledAssets.forEach {
                    exclude("**/$it/**")
                }
            }
            into(generatedPluginAssetsDir)
            eachFile {
                path = File(path).toPathString().segments.drop(3).joinToString("/")
            }
            includeEmptyDirs = false
        }
        copy {
            from(coronaPlugins) {
                include("*/lua/lua_51/**/*")
                include("*/lua_51/**/*")
                exclude("**/*.lua")
            }
            into("$generatedPluginAssetsDir/.corona-plugins")
            eachFile {
                path = File(path).toPathString().segments.drop(2).dropWhile { it == "lua_51" }.joinToString("/")
            }
            includeEmptyDirs = false
        }
    }

    // Collect Resources
    logger.lifecycle("Collecting plugin resources")
    run {
        file(generatedPluginsOutput).mkdirs()
        val resourceDirectories = File(coronaPlugins.path)
                .walk()
                .maxDepth(2)
                .filter { it.isDirectory }
                .filter { it.name == "resources" }
                .filter { !pluginDisabledResources.contains(it.parentFile.name) }
                .flatMap { pr ->
                    pr.walk()
                            .filter { r -> r.isDirectory }
                            .filter { r ->
                                r == pr || r.name.startsWith("package")
                            }
                }.toList()
        val resDirectories = resourceDirectories.map { File(it, "res") }
                .filter { it.exists() && it.isDirectory }
                .map { it.absolutePath }
        file("$generatedPluginsOutput/resourceDirectories.json").writeText(JsonArray(resDirectories).toJsonString())

        val extraPackages = resourceDirectories.map {
            val packageFile = File(it, "package.txt")
            if (packageFile.exists()) {
                packageFile.readText().trim()
            } else {
                ""
            }
        }.filter { it.isNotBlank() }.joinToString(":")
        file("$generatedPluginsOutput/resourcePackages.txt").writeText(extraPackages)
    }

    // Run `convert_metadata.lua` which collects plugin metadata into single file
    logger.lifecycle("Collecting plugin metadata")
    run {
        file("$buildDir/intermediates").mkdirs()
        val metadataFiles = fileTree(coronaPlugins) {
            include("*/metadata.lua")
            pluginDisabledMetadata.forEach {
                exclude("**/$it/**")
            }
        }.map { it.absolutePath }
        exec {
            commandLine(luaCmd
                    , "-e"
                    , "package.path='$nativeDir/Corona/shared/resource/?.lua;'..package.path"
                    , *luaVerbosityPlug
                    , "$buildToolsDir/convert_metadata.lua"
                    , "$buildDir/intermediates/plugins_metadata.json"
                    , *metadataFiles.toTypedArray()
            )
        }
    }

    // Run `update_manifest.lua` which generates AndroidManifest.xml
    logger.lifecycle("Creating AndroidManifest.xml")
    run {
        val buildPropsFile = file("$coronaTmpDir/build.properties")
        val inputSettingsFile = if (buildPropsFile.exists()) {
            buildPropsFile
        } else {
            val buildPropsOut = file("$buildDir/intermediates/corona.build.props")
            buildPropsOut.writeText(parsedBuildProperties.toJsonString())
            buildPropsOut
        }

        val manifestGenDir = "$buildDir/intermediates/corona_manifest_gen"
        file(manifestGenDir).mkdirs()
        exec {
            commandLine(luaCmd
                    , "-e"
                    , "package.path='$nativeDir/Corona/shared/resource/?.lua;'..package.path"
                    , *luaVerbosityPlug
                    , "$buildToolsDir/update_manifest.lua"
                    , /*1*/ "$buildToolsDir/AndroidManifest.template.xml"
                    , /*2*/ inputSettingsFile
                    , /*3*/ coronaAppFileName ?: "Corona App"
                    , /*4*/ "$manifestGenDir/AndroidManifest.xml"
                    , /*5*/ "$buildDir/intermediates/plugins_metadata.json"
                    , /*6*/ "$buildToolsDir/strings.xml"
                    , /*7*/ "$manifestGenDir/strings.xml"
                    , /*8*/ "$manifestGenDir/CopyToApk.txt"
            )
        }
        copy {
            from(manifestGenDir) {
                include("AndroidManifest.xml")
            }
            into("$projectDir/src/main")
        }
        copy {
            from(manifestGenDir) {
                include("strings.xml")
            }
            into("$projectDir/src/main/res/values")
        }
    }

    logger.lifecycle("Collecting native libraries")
    run {
        delete(generatedPluginNativeLibsDir)
        file(coronaPlugins).walk().maxDepth(1).forEach {
            if (it == coronaPlugins) return@forEach
            if (pluginDisabledNative.contains(it.name)) return@forEach

            val hasJniLibs = !fileTree(it) {
                include("**/jniLibs/**/*.so")
            }.isEmpty
            val armeabiV7Libs = fileTree(it) {
                include("**/*.so")
            }
            val hasArm32Libs = !armeabiV7Libs.isEmpty
            if (!hasJniLibs && hasArm32Libs) {
                logger.error("WARNING: Plugin '$it' contain native library for armeabi-v7a but not for arm64.")
                copy {
                    from(armeabiV7Libs)
                    into(generatedPluginNativeLibsDir)
                    eachFile {
                        path = "armeabi-v7a/$name"
                    }
                    includeEmptyDirs = false
                }
            }
        }
    }

    logger.lifecycle("Collecting legacy jar libraries")
    run {
        val megaJarExtracted = "$buildDir/intermediates/corona-mega-jar"
        delete(megaJarExtracted)
        delete(generatedPluginMegaJar)
        fileTree(coronaPlugins) {
            include("*/*.jar")
            pluginDisabledJar.forEach {
                exclude("**/$it/**")
            }
        }.sortedBy {
            it.parent.contains("shared.")
        }.forEach {
            copy {
                from(zipTree(it))
                into(megaJarExtracted)
            }
        }
        if (file(megaJarExtracted).exists()) {
            ant.withGroovyBuilder {
                "zip"("destfile" to generatedPluginMegaJar, "basedir" to megaJarExtracted)
            }
        }
    }
}

tasks.register("setUpCoronaAppAndPlugins") {
    group = "Corona"
    doLast {
        downloadAndProcessCoronaPlugins()
    }
}

tasks.register("processPluginsNoDownload") {
    if (coronaBuiltFromSource) group = "Corona"
    doLast {
        downloadAndProcessCoronaPlugins(false)
    }
}

//</editor-fold>


//<editor-fold desc="Core Development helpers" defaultstate="collapsed">

tasks.register<Zip>("exportCoronaAppTemplate") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    destinationDirectory.set(file("$buildDir/outputs"))
    archiveFileName.set("android-template.zip")
    from(rootDir) {
        include("build.gradle.kts", "settings.gradle.kts", "gradle.properties")
        include("gradlew", "gradlew.bat", "gradle/wrapper/**")
        include("app/**")
        exclude("app/build/**", "app/CMakeLists.txt")
        exclude("**/*.iml", "**/\\.*")
        include("setup.sh", "setup.bat")
        include("preloadedAssets/build.gradle.kts")
        include("PAD.kts.template")
        into("template")
    }
    from(android.sdkDirectory) {
        include("licenses/android-sdk-license")
        into("sdk")
    }
    from("$projectDir/../template") {
        include("AndroidManifest.template.xml", "convert_metadata.lua", "update_manifest.lua", "strings.xml")
        into("template/app/buildTools")
    }
    from("$rootDir/../../plugins/build/licensing-google/android/bin") {
        include("classes.jar")
        into("template/app/libs")
        rename("classes.jar", "licensing-google.jar")
    }
    doLast {
        logger.lifecycle("Exported to '${archiveFile.get()}'")
    }
}

tasks.register<Copy>("exportToNativeAppTemplate") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    val templateDir = "$rootDir/../../subrepos/enterprise/contents/Project Template/App/android"

    into(templateDir)
    from(rootDir) {
        include("build.gradle.kts")
        include("gradlew", "gradlew.bat", "gradle/wrapper/**")
        include("app/**")
        exclude("app/build/**", "app/CMakeLists.txt", "app/build.gradle.kts")
        exclude("**/*.iml", "**/\\.*")
        exclude("**/AndroidManifest.xml")
    }
    from(rootDir) {
        include("app/build.gradle.kts")
        filter {
            it.replace("com.corona.app", "com.mycompany.app")
        }
    }

    doFirst {
        delete(fileTree(templateDir) {
            exclude("plugin/**")
            exclude("settings.gradle")
            exclude("**/AndroidManifest.xml")
            exclude("**/*.java")
            exclude("gradle.properties")
        })
    }
    doLast {
        logger.lifecycle("Copied to ${file(templateDir).absolutePath}")
    }
}


val coronaNativeOutputDir = project.findProperty("coronaNativeOutputDir") as? String
        ?: "$nativeDir/Corona"

tasks.register<Copy>("installAppTemplateToSim") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    dependsOn("exportCoronaAppTemplate")
    from("$buildDir/outputs") {
        include("android-template.zip")
    }
    into("$coronaNativeOutputDir/android/resource")
}

tasks.register<Copy>("installAppTemplateAndAARToSim") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    dependsOn("installAppTemplateToSim")
    dependsOn(":Corona:assembleRelease")
    from("${findProject(":Corona")?.buildDir}/outputs/aar/") {
        include("Corona-release.aar")
        rename("Corona-release.aar", "Corona.aar")
    }
    into("$coronaNativeOutputDir/android/lib/gradle")
}

fun copyWithAppFilename(dest: String, appName: String?) {
    delete("$dest/$coronaAppFileName.apk")
    delete("$dest/$coronaAppFileName.aab")
    var hasODR = false
    parsedBuildProperties.lookup<JsonArray<JsonObject>>("buildSettings.android.onDemandResources").firstOrNull()?.forEach {
        it["resource"].let { res ->
            hasODR = true
        }
    }

    copy {
        into(dest)
        val copyTask = this
        android.applicationVariants.matching {
            it.name.equals("release", true)
        }.all {
            if(!isExpansionFileRequired && !hasODR) {
                copyTask.from(packageApplicationProvider!!.get().outputDirectory) {
                    include("*.apk")
                    exclude("*unsigned*")
                }
            }
            copyTask.from("$buildDir/outputs/bundle/$name") {
                include("*.aab")
            }
        }
        rename {
            "$appName.${file(it).extension}"
        }
    }
}

tasks.create("buildCoronaApp") {
    description = "Used when Simulator invokes a build. It all project variables must be passed"
    dependsOn("assembleRelease")
    dependsOn("bundleRelease")
    dependsOn("createExpansionFile")

    coronaDstDir?.let {
        doLast {
            try {
                copyWithAppFilename(it, coronaAppFileName)
            } catch (ignore: Throwable) {
                try {
                    val defaultName = "App"
                    copyWithAppFilename(it, defaultName)
                    logger.error("WARNING: Used default filename '$defaultName' because original contains non-ASCII symbols.")
                } catch (ex: Throwable) {
                    logger.error("ERROR: Unable to finalize build. Make sure path to destination doesn't contain non-ASCII symbols")
                    throw ex
                }
            }
            delete("$it/$coronaExpansionFileName")
        }
    }
}

//</editor-fold>


//<editor-fold desc="Corona project icons" defaultstate="collapsed">

val cleanupIconsDir = tasks.create<Delete>("cleanupIconsDirectory") {
    delete(generatedMainIconsAndBannersDir)
}

val copyCoronaIconFiles = tasks.create("copyCoronaIconFiles") {
    description = "Copies icon files to appropriate resources location"
    tasks.findByName("preBuild")?.dependsOn(this)
}

tasks.create<Copy>("copyAdaptiveIconResources") {
    dependsOn(copyCoronaIconFiles, cleanupIconsDir)

    into(generatedMainIconsAndBannersDir)
    from("$coronaSrcDir/AndroidResources/res")
}

tasks.create("copyCoronaIcons") {
    description = "Copies icon files to appropriate resources location"
    tasks.findByName("preBuild")?.dependsOn(this)
    dependsOn("copyAdaptiveIconResources")
    dependsOn(copyCoronaIconFiles)
}

tasks.create<Copy>("copyMainApplicationIcon") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    into(generatedMainIconsAndBannersDir)
    rename { "ic_launcher.png" }

    val suffixes = listOf("ldpi", "mdpi", "hdpi", "xhdpi", "xxhdpi", "xxxhdpi").reversed()
    suffixes.forEach { dstSuffix ->
        val fallbacks = listOf(dstSuffix) + suffixes.takeLastWhile { it != dstSuffix }
        fallbacks.firstOrNull {
            !fileTree(coronaSrcDir).matching {
                include("Icon-$it*.png")
            }.isEmpty
        }?.let {
            into("mipmap-$dstSuffix") {
                from(coronaSrcDir)
                include("Icon-$it*.png")
            }
        }
    }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/ic_launcher.png")
    }
}


tasks.create<Copy>("copyMainApplicationBanner") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Banner-xhdpi.png")
    }
    into("$generatedMainIconsAndBannersDir/drawable-xhdpi")
    rename { "banner.png" }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/banner.png")
    }
}

tasks.create<Copy>("copyOuyaApplicationIcon") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Icon-ouya.png")
    }
    into("$generatedMainIconsAndBannersDir/drawable-xhdpi")
    rename { "ouya_icon.png" }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/ouya_icon.png")
    }
}

tasks.create<Copy>("copyXiaomiApplicationIcon") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Icon-ouya-xiaomi.png")
    }
    into("$generatedMainIconsAndBannersDir/drawable-xhdpi")
    rename { "ouya_xiaomi_icon.png" }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/ouya_xiaomi_icon.png")
    }
}

tasks.create<Copy>("copyDefaultNotificationIcons") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    into(generatedMainIconsAndBannersDir)
    rename { "corona_statusbar_icon_default.png" }

    val suffixes = listOf("ldpi", "mdpi", "hdpi", "xhdpi", "xxhdpi", "xxxhdpi").reversed()
    suffixes.forEach { dstSuffix ->
        val fallbacks = listOf(dstSuffix) + suffixes.takeLastWhile { it != dstSuffix }
        fallbacks.firstOrNull {
            !fileTree(coronaSrcDir).matching {
                include("IconNotificationDefault-$it*.png")
            }.isEmpty
        }?.let {
            into("drawable-$dstSuffix") {
                from(coronaSrcDir)
                include("IconNotificationDefault-$it*.png")
            }
        }
    }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/corona_statusbar_icon_default.png")
    }
}

tasks.create<Copy>("copySplashScreen") {
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    val enableSplash: Boolean = parsedBuildProperties.lookup<Boolean?>("buildSettings.splashScreen.android.enable").firstOrNull()
            ?: parsedBuildProperties.lookup<Boolean?>("buildSettings.splashScreen.enable").firstOrNull()
            ?: true
    val image: String? = parsedBuildProperties.lookup<String?>("buildSettings.splashScreen.android.image").firstOrNull()
            ?: parsedBuildProperties.lookup<String?>("buildSettings.splashScreen.image").firstOrNull()

    logger.info("Configured Splash Screen enable: $enableSplash, image: $image.")
    if (enableSplash) {
        if (image != null) {
            from(coronaSrcDir) {
                include(image)
            }
            if (inputs.sourceFiles.isEmpty) throw InvalidUserDataException("Custom Splash Screen file '$image' not found!")
        } else {
            from(projectDir) {
                include("_corona_splash_screen.png")
            }
            if (inputs.sourceFiles.isEmpty) throw InvalidUserDataException("Splash screen was not disabled in build.settings but '$projectDir/_corona_splash_screen.png' was not found!")
        }
    }

    into("$generatedMainIconsAndBannersDir/drawable")
    rename {
        "_corona_splash_screen.${file(it).extension}"
    }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/_corona_splash_screen.png")
    }
}

tasks.register<Zip>("createExpansionFile") {
    enabled = isExpansionFileRequired
    destinationDirectory.set(file("$buildDir/outputs"))
    archiveFileName.set(coronaExpansionFileName)

    from(coronaSrcDir) {
        coronaAssetsCopySpec(this)
        into("")
    }
    from("$generatedPluginAssetsDir/.corona-plugins") {
        into(".corona-plugins")
    }

}

//</editor-fold>

dependencies {
    if (coronaBuiltFromSource) {
        implementation(project(":Corona"))
        implementation(files("$rootDir/../../plugins/build/licensing-google/android/bin/classes.jar"))
    } else {
        val coronaLocal = file("libs/Corona.aar")
        if (coronaLocal.exists()) {
            implementation(files(coronaLocal))
        } else {
            checkCoronaNativeInstallation()
            implementation(":Corona@aar")
            implementation(":licensing-google@jar")
        }
        implementation(fileTree("libs") {
            include("**/*.jar")
        })
    }
    if (file(generatedPluginMegaJar).exists()) {
        implementation(files(generatedPluginMegaJar))
    }
    implementation(fileTree(coronaPlugins) {
        include("**/*.aar")
    })
    implementation(fileTree(coronaPlugins) {
        include("*/jarLibs/*.jar")
    })
    implementation(fileTree("$coronaSrcDir/AndroidResources") {
        include("**/*.jar", "**/*.aar")
    })
    if (file("../plugin").exists()) {
        implementation(project(":plugin"))
    }
    implementation("androidx.multidex:multidex:2.0.1")
}
