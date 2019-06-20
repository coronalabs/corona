import com.android.ide.common.util.toPathString
import de.undercouch.gradle.tasks.download.DownloadAction
import groovy.json.JsonOutput
import groovy.json.JsonSlurper
import org.apache.commons.io.output.ByteArrayOutputStream
import org.apache.tools.ant.filters.StringInputStream


plugins {
    id("com.android.application")
}

val coronaResourcesDir: String? by project
val coronaDstDir: String? by project
val coronaTmpDir: String? by project
val coronaAppFileName: String? by project
val coronaAppPackage: String? by project
val coronaVersionCode: String? by project
val coronaVersionName: String? by project
val coronaKeystore: String? by project
val coronaKeystorePassword: String? by project
val coronaKeyAlias: String? by project
val coronaKeyAliasPassword: String? by project
val configureCoronaPlugins: String? by project
val coronaBuild: String? by project
val coronaBuildData: String? by project
val isLiveBuild = project.findProperty("coronaLiveBuild") == "YES"
val coronaSrcDir = project.findProperty("coronaSrcDir") as? String
        ?: if (file("$rootDir/../test/assets2").exists()) {
            "$rootDir/../test/assets2"
        } else {
            "$rootDir/../Corona"
        }

val windows = System.getProperty("os.name").toLowerCase().contains("windows")
val shortOsName = if (windows) "win" else "mac"
val nativeDir = if (windows) {
    val resourceDir = coronaResourcesDir?.let { file("$it/../Native/").absolutePath }?.takeIf { file(it).exists() }
    (resourceDir ?: System.getenv("CORONA_ROOT")).replace("\\", "/")
} else {
    val resourceDir = coronaResourcesDir?.let { file("$it/../../../Native//").absolutePath }?.takeIf { file(it).exists() }
    resourceDir ?: "${System.getenv("HOME")}/Library/Application Support/Corona/Native/"
}
val windowsPathHelper = "${System.getenv("PATH")}${File.pathSeparator}${System.getenv("CORONA_PATH")}"

val coronaPlugins = file("$buildDir/corona-plugins")

val buildToolsDir = if (file("$projectDir/buildTools").exists()) {
    "$projectDir/buildTools"
} else {
    "$projectDir/../template"
}
val generatedPluginsOutput = "$buildDir/generated/corona_plugins"
val generatedPluginAssetsDir = "$generatedPluginsOutput/assets"
val generatedPluginNativeLibsDir = "$generatedPluginsOutput/native"
val generatedControlPath = "$generatedPluginsOutput/control"
val generatedBuildIdPath = "$generatedPluginsOutput/build"
val generatedMainIconsAndBannersDir = "$buildDir/generated/corona_icons"


var buildSettings: Any? = null
var fakeBuildData: String? = null
coronaTmpDir?.let { srcDir ->
    file("$srcDir/build.properties").takeIf { it.exists() }?.let { f ->
        buildSettings = JsonSlurper().parse(f)
    }
}

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

@Suppress("OldTargetApi")
android {
    compileSdkVersion(27)
    defaultConfig {
        applicationId = coronaAppPackage ?: "com.corona.test"
        minSdkVersion(15)
        targetSdkVersion(27)
        versionCode = coronaVersionCode?.toInt() ?: 1
        versionName = coronaVersionName ?: "1.0"
        multiDexEnabled = true
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

    flavorDimensions("corona")

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
        val resourceDirs = JsonSlurper().parse(it)
        if (resourceDirs is List<*>) {
            resourceDirs.forEach { res ->
                mainSourceSet.res.srcDir(res)
            }
        }
    }
    val extraPackages = mutableListOf(defaultConfig.applicationId!!)
    file("$generatedPluginsOutput/resourcePackages.txt").takeIf { it.exists() }?.let {
        extraPackages += it.readText().trim()
    }
    aaptOptions {
        additionalParameters("--extra-packages", extraPackages.filter { it.isNotBlank() }.joinToString(":"))
    }
    // This is dirty hack because Android Assets refuse to copy assets which start with . or _
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
        } catch (ex: Exception) {
            logger.error("ERROR: configuring '$pluginName' failed!")
            throw(ex)
        }
    }
    fileTree("$coronaSrcDir/AndroidResources") {
        include("**/corona.gradle", "**/corona.gradle.kts")
    }.forEach {
        try {
            apply(from = it)
        } catch (ex: Exception) {
            logger.error("ERROR: executing configuration from '${it.relativeTo(file(coronaSrcDir)).path}' failed!")
            throw(ex)
        }
    }
}
processPluginGradleScripts()

