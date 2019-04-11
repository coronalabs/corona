plugins {
    id("com.android.library")
}

val sharedResLocation = "$buildDir/generated/sharedRes"

@Suppress("OldTargetApi")
android {
    compileSdkVersion(27)
    buildToolsVersion("28.0.3")

    defaultConfig {
        minSdkVersion(15)
        targetSdkVersion(27)
        versionCode = 1
        versionName = "1.0"
        ndk {
//            abiFilters("x86_64", "x86")
        }
    }

    sourceSets["main"].manifest.srcFile(File("AndroidManifest.xml"))
    sourceSets["main"].java.srcDirs(File("src"))
    sourceSets["main"].res.srcDirs(File("res"))
    sourceSets["main"].res.srcDirs(File(sharedResLocation))

    externalNativeBuild {
        cmake {
            setPath("../sdk/CMakeLists.txt")
        }
    }
    useLibrary("org.apache.http.legacy")
}

android.libraryVariants.all {
    generateBuildConfigProvider?.configure {
        enabled = false
    }
}

android.libraryVariants.all {
    val variant = this
    val outputDir = File("$buildDir/generated/source/corona/${variant.dirName}")
    val name = "splashScreenChecker${variant.name.capitalize()}"
    tasks.create<Copy>(name) {
        group = "Corona"

        val splash = "res/drawable/_corona_splash_screen.png"

        from("SplashScreenBeacon.java.template")
        into("$outputDir/com/ansca/corona")
        rename("SplashScreenBeacon.java.template", "SplashScreenBeacon.java")
        filter {
            it.replace("999", file(splash).length().toString())
        }

        doFirst {
            delete("src/com/ansca/corona/SplashScreenBeacon.java")
        }

        inputs.file(splash)

        variant.registerJavaGeneratingTask(this, outputDir)
    }
}

val updateWidgetTask = tasks.register<Copy>("updateWidgetResources") {
    group = "Corona"
    from("../../../subrepos/widget/")
    include("*.png")
    into("$sharedResLocation/raw")
    rename { "corona_asset_$it".replace("@", "_").toLowerCase() }
}
android.libraryVariants.all {
    mergeResourcesProvider?.configure {
        dependsOn(updateWidgetTask)
    }
}

dependencies {
    implementation(project(":JNLua"))
    implementation(files("../../../plugins/build-core/network/android/network.jar"))
}
