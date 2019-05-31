import org.apache.tools.ant.filters.StringInputStream

plugins {
    id("com.android.application")
}

val coronaDstDir: String? by project
val coronaTmpDir: String? by project
val coronaSrcDir: String? by project
val coronaAppFileName: String? by project
val coronaAppPackage: String? by project
val coronaVersionCode: String? by project
val coronaVersionName: String? by project
val coronaKeystore: String? by project
val coronaKeystorePassword: String? by project
val coronaKeyAlias: String? by project
val coronaKeyAliasPassword: String? by project

val coronaSrcDirFinal = coronaSrcDir ?: if (file("$rootDir/../test/assets2").exists()) {
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
    val baseName = this.dirName.toLowerCase()
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

        val luaFiles = fileTree(coronaSrcDirFinal) {
            include("**/*.lua")
        }
        inputs.files(luaFiles)
        outputs.file(compiledLuaArchive)
        doLast {
            val rootFile = file(coronaSrcDirFinal)
            val compiledDir = "$buildDir/intermediates/compiledLua"
            delete(compiledDir)
            mkdir(compiledDir)
            val outputsList = mutableListOf<String>()
            luaFiles.forEach {
                exec {
                    val compiled = it.relativeTo(rootFile).path.replace(File.separator, ".").replaceAfterLast(".", "lu")
                    outputsList += compiled
                    if (isRelease) {
                        commandLine(luac, "-s", "-o", "$compiledDir/$compiled", "--", it)
                    } else {
                        commandLine(luac, "-s", "-o", "$compiledDir/$compiled", "--", it)
                    }
                }
            }
            mkdir(file(compiledLuaArchive).parent)
            exec {
                workingDir = file(compiledDir)
                standardInput = StringInputStream(outputsList.joinToString("\n"))
                if(windows) {
                    environment["PATH"] = "${System.getenv("PATH")}${File.pathSeparator}${System.getenv("CORONA_PATH")}"
                }
                commandLine(coronaBuilder, "car", "-f", "-", compiledLuaArchive)
            }
        }
    }

    val task = tasks.create<Copy>("packageCoronaApp${baseName.capitalize()}") {
        group = "Corona"
        description = "Copies all resources and compiled Lua to the project"

        from(coronaSrcDirFinal) {
            file("$coronaTmpDir/excludesfile.properties").takeIf { it.exists() }?.readLines()?.forEach {
                exclude(it)
            }
            exclude("**/*.lua")
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
            if (!file(coronaSrcDirFinal).isDirectory) {
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
