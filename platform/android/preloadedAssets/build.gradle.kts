import com.android.build.gradle.internal.tasks.factory.dependsOn

plugins {
    id("com.android.asset-pack")
}

assetPack {
    packName.set("preloadedAssets")
    dynamicDelivery {
        deliveryType.set("install-time")
    }

}

val copyS2DAssets = tasks.register<Sync>("copyS2DAssets") {
    description = "sync assets"
    val expansionTask = tasks.getByPath(":App:createExpansionFile") as Zip
    from(zipTree(expansionTask.archiveFile))
    into("./src/main/assets")
    include("**/*")
}

project.afterEvaluate {
    tasks.named("generateAssetPackManifest")?.dependsOn(copyS2DAssets)
    copyS2DAssets.dependsOn(":App:createExpansionFile")
}

