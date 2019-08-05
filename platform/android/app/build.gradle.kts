import com.android.ide.common.util.toPathString
import com.beust.klaxon.JsonArray
import com.beust.klaxon.JsonObject
import com.beust.klaxon.Klaxon
import com.beust.klaxon.Parser
import de.undercouch.gradle.tasks.download.DownloadAction
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
val coronaAppPackage = project.findProperty("coronaAppPackage") as? String ?: "com.corona.test"
val coronaKeystore: String? by project
val coronaKeystorePassword: String? by project
val coronaKeyAlias: String? by project
val coronaKeyAliasPassword: String? by project
val configureCoronaPlugins: String? by project
val coronaBuild: String? by project
val coronaBuildData: String? by project
val coronaExpansionFileName: String? by project
val coronaCustomHome: String? by project
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

val buildToolsDir = "$projectDir/buildTools".takeIf { file(it).exists() }
        ?: "$projectDir/../template".takeIf { file(it).exists() } ?: {
            copy {
                from(zipTree("$nativeDir/Corona/android/resource/android-template.zip"))
                into("$buildDir/intermediates/corona-build-tools")
            }
            "$buildDir/intermediates/corona-build-tools/template/app/buildTools"
        }()

val generatedPluginsOutput = "$buildDir/generated/corona_plugins"
val generatedPluginAssetsDir = "$generatedPluginsOutput/assets"
val generatedPluginNativeLibsDir = "$generatedPluginsOutput/native"
val generatedControlPath = "$generatedPluginsOutput/control"
val generatedBuildIdPath = "$generatedPluginsOutput/build"
val generatedPluginMegaJar = "$generatedPluginsOutput/plugins.jar"
val generatedMainIconsAndBannersDir = "$buildDir/generated/corona_icons"


var buildSettings: JsonObject? = null
var fakeBuildData: String? = null
coronaTmpDir?.let { srcDir ->
    file("$srcDir/build.properties").takeIf { it.exists() }?.let { f ->
        buildSettings = Parser.default().parse(f.absolutePath) as? JsonObject
    }
}
parseBuildSettingsFile()
val coronaMinSdkVersion = try {
    buildSettings?.obj("buildSettings")?.obj("android")?.let {
        try {
            return@let it.string("minSdkVersion")?.toIntOrNull()
        } catch (ignore: Throwable) {
        }
        try {
            return@let it.int("minSdkVersion")
        } catch (ignore: Throwable) {
        }
        null
    }
} catch (ignore: Throwable) {
    null
} ?: 15

val coronaVersionName = try {
    buildSettings?.obj("buildSettings")?.obj("android")?.let {
        try {
            return@let it.string("versionName")
        } catch (ignore: Throwable) {
        }
        try {
            return@let it.int("versionName")?.toString()
        } catch (ignore: Throwable) {
        }
        null
    }
} catch (ignore: Throwable) {
    null
} ?: project.findProperty("coronaVersionName") as? String ?: "1.0"

val coronaVersionCode: Int = try {
    buildSettings?.obj("buildSettings")?.obj("android")?.let {
        try {
            return@let it.string("versionCode")?.toIntOrNull()
        } catch (ignore: Throwable) {
        }
        try {
            return@let it.int("versionCode")
        } catch (ignore: Throwable) {
        }
        null
    }
} catch (ignore: Throwable) {
    null
} ?: (project.findProperty("coronaVersionCode") as? String)?.toIntOrNull() ?: 1


val coronaAndroidPluginsCache = file(if (windows) {
    if (coronaCustomHome.isNullOrEmpty()) {
        "${System.getenv("APPDATA")}/Corona Labs/Corona Simulator/build cache/android"
    } else {
        "$coronaCustomHome/build cache/android"
    }
} else {
    "${System.getenv("HOME")}/Library/Application Support/Corona/build cache/android"
})
val eTagFileName = "${coronaAndroidPluginsCache.parent}/CoronaETags.txt"


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
    compileSdkVersion(28)
    defaultConfig {
        applicationId = coronaAppPackage
        targetSdkVersion(28)
        minSdkVersion(coronaMinSdkVersion)
        versionCode = coronaVersionCode
        versionName = coronaVersionName
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
        if (coronaTmpDir == null) {
            parseBuildSettingsFile()
            try {
                buildSettings?.obj("buildSettings")?.obj("excludeFiles")?.let {
                    it.array<String>("all")?.forEach { excludeEntry ->
                        exclude("**/$excludeEntry")
                    }
                    it.array<String>("android")?.forEach { excludeEntry ->
                        exclude("**/$excludeEntry")
                    }
                }
            } catch (ignore: Throwable) {
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
                    if (windows) environment["PATH"] = windowsPathHelper
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
                if (windows) environment["PATH"] = windowsPathHelper
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
                throw InvalidUserDataException("Unable to find Corona project to build!")
            }
        }
    }

    mergeAssetsProvider!!.configure {
        dependsOn(taskCopyResources)
    }
    android.sourceSets[name].assets.srcDirs(generatedAssetsDir)
}


fun readETagMap(): Map<String, String> {
    return file(eTagFileName).takeIf { it.exists() }?.readLines()?.map { s ->
        val (k, v) = s.split("\t")
        k to v
    }?.toMap() ?: mapOf()
}

