rootProject.name = "root"
include(":Corona", ":JNLua", ":app")

project(":JNLua").projectDir = file("../../external/JNLua/")
project(":Corona").projectDir = file("sdk")
project(":app").projectDir = file("app")
