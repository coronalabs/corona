# R8 keep rules for Solar2D apps. Applied to release builds when build.settings sets android.minify = true.
# Projects can add rules in AndroidResources/proguard-rules.pro; plugins can ship proguard*.pro / proguard*.txt.

# Keep line numbers so stack traces can be deobfuscated with the mapping file.
-keepattributes SourceFile,LineNumberTable,*Annotation*,Signature,InnerClasses,EnclosingMethod,Exceptions
-renamesourcefileattribute SourceFile

# Engine and JNLua: called from native code through JNI by name.
-keep class com.ansca.** { *; }
-keep class com.coronalabs.** { *; }
-keep class com.naef.** { *; }

# Plugins: the engine loads "<library name>.LuaLoader" by reflection (NativeToJavaBridge), and plugin
# native libraries call back into their Java classes by name.
-keep class **.LuaLoader { *; }
-keep class **.LuaLoader$* { *; }
-keep class plugin.** { *; }
-keep class * implements com.naef.jnlua.JavaFunction { *; }
-keep class * implements com.naef.jnlua.NamedJavaFunction { *; }
-keep class * implements com.ansca.corona.CoronaRuntimeListener { *; }

# Classes with native methods must keep their names for JNI registration.
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}

# R classes are read by reflection (ResourceServices.getRClass).
-keep class **.R { *; }
-keep class **.R$* { *; }

# Optional dependencies that are not linked into every build.
-dontwarn androidx.window.**
-dontwarn com.google.android.vending.licensing.**

# Store SDKs bundled inside plugins without keep rules of their own. They use reflection internally:
# without these, Amazon IAP fails at runtime with a NullPointerException.
-keep class com.amazon.** { *; }
-keep class com.samsung.** { *; }
-keep interface com.amazon.** { *; }
-keep interface com.samsung.** { *; }

# Plugins often reference classes that are not linked into the app (compile-only annotations, optional
# SDKs). The same classes are missing without R8, so do not fail the build over them.
-ignorewarnings