fun downloadPluginsBasedOnBuilderOutput(builderOutput: ByteArrayOutputStream, eTagMap: Map<String, String>, newETagMap: MutableMap<String, String>): Int {
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
        val existingTag = eTagMap[plugin]
        try {
            val outputFile = with(DownloadAction(project)) {
                src(url)
                dest("$coronaAndroidPluginsCache/$plugin")
                val outputFile = outputFiles.first()
                if (existingTag != null && outputFile.exists()) {
                    header("If-None-Match", existingTag)
                }
                responseInterceptor { response, _ ->
                    val eTag = response.getFirstHeader("ETag")
                    if (eTag != null) {
                        newETagMap[plugin] = eTag.value
                    }
                }
                execute()
                outputFile
            }
            copy {
                from(tarTree(outputFile))
                into("$coronaPlugins/${outputFile.nameWithoutExtension}")
            }
        } catch (ex: Exception) {
            if(ex.message?.equals("Not Found", ignoreCase = true) == true) {
                logger.error("WARNING: plugin '${plugin.removeSuffix(".tgz")}' was not found for current platform. Consider disabling it with 'supportedPlatforms' field.")
            } else {
                logger.error("ERROR: There was a problem downloading plugin '${plugin.removeSuffix(".tgz")}'. Please, try again.")
                throw ex
            }
        }
    }
    return pluginUrls.count()
}

fun downloadAndProcessCoronaPlugins(reDownloadPlugins: Boolean = true) {

    parseBuildSettingsFile()

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
        val eTagMap = readETagMap()
        val newETagMap = mutableMapOf<String, String>()

        run {
            val buildPropsFile = file("$coronaTmpDir/build.properties")
            val inputSettingsFile = if (buildPropsFile.exists()) {
                buildPropsFile
            } else {
                file("$coronaSrcDir/build.settings")
            }

            val builderOutput = ByteArrayOutputStream()
            val execResult = exec {
                val buildData = coronaBuildData?.let { file(it).readText() } ?: fakeBuildData
                commandLine(coronaBuilder, "plugins", "download", "android", inputSettingsFile, "--android-build", "--build-data")
                if (windows) environment["PATH"] = windowsPathHelper
                standardInput = StringInputStream(buildData)
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
            downloadPluginsBasedOnBuilderOutput(builderOutput, eTagMap, newETagMap)
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
            didDownloadSomething = downloadPluginsBasedOnBuilderOutput(builderOutput, eTagMap, newETagMap) > 0
        } while (didDownloadSomething)

        if (newETagMap.count() > 0) {
            val combinedETags = readETagMap() + newETagMap
            file(eTagFileName).writeText(combinedETags.map { it.key + "\t" + it.value }.joinToString("\n"))
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
            if (windows) environment["PATH"] = windowsPathHelper
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
            buildPropsOut.writeText(buildSettings!!.toJsonString())
            buildPropsOut
        }

        val manifestGenDir = "$buildDir/intermediates/corona_manifest_gen"
        file(manifestGenDir).mkdirs()
        exec {
            if (windows) environment["PATH"] = windowsPathHelper
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

tasks.register("parseBuildSettings") {
    doLast {
        parseBuildSettingsFile()
    }
}


fun parseBuildSettingsFile() {
    if (buildSettings != null) {
        return
    }
    val buildSettingsFile = file("$coronaSrcDir/build.settings")
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
    val parsedBuildSettingsFile = Parser.default().parse(StringBuilder(fakeBuildData)) as? JsonObject
    buildSettings = JsonObject(mapOf("buildSettings" to parsedBuildSettingsFile, "packageName" to coronaAppPackage))
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

tasks.register<Copy>("exportToNativeTemplate") {
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
            it.replace("com.corona.test", "com.mycompany.app")
        }
    }

    doFirst {
        delete(fileTree(templateDir) {
            exclude("plugin/**")
            exclude("settings.gradle")
            exclude("**/AndroidManifest.xml")
            exclude("**/*.java")
        })
    }
    doLast {
        println("Copied to ${file(templateDir).absolutePath}")
    }
}


val coronaNativeOutputDir = project.findProperty("coronaNativeOutputDir") as? String
        ?: "$nativeDir/Corona"

tasks.register<Copy>("installAppTemplateToNative") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    dependsOn("exportCoronaAppTemplate")
    from("$buildDir/outputs") {
        include("android-template.zip")
    }
    into("$coronaNativeOutputDir/android/resource")
}

tasks.register<Copy>("installAppTemplateAndAARToNative") {
    if (coronaBuiltFromSource) group = "Corona-dev"
    enabled = coronaBuiltFromSource
    dependsOn("installAppTemplateToNative")
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
    copy {
        into(dest)
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
            copy {
                from("$buildDir/outputs/$coronaExpansionFileName")
                into(it)
            }
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

    val control = file(generatedControlPath).takeIf { it.exists() }?.readText()?.trim()
    if (control == null) {
        from(projectDir) {
            include("_corona_splash_screen.png")
        }
    } else if (control != "nil") {
        from(coronaSrcDir) {
            include(control)
        }
        if (inputs.sourceFiles.isEmpty) throw InvalidUserDataException("Custom Splash Screen file '$control' not found!")
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
            val coronaNativeAAR = file("$nativeDir/Corona/android/lib/gradle/Corona.aar")
            if (!coronaNativeAAR.exists()) {
                throw InvalidUserDataException("Corona Native was not set-up properly. Launch `Native/Setup Corona Native.app` on macOS or reinstall on Windows.")
            }
            implementation(files(coronaNativeAAR))
            implementation(files("$nativeDir/Corona/android/lib/Corona/libs/licensing-google.jar"))
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
}
