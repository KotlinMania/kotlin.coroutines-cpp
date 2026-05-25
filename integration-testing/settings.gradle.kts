pluginManagement {
    repositories {
        mavenCentral()
        maven("https://plugins.gradle.org/m2/")
        maven("https://maven.pkg.jetbrains.space/kotlin/p/kotlin/dev")
    }
}

include("smokeTest")
include("java8Test")
include(":jpmsTest")

rootProject.name = "kotlinx-coroutines-integration-testing"
