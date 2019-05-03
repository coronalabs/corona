plugins {
    id("com.android.application")
}

@Suppress("OldTargetApi")
android {
    compileSdkVersion(27)
    defaultConfig {
        applicationId = "com.corona.template"
        minSdkVersion(15)
        targetSdkVersion(27)
        versionCode = 1
        versionName = "1.0"
        multiDexEnabled = true
    }
    buildTypes {
        getByName("release") {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android.txt"), "proguard-rules.pro")
        }
    }
    externalNativeBuild {
        cmake {
            setPath("CMakeLists.txt")
        }
    }

    applicationVariants.all {
        generateBuildConfigProvider.configure {
            enabled = false
        }
    }
}


//val windows = System.getProperty("os.name").toLowerCase().contains("windows")
val nativeDir = "${System.getenv("HOME")}/Library/Application Support/Corona/Native/"


android.applicationVariants.all {
    val baseName = this.dirName.toLowerCase()
    val generatedAssetsDir = "$buildDir/generated/coronaAssets/$baseName"

    val task = tasks.create<Exec>("packageCoronaApp${baseName.capitalize()}") {
        description = "Compile Lua project"
        group = "Corona"

        setWorkingDir("$nativeDir/Corona/mac/bin")
        val packagePath = """package.path="$nativeDir/Corona/shared/bin/?.lua;$nativeDir/Corona/shared/bin/?/init.lua;"..package.path"""
        val compileLua = "$nativeDir/Corona/shared/bin/Compile.lua"
        val platform = "mac"

        commandLine("$nativeDir/Corona/mac/bin/lua",
                "-e",
                packagePath,
                compileLua,
                platform,
                nativeDir
        )

        val cpath = "$nativeDir/Corona/mac/bin/?.so"
        val coronaAssetsDir = "$projectDir/../../test/assets2"

//      environment "PATH", "$execPath"
        environment["LUA_CPATH"] = cpath
        environment["TARGET_PLATFORM"] = "android"
        environment["PROJECT_DIR"] = "$rootDir"
        environment["CORONA_COPY_PNG_PRESERVE"] = "--preserve"
        environment["CONFIGURATION"] = baseName
        assert(baseName == "release" || baseName == "debug")
        environment["CORONA_ASSETS_DIR"] = coronaAssetsDir
        environment["CORONA_TARGET_RESOURCES_DIR"] = generatedAssetsDir
        environment["CORONA_TARGET_EXECUTABLE_DIR"] = generatedAssetsDir

        inputs.dir(coronaAssetsDir)
        outputs.dir(generatedAssetsDir)

        doFirst {
            delete(generatedAssetsDir)
            mkdir(generatedAssetsDir)
        }
    }

    mergeAssetsProvider.configure {
        dependsOn(task)
    }
    android.sourceSets[name].assets.srcDirs(generatedAssetsDir)
}


dependencies {
    implementation(project(":Corona"))
}