//region Packaging Corona App

android.applicationVariants.all {
    val baseName = this.baseName.toLowerCase()
    val isRelease = (baseName == "release")
    val generatedAssetsDir = "$buildDir/generated/corona_assets/$baseName"
    val compiledLuaArchive = "$buildDir/intermediates/compiled_lua_archive/$baseName/resource.car"

    val compileLuaTask = tasks.create("compileLua${baseName.capitalize()}") {
        group = "Corona"
        description = "If required, compiles Lua and archives it into resource.car"
        shouldRunAfter("processPlugins")
        val luac = "$nativeDir/Corona/$shortOsName/bin/luac"
        val coronaBuilder = if (windows) {
            "$nativeDir/Corona/win/bin/CoronaBuilder.exe"
        } else {
            "$nativeDir/Corona/$shortOsName/bin/CoronaBuilder.app/Contents/MacOS/CoronaBuilder"
        }

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
            val luaFiles = if (coronaTmpDir == null) {
                srcLuaFiles + pluginLuaFiles
            } else {
                pluginLuaFiles
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
                            .removePrefix("lua.lua_51.")
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
            } + if (coronaTmpDir != null) {
                fileTree(coronaTmpDir!!) {
                    include("*.lu")
                }
            } else {
                files().asFileTree
            }
            val buildId = file(generatedBuildIdPath)
                    .takeIf { it.exists() }?.let {
                        file(generatedBuildIdPath).readText().trim()
                    } ?: "unknown"
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
                if (windows) environment["PATH"] = windowsPathHelper
                commandLine(coronaBuilder, "car", "-f", "-", compiledLuaArchive)
            }
        }
    }

    val taskCopyResources = tasks.create<Copy>("packageCoronaApp${baseName.capitalize()}") {
        group = "Corona"
        description = "Copies all resources and compiled Lua to the project"

        dependsOn(compileLuaTask)

        into(generatedAssetsDir)
        from("$coronaTmpDir/output/assets")
        from(compiledLuaArchive)

        from(coronaSrcDir) {
            file("$coronaTmpDir/excludesfile.properties").takeIf { it.exists() }?.readLines()?.forEach {
                exclude(it)
            }
            exclude("**/Icon\r")
            exclude("AndroidResources/**")
            if (isLiveBuild) {
                into("corona_live_build_app_")
            } else {
                exclude("**/*.lua", "build.settings")
            }
        }


        doFirst {
            delete(generatedAssetsDir)
            mkdir(generatedAssetsDir)
        }
        doFirst {
            if (!file(coronaSrcDir).isDirectory) {
                throw InvalidUserDataException("Unable to find Corona project to build!")
            }
        }
    }

    mergeAssetsProvider!!.configure {
        dependsOn(taskCopyResources)
    }
    android.sourceSets[name].assets.srcDirs(generatedAssetsDir)
}

