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

    sourceSets["main"].manifest.srcFile(file("AndroidManifest.xml"))
    sourceSets["main"].java.srcDirs(file("src"))
    sourceSets["main"].res.srcDirs(file("res"))
    sourceSets["main"].res.srcDirs(file(sharedResLocation))

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

tasks.create<Copy>("splashScreenChecker") {
    group = "Corona"

    val outputDir = file("$buildDir/generated/source/corona")
    val splash = "res/drawable/_corona_splash_screen.png"
    inputs.file(splash)
    from("SplashScreenBeacon.java.template")
    into("$outputDir/com/ansca/corona")
    rename("SplashScreenBeacon.java.template", "SplashScreenBeacon.java")
    filter {
        it.replace("999", file(splash).length().toString())
    }
    val task = this
    android.libraryVariants.all {
        this.registerJavaGeneratingTask(task, outputDir)
    }
}

tasks.create<Copy>("updateWidgetResources") {
    group = "Corona"
    from("../../../subrepos/widget/")
    include("*.png")
    into("$sharedResLocation/raw")
    rename { "corona_asset_$it".replace("@", "_").toLowerCase() }

    val task = this
    android.libraryVariants.all {
        mergeResourcesProvider!!.configure {
            dependsOn(task)
        }
    }
}

dependencies {
    implementation(project(":JNLua"))
    implementation(files("../../../plugins/build-core/network/android/network.jar"))
}
