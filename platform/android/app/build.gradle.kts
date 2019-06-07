import groovy.json.JsonSlurper
import org.apache.commons.io.output.ByteArrayOutputStream
import org.apache.tools.ant.filters.StringInputStream


plugins {
    id("com.android.application")
}

val coronaDstDir: String? by project
val coronaTmpDir: String? by project
val coronaSrcDirSim: String? by project
val coronaAppFileName: String? by project
val coronaAppPackage: String? by project
val coronaVersionCode: String? by project
val coronaVersionName: String? by project
val coronaKeystore: String? by project
val coronaKeystorePassword: String? by project
val coronaKeyAlias: String? by project
val coronaKeyAliasPassword: String? by project

val coronaSrcDir = coronaSrcDirSim ?: if (file("$rootDir/../test/assets2").exists()) {
    "$rootDir/../test/assets2"
} else {
    "$rootDir/../Corona"
}

val windows = System.getProperty("os.name").toLowerCase().contains("windows")
val shortOsName = if (windows) "win" else "mac"
val nativeDir = if (windows) {
    System.getenv("CORONA_ROOT").replace("\\", "/")
} else {
    "${System.getenv("HOME")}/Library/Application Support/Corona/Native/"
}


var buildSettings: Any? = null
coronaTmpDir?.let { srcDir ->
    file("$srcDir/build.properties").takeIf { it.exists() }?.let { f ->
        buildSettings = JsonSlurper().parse(f)
        if (buildSettings is Map<*, *>) {
            buildSettings = (buildSettings as Map<*, *>)["buildSettings"]
        }
    }
}
if (buildSettings == null) {
    val buildSettingsFile = file("$coronaSrcDir/build.settings")
    if (buildSettingsFile.exists()) {
        tasks.create<Exec>("parseBuildSettings") {

            val output = ByteArrayOutputStream()
            setWorkingDir("$nativeDir/Corona/$shortOsName/bin")
            commandLine("./lua",
                    "-e",
                    "package.path='$nativeDir/Corona/shared/resource/?.lua;'..package.path",
                    "-e",
                    """
                        pcall( function() dofile('${buildSettingsFile.path}') end	)
                        if type(settings) == "table" then
                            print(require('json').encode(settings))
                        end
                    """.trimIndent()
            )
            standardOutput = output
            if (windows) {
                environment["PATH"] = "${System.getenv("PATH")}${System.getProperty("path.separator")}${System.getenv("CORONA_PATH")}"
            }
            doLast {
                buildSettings = JsonSlurper().parseText(output.toString())
            }
        }
    } else {
        buildSettings = JsonSlurper().parseText("{}")
        tasks.create("parseBuildSettings")
    }.group = "Corona"
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
}


android.applicationVariants.all {
    val baseName = this.baseName.toLowerCase()
    val isRelease = (baseName == "release")
    val generatedAssetsDir = "$buildDir/generated/coronaAssets/$baseName"
    val compiledLuaArchive = "$buildDir/generated/compiledLua/$baseName/resource.car"

    val compileLuaTask = tasks.create("compileLua${baseName.capitalize()}") {
        group = "Corona"
        description = "If required, compiles Lua and archives it into resource.car"
        val luac = "$nativeDir/Corona/$shortOsName/bin/luac"
        val coronaBuilder = if (windows) {
            "$nativeDir/Corona/win/bin/CoronaBuilder.exe"
        } else {
            "$nativeDir/Corona/$shortOsName/bin/CoronaBuilder.app/Contents/MacOS/CoronaBuilder"
        }

        val luaFiles = fileTree(coronaSrcDir) {
            include("**/*.lua")
        }
        inputs.files(luaFiles)
        outputs.file(compiledLuaArchive)
        doLast {
            val rootFile = file(coronaSrcDir)
            val compiledDir = "$buildDir/intermediates/compiledLua"
            delete(compiledDir)
            mkdir(compiledDir)
            val outputsList = mutableListOf<String>()
            luaFiles.forEach {
                exec {
                    val compiled = it.relativeTo(rootFile).path.replace(File.separator, ".").replaceAfterLast(".", "lu")
                    outputsList += compiled
                    val additionalArguments = mutableListOf<String>()
                    if (isRelease) {
                        additionalArguments += "-s"
                    }
                    commandLine(luac, *additionalArguments.toTypedArray(), "-o", "$compiledDir/$compiled", "--", it)
                }
            }
            mkdir(file(compiledLuaArchive).parent)
            exec {
                workingDir = file(compiledDir)
                standardInput = StringInputStream(outputsList.joinToString("\n"))
                if (windows) {
                    environment["PATH"] = "${System.getenv("PATH")}${File.pathSeparator}${System.getenv("CORONA_PATH")}"
                }
                commandLine(coronaBuilder, "car", "-f", "-", compiledLuaArchive)
            }
        }
    }

    val task = tasks.create<Copy>("packageCoronaApp${baseName.capitalize()}") {
        group = "Corona"
        description = "Copies all resources and compiled Lua to the project"

        from(coronaSrcDir) {
            file("$coronaTmpDir/excludesfile.properties").takeIf { it.exists() }?.readLines()?.forEach {
                exclude(it)
            }
            exclude("**/*.lua", "build.settings")
            exclude("**/Icon\r")
        }

        if (coronaTmpDir != null) {
            from("$coronaTmpDir/resource.car")
        } else {
            from(compiledLuaArchive)
            dependsOn(compileLuaTask)
        }

        into(generatedAssetsDir)

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
        dependsOn(task)
    }
    android.sourceSets[name].assets.srcDirs(generatedAssetsDir)
}

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
        include("template.properties")
        rename("template.properties", "local.properties")
        into("template")
    }
    from(android.sdkDirectory) {
        include("licenses/android-sdk-license")
        into("sdk")
    }
    doLast {
        println("Exported to '${archiveFile.get()}'")
    }
}