fun downloadPluginsBasedOnBuilderOutput(builderOutput: ByteArrayOutputStream): Int {
    val coronaAndroidPluginsCache = file(if (windows) {
        "${System.getenv("APPDATA")}/Corona Labs/Corona Simulator/build cache/android"
    } else {
        "${System.getenv("HOME")}/Library/Application Support/Corona/build cache/android"
    })
    coronaAndroidPluginsCache.mkdirs()
    mkdir(generatedPluginsOutput)
    val builderOutputStr = builderOutput.toString()
    builderOutputStr.lines()
            .filter { it.startsWith("SPLASH\t") }
            .map { it.removePrefix("SPLASH\t").trim() }
            .lastOrNull()?.let {
                file(generatedControlPath).writeText(it)
            }
    builderOutputStr.lines()
            .filter { it.startsWith("BUILD\t") }
            .map { it.removePrefix("BUILD\t").trim() }
            .lastOrNull()?.let {
                file(generatedBuildIdPath).writeText(it)
            }
    val pluginUrls = builderOutputStr
            .lines()
            .filter { it.startsWith("plugin\t") }
            .map { it.trim().removePrefix("plugin\t").split("\t") }
    pluginUrls.forEach { (plugin, url) ->
        val outputFile = with(DownloadAction(project)) {
            src(url)
            dest("$coronaAndroidPluginsCache/$plugin")
            onlyIfModified(true)
            cachedETagsFile("${coronaAndroidPluginsCache.parent}/ETags.txt")
            execute()
            outputFiles.first()
        }
        copy {
            from(tarTree(outputFile))
            into("$coronaPlugins/${outputFile.nameWithoutExtension}")
        }
    }
    return pluginUrls.count()
}

