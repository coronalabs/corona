rootProject.name = "Corona Android"
include(":Corona", ":App")

project(":Corona").projectDir = file("sdk")
project(":App").projectDir = file("app")