tasks.register<Copy>("installAppTemplateToNative") {
    group = "Corona-dev"
    dependsOn("exportCoronaAppTemplate")
    from("$buildDir/outputs") {
        include("android-template.zip")
    }
    into("$nativeDir/Corona/android/resource")
}

tasks.register<Copy>("installAppTemplateAndAARToNative") {
    group = "Corona-dev"
    dependsOn("installAppTemplateToNative")
    dependsOn(":Corona:assembleRelease")
    from("${findProject(":Corona")?.buildDir}/outputs/aar/") {
        include("Corona-release.aar")
        rename("Corona-release.aar", "Corona.aar")
    }
    into("$nativeDir/Corona/android/lib/gradle")
}
//endregion

//region Corona Project icons

val resourceOutputDirectory = "$buildDir/generated/coronaIcons/src/main/res"
android.sourceSets["main"].res.srcDirs(resourceOutputDirectory)

val cleanupIconsDir = tasks.create<Delete>("cleanupIconsDirectory") {
    group = "Corona"
    delete(resourceOutputDirectory)
}

val copyCoronaIconFiles = tasks.create("copyCoronaIconFiles") {
    description = "Copies icon files to appropriate resources location"
    group = "Corona"
    tasks.findByName("preBuild")?.dependsOn(this)
    if (buildSettings == null) {
        dependsOn("parseBuildSettings")
    }
}

tasks.create<Copy>("copyAdaptiveIconResources") {
    group = "Corona"
    dependsOn(copyCoronaIconFiles, cleanupIconsDir)

    into(resourceOutputDirectory)
    from("$coronaSrcDir/AndroidResources/res")
}

tasks.create("copyCoronaIcons") {
    description = "Copies icon files to appropriate resources location"
    group = "Corona"
    tasks.findByName("preBuild")?.dependsOn(this)
    if (buildSettings == null) {
        dependsOn("parseBuildSettings")
    }
    dependsOn("copyAdaptiveIconResources")
    dependsOn(copyCoronaIconFiles)
}

tasks.create<Copy>("copyMainApplicationIcon") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    into(resourceOutputDirectory)
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
        delete("$resourceOutputDirectory/**/icon.png")
    }
}


tasks.create<Copy>("copyMainApplicationBanner") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Banner-xhdpi.png")
    }
    into("$resourceOutputDirectory/drawable-xhdpi")
    rename { "banner.png" }
    doFirst {
        delete("$resourceOutputDirectory/**/banner.png")
    }
}

tasks.create<Copy>("copyOuyaApplicationIcon") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Icon-ouya.png")
    }
    into("$resourceOutputDirectory/drawable-xhdpi")
    rename { "ouya_icon.png" }
    doFirst {
        delete("$resourceOutputDirectory/**/ouya_icon.png")
    }
}

tasks.create<Copy>("copyXiaomiApplicationIcon") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(coronaSrcDir) {
        include("Icon-ouya-xiaomi.png")
    }
    into("$resourceOutputDirectory/drawable-xhdpi")
    rename { "ouya_xiaomi_icon.png" }
    doFirst {
        delete("$resourceOutputDirectory/**/ouya_xiaomi_icon.png")
    }
}

tasks.create<Copy>("copyDefaultNotificationIcons") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    into(resourceOutputDirectory)
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
        delete("$resourceOutputDirectory/**/corona_statusbar_icon_default.png")
    }
}

tasks.create<Copy>("copySplashScreen") {
    group = "Corona"
    copyCoronaIconFiles.dependsOn(this)
    dependsOn(cleanupIconsDir)

    from(projectDir) {
        include("_corona_splash_screen.png")
    }
    into("$resourceOutputDirectory/drawable")
    doFirst {
        delete("$resourceOutputDirectory/**/_corona_splash_screen.png")
    }
}

//endregion

tasks.create<Copy>("buildCoronaApp") {
    group = "Corona"
    description = "Used when Simulator invokes a build. It all project variables must be passed"
    dependsOn("assembleRelease")
    dependsOn("bundleRelease")
    tasks.findByName("assembleRelease")?.shouldRunAfter("clean")
    tasks.findByName("bundleRelease")?.shouldRunAfter("clean")

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
    } else {
        implementation(group = "", name = "Corona", ext = "aar")
    }
}
