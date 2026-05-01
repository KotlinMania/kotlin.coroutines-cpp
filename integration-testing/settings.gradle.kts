pluginManagement {
    repositories {
        mavenCentral()
        maven("https://plugins.gradle.org/m2/")
        maven("https://maven.pkg.jetbrains.space/kotlin/p/kotlin/dev")
    }
}

include("smokeTest")

listOf("java8Test", "jpmsTest").forEach { projectName ->
    if (file(projectName).isDirectory) {
        include(projectName)
    }
}

rootProject.name = "kotlinx-coroutines-integration-testing"