fun downloadAndProcessCoronaPlugins(reDownloadPlugins: Boolean = true) {

    if (buildSettings == null) {
        parseBuildSettingsFile()
    }

    val luaVerbosityPlug = if (!logger.isLifecycleEnabled) {
        arrayOf("-e", "printError=print;print=function()end")
    } else {
        arrayOf()
    }

    // Download plugins
    logger.lifecycle("Authorizing plugins")
    if (reDownloadPlugins) {

        delete(coronaPlugins)

        val coronaBuilder = if (windows) {
            "$nativeDir/Corona/win/bin/CoronaBuilder.exe"
        } else {
            "$nativeDir/Corona/$shortOsName/bin/CoronaBuilder.app/Contents/MacOS/CoronaBuilder"
        }

        run {
            val buildPropsFile = file("$coronaTmpDir/build.properties")
            val inputSettingsFile = if (buildPropsFile.exists()) {
                buildPropsFile
            } else {
                file("$coronaSrcDir/build.settings")
            }

            val builderOutput = ByteArrayOutputStream()
            val execResult = exec {
                val buildData = coronaBuildData ?: fakeBuildData ?: "{}"
                commandLine(coronaBuilder, "plugins", "download", "android", inputSettingsFile, "--android-build", "--build-data", buildData)
                if (windows) environment["PATH"] = windowsPathHelper
                standardOutput = builderOutput
                isIgnoreExitValue = true
            }
            if (execResult.exitValue != 0) {
                logger.error("Error while fetching plugins")
                logger.error(builderOutput.toString())
                execResult.rethrowFailure()
                throw InvalidPluginException("Error while fetching plugins")
            }
            logger.lifecycle("Downloading plugins")
            delete(generatedControlPath)
            downloadPluginsBasedOnBuilderOutput(builderOutput)
        }

        logger.lifecycle("Fetching plugin dependencies")
        var didDownloadSomething: Boolean
        do {
            processPluginGradleScripts()
            val pluginDirectoriesSet = file(coronaPlugins)
                    .walk()
                    .maxDepth(1)
                    .filter { it.isDirectory && it != coronaPlugins }
                    .map { it.relativeTo(coronaPlugins).path }
                    .toSet()
            val pluginDirectories = (pluginDirectoriesSet - pluginDisabledDependencies).toTypedArray()
            val builderOutput = ByteArrayOutputStream()
            val execResult = exec {
                commandLine(coronaBuilder, "plugins", "download", "android", "--fetch-dependencies", coronaPlugins, *pluginDirectories)
                if (windows) environment["PATH"] = windowsPathHelper
                standardOutput = builderOutput
                isIgnoreExitValue = true
            }
            if (execResult.exitValue != 0) {
                logger.error("Error while fetching plugin dependencies")
                logger.error(builderOutput.toString())
                execResult.rethrowFailure()
                throw InvalidPluginException("Error while fetching plugin dependencies")
            }
            didDownloadSomething = downloadPluginsBasedOnBuilderOutput(builderOutput) > 0
        } while (didDownloadSomething)
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
                exclude("**/*.lua")
            }
            into("$generatedPluginAssetsDir/.corona-plugins")
            eachFile {
                path = File(path).toPathString().segments.drop(3).joinToString("/")
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
        file("$generatedPluginsOutput/resourceDirectories.json").writeText(JsonOutput.toJson(resDirectories))

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
            commandLine("$nativeDir/Corona/$shortOsName/bin/lua"
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
            buildPropsOut.writeText(JsonOutput.toJson(buildSettings))
            buildPropsOut
        }

        val manifestGenDir = "$buildDir/intermediates/corona_manifest_gen"
        exec {
            file(manifestGenDir).mkdirs()
            commandLine("$nativeDir/Corona/$shortOsName/bin/lua"
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
}

tasks.create("processPlugins") {
    group = "Corona"
    doLast {
        downloadAndProcessCoronaPlugins()
    }
}

tasks.create("processPluginsNoDownload") {
    group = "Corona"
    doLast {
        downloadAndProcessCoronaPlugins(false)
    }
}

tasks.create("parseBuildSettings") {
    group = "Corona"
    doLast {
        parseBuildSettingsFile()
    }
}


fun parseBuildSettingsFile() {
    val buildSettingsFile = file("$coronaSrcDir/build.settings")
    if (buildSettings != null) {
        return
    }
    if (!buildSettingsFile.exists()) {
        return
    }

    val output = ByteArrayOutputStream()
    exec {
        setWorkingDir("$nativeDir/Corona/$shortOsName/bin")
        commandLine("$nativeDir/Corona/$shortOsName/bin/lua",
                "-e",
                "package.path='$nativeDir/Corona/shared/resource/?.lua;'..package.path",
                "-e",
                """
                        pcall( function() dofile('${buildSettingsFile.path.replace("\\", "\\\\")}') end )
                        if type(settings) == 'table' then
                            print(require('json').encode(settings))
                        else
                            print('{}')
                        end
                    """.trimIndent()
        )
        standardOutput = output
        if (windows) environment["PATH"] = windowsPathHelper
    }
    fakeBuildData = output.toString()
    buildSettings = mapOf("buildSettings" to JsonSlurper().parseText(fakeBuildData), "packageName" to android.defaultConfig.applicationId)
}
//endregion


//region Corona core development helpers
tasks.register<Zip>("exportCoronaAppTemplate") {
    group = "Corona-dev"
    destinationDirectory.set(file("$buildDir/outputs"))
    archiveFileName.set("android-template.zip")
    from(rootDir) {
        include("build.gradle.kts", "settings.gradle.kts")
        include("gradlew", "gradlew.bat", "gradle/wrapper/**")
        include("app/**")
        exclude("app/build/**", "app/CMakeLists.txt")
        exclude("**/*.iml", "**/\\.*")
        include("setup.sh", "setup.bat")
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

val coronaNativeOutputDir = project.findProperty("coronaNativeOutputDir") as? String
        ?: "$nativeDir/Corona"

tasks.register<Copy>("installAppTemplateToNative") {
    group = "Corona-dev"
    dependsOn("exportCoronaAppTemplate")
    from("$buildDir/outputs") {
        include("android-template.zip")
    }
    into("$coronaNativeOutputDir/android/resource")
}

tasks.register<Copy>("installAppTemplateAndAARToNative") {
    group = "Corona-dev"
    dependsOn("installAppTemplateToNative")
    dependsOn(":Corona:assembleRelease")
    from("${findProject(":Corona")?.buildDir}/outputs/aar/") {
        include("Corona-release.aar")
        rename("Corona-release.aar", "Corona.aar")
    }
    into("$coronaNativeOutputDir/android/lib/gradle")
}

//endregion

//region Corona Project icons

val cleanupIconsDir = tasks.create<Delete>("cleanupIconsDirectory") {
    group = "Corona"
    delete(generatedMainIconsAndBannersDir)
}

val copyCoronaIconFiles = tasks.create("copyCoronaIconFiles") {
    description = "Copies icon files to appropriate resources location"
    group = "Corona"
    tasks.findByName("preBuild")?.dependsOn(this)
}

tasks.create<Copy>("copyAdaptiveIconResources") {
    group = "Corona"
    dependsOn(copyCoronaIconFiles, cleanupIconsDir)

    into(generatedMainIconsAndBannersDir)
    from("$coronaSrcDir/AndroidResources/res")
}

tasks.create("copyCoronaIcons") {
    description = "Copies icon files to appropriate resources location"
    group = "Corona"
    tasks.findByName("preBuild")?.dependsOn(this)
    dependsOn("copyAdaptiveIconResources")
    dependsOn(copyCoronaIconFiles)
}

tasks.create<Copy>("copyMainApplicationIcon") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    into(generatedMainIconsAndBannersDir)
    rename { "icon.png" }

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
        delete("$generatedMainIconsAndBannersDir/**/icon.png")
    }
}


tasks.create<Copy>("copyMainApplicationBanner") {
    group = "Corona"
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
    group = "Corona"
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
    group = "Corona"
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
    group = "Corona"
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
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    val control = file(generatedControlPath).takeIf { it.exists() }?.readText()?.trim()
    if (control == null) {
        from(projectDir) {
            include("_corona_splash_screen.png")
        }
    } else {
        if (control != "nil") {
            from(projectDir) {
                include(control)
            }
        }
    }

    into("$generatedMainIconsAndBannersDir/drawable")
    rename {
        "_corona_splash_screen.png"
    }
    doFirst {
        delete("$generatedMainIconsAndBannersDir/**/_corona_splash_screen.png")
    }
}

//endregion

tasks.create<Copy>("buildCoronaApp") {
    group = "Corona"
    description = "Used when Simulator invokes a build. It all project variables must be passed"
    dependsOn("assembleRelease")
    dependsOn("bundleRelease")

    val copyTask = this
    android.applicationVariants.matching {
        it.name.compareTo("release", true) == 0
    }.all {
        copyTask.from(packageApplicationProvider!!.get().outputDirectory) {
            include("*.apk")
            exclude("*unsigned*")
        }
        copyTask.from("$buildDir/outputs/bundle/$name") {
            include("*.aab")
        }
    }
    rename {
        "$coronaAppFileName.${file(it).extension}"
    }
    coronaDstDir?.let {
        into(it)
        doFirst {
            delete("$it/$coronaAppFileName.apk")
            delete("$it/$coronaAppFileName.aab")
        }
    }
}

repositories {
    flatDir {
        dir("libs")
    }
}

dependencies {
    val buildFromSource = file("CMakeLists.txt").exists() && file("../sdk").exists()
    if (buildFromSource) {
        implementation(project(":Corona"))
        implementation(files("$rootDir/../../plugins/build/licensing-google/android/bin/classes.jar"))
    } else {
        implementation(group = "", name = "Corona", ext = "aar")
        implementation(fileTree("libs") {
            include("**/*.jar")
        })
    }
    implementation(fileTree(coronaPlugins) {
        include("*/*.jar")
        pluginDisabledJar.forEach {
            exclude("**/$it/**")
        }
    })
    implementation(fileTree(coronaPlugins) {
        include("**/*.aar")
    })
    implementation(fileTree(coronaPlugins) {
        include("*/jarLibs/*.jar")
    })
    implementation(fileTree("$coronaSrcDir/AndroidResources") {
        include("**/*.jar", "**/*.aar")
    })
}
