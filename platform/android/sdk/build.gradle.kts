plugins {
    id("com.android.library")
}

android {
    ndkVersion = "18.1.5063045"
    compileSdkVersion(29)

    defaultConfig {
        minSdkVersion(15)
        targetSdkVersion(29)
        versionCode = 1
        versionName = "1.0"
    }

    sourceSets["main"].manifest.srcFile(file("AndroidManifest-New.xml"))
    sourceSets["main"].java.srcDirs(file("src"), file("../../../external/JNLua/src/main"))
    sourceSets["main"].java.filter.exclude("**/script/**")
    sourceSets["main"].res.srcDirs(file("res-new"))

    externalNativeBuild {
        cmake {
            path = file("../sdk/CMakeLists.txt")
        }
    }
    useLibrary("org.apache.http.legacy")

    libraryVariants.all {
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

tasks.create<Copy>("updateWidgetResources") {
    group = "Corona"
    val widgetResLocation = "$buildDir/generated/widgetResources"
    from("../../../subrepos/widget/")
    include("*.png")
    into("$widgetResLocation/raw")
    rename { "corona_asset_$it".replace("@", "_").toLowerCase() }
    val task = this
    android.libraryVariants.all {
        registerGeneratedResFolders(files(widgetResLocation) {
            builtBy(task)
        })
    }
}

dependencies {
    api(files("../../../plugins/build-core/network/android/network.jar"))
}
