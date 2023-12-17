plugins {
    id("com.android.library")
}

android {
    ndkVersion = "18.1.5063045"
    compileSdk = 33

    defaultConfig {
        minSdk = 15
        targetSdk = 33
        version = 1
    }
    sourceSets["main"].manifest.srcFile(file("AndroidManifest-New.xml"))
    sourceSets["main"].java.srcDirs(file("src"), file("../../../external/JNLua/src/main"), file("../../../plugins/network/android/src"))
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

tasks.create<Copy>("generateNetworkHelper") {

    from("../../../plugins/network/android/src/")
    include("**/LuaHelper.java.template")
    val outputDir = file("$buildDir/generated/source/networkJava")
    into(outputDir)
    rename { it.removeSuffix(".template") }

    val networkLua = file("../../../plugins/network/shared/network.lua")
    val luaCode = networkLua.readText()
            .replace("lib.", "network.")
            .replace("\\", "\\\\")
            .replace("\"", "\\\"")
            .lines()
            .joinToString("\\n\" +\n\"","\"", "\";")
    this.inputs.file(networkLua)
    expand(mutableMapOf("luaCode" to luaCode))

    val task = this
    android.libraryVariants.all {
        registerJavaGeneratingTask(task, outputDir)
    }
}
