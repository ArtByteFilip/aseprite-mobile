import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream

plugins {
    id("com.android.application")
}

val asepriteRoot: File = rootProject.projectDir.parentFile
val skiaDir: String = (findProperty("aseprite.skiaDir") as String?) ?: "C:/deps/skia"
val genExe: String = (findProperty("aseprite.genExe") as String?) ?: "C:/deps/build-gen-host/bin/gen.exe"
val abis: List<String> = ((findProperty("aseprite.abis") as String?) ?: "arm64-v8a,x86_64")
    .split(",").map { it.trim() }.filter { it.isNotEmpty() }

android {
    namespace = "org.aseprite.android"
    compileSdk = 36
    ndkVersion = "28.2.13676358"

    defaultConfig {
        applicationId = "org.aseprite.android"
        minSdk = 26
        targetSdk = 36
        versionCode = 1
        versionName = "1.3-android"

        ndk {
            abiFilters += abis
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_static",
                    "-DLAF_BACKEND=skia",
                    "-DSKIA_DIR=$skiaDir",
                    "-DGEN_EXE=$genExe",
                    "-DENABLE_UPDATER=OFF",
                    "-DENABLE_NEWS=OFF",
                    "-DENABLE_WEBSOCKET=OFF",
                    "-DENABLE_CCACHE=OFF",
                    "-DENABLE_TESTS=OFF",
                    "-DENABLE_BENCHMARKS=OFF",
                    "-DLAF_WITH_EXAMPLES=OFF",
                    "-DLAF_WITH_TESTS=OFF",
                    "-DFULLSCREEN_PLATFORM=ON",
                )
                targets += "aseprite"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = File(asepriteRoot, "CMakeLists.txt")
            version = "3.31.6"
        }
    }

    signingConfigs {
        getByName("debug")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            // Signed with the debug key so the APK can be installed
            // directly (sideloaded) on a device.
            signingConfig = signingConfigs.getByName("debug")
        }
        debug {
            externalNativeBuild {
                cmake {
                    // Debug native builds of Aseprite are too slow to be usable
                    arguments += "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
                }
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    androidResources {
        // data.zip is already compressed
        noCompress += "zip"
    }

    packaging {
        jniLibs {
            useLegacyPackaging = false
        }
    }
}

// Packs the Aseprite data/ directory (skins, palettes, strings,
// scripts, etc.) in assets/data.zip. The activity extracts it in the
// first run (or after an update).
abstract class PackAsepriteDataTask : DefaultTask() {
    @get:InputDirectory
    abstract val dataDir: DirectoryProperty

    @get:InputFiles
    abstract val extraFiles: ConfigurableFileCollection

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @TaskAction
    fun pack() {
        val outDir = outputDir.get().asFile
        outDir.mkdirs()
        val zipFile = File(outDir, "data.zip")
        val root = dataDir.get().asFile
        ZipOutputStream(zipFile.outputStream().buffered()).use { zip ->
            root.walkTopDown().filter { it.isFile }.forEach { file ->
                val rel = file.relativeTo(root).invariantSeparatorsPath
                zip.putNextEntry(ZipEntry("data/$rel"))
                file.inputStream().use { it.copyTo(zip) }
                zip.closeEntry()
            }
            extraFiles.forEach { file ->
                val name = if (file.name == "LICENSES.md") "docs/LICENSES.md" else file.name
                zip.putNextEntry(ZipEntry("data/$name"))
                file.inputStream().use { it.copyTo(zip) }
                zip.closeEntry()
            }
        }
    }
}

val packAsepriteData = tasks.register<PackAsepriteDataTask>("packAsepriteData") {
    dataDir.set(File(asepriteRoot, "data"))
    extraFiles.from(
        File(asepriteRoot, "README.md"),
        File(asepriteRoot, "AUTHORS.md"),
        File(asepriteRoot, "EULA.txt"),
        File(asepriteRoot, "docs/LICENSES.md"))
}

androidComponents {
    onVariants { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(
            packAsepriteData, PackAsepriteDataTask::outputDir)
    }
}
