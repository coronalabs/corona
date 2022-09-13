import com.beust.klaxon.lookup
import com.beust.klaxon.JsonObject
import com.beust.klaxon.JsonArray

buildscript {
    repositories {
        google()
        jcenter()
    }
    dependencies {
        classpath("com.beust:klaxon:5.0.1")
    }
}

rootProject.name = "Corona Android"
include(":App")
project(":App").projectDir = file("app")

if (file("sdk").exists()) {
    include(":Corona")
    project(":Corona").projectDir = file("sdk")
}

val coronaExpansionFileName: String? by settings
if(!coronaExpansionFileName.isNullOrBlank()) {
    include(":preloadedAssets")
}

val coronaTmpDir: String? by settings
val parsedBuildProperties: JsonObject? = run {
    coronaTmpDir?.let { srcDir ->
        file("$srcDir/build.properties").takeIf { it.exists() }?.let { f ->
            return@run com.beust.klaxon.Parser.default().parse(f.absolutePath) as JsonObject
        }
    }
}
parsedBuildProperties?.lookup<JsonArray<JsonObject>>("buildSettings.android.onDemandResources")?.firstOrNull()?.forEach {
    val tag : String = it["tag"]?.toString() ?: throw InvalidPluginException("Missing 'tag' for onDemand/PlayDelivery asset")
    val resource : String = it["resource"]?.toString() ?: throw InvalidPluginException("Missing 'resource' for onDemand/PlayDelivery asset tagged '$tag'")
    val type : String = when( it["type"] ) {
        "prefetch", "fast-follow" -> "fast-follow"
        "install", "install-time" -> "install-time"
        null, "on-demand" -> "on-demand"
        else -> throw InvalidPluginException("Invalid 'type' for onDemand/PlayDelivery resource tagged '$tag'. Must be 'prefetch' ('fast-follow'), 'install' ('install-time') or nil ('on-demand')")
    }
    val padName = "pda-$tag"
    mkdir(padName)
    copy {
        from(".") {
            include("PAD.kts.template")
            rename { "build.gradle.kts" }
        }
        into(padName)
        filter { line ->
            line.replace("pad-pack-name", tag).replace("resource/path", resource).replace("on-demand", type)
        }
    }
    include(":$padName")
}