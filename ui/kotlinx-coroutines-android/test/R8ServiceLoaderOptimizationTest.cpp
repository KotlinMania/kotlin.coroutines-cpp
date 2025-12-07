// Original: ui/kotlinx-coroutines-android/test/R8ServiceLoaderOptimizationTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Convert TestBase inheritance
// TODO: Implement File and System.getProperty API
// TODO: Implement DEX file parsing (DexFileFactory.loadDexFile)
// TODO: Map @Test and @Ignore annotations
// TODO: Convert Java streams API to C++ equivalent
// TODO: Implement class loader resource stream API

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import org.jf.dexlib2.*
// TODO: import org.junit.Test
// TODO: import java.io.*
// TODO: import java.util.stream.*
// TODO: import kotlin.test.*

class R8ServiceLoaderOptimizationTest : public TestBase {
private:
    // TODO: Implement File and DEX loading
    DexFile r8_dex_ = File(System::getProperty("dexPath")).asDexFile();
    DexFile r8_dex_no_optim_ = File(System::getProperty("noOptimDexPath")).asDexFile();

public:
    // TODO: @Test
    void testNoServiceLoaderCalls() {
        bool service_loader_invocations = std::any_of(
            r8_dex_.types.begin(),
            r8_dex_.types.end(),
            [](const auto& it) {
                return it.type == "Ljava/util/ServiceLoader;";
            }
        );
        assertEquals(
            false,
            service_loader_invocations,
            "References to the ServiceLoader class were found in the resulting DEX."
        );
    }

    // TODO: @Test
    void testAndroidDispatcherIsKept() {
        bool has_android_dispatcher = std::any_of(
            r8_dex_no_optim_.classes.begin(),
            r8_dex_no_optim_.classes.end(),
            [](const auto& it) {
                return it.type == "Lkotlinx/coroutines/android/AndroidDispatcherFactory;";
            }
        );

        assertEquals(true, has_android_dispatcher);
    }

    // TODO: @Test
    // TODO: @Ignore
    void testNoOptimRulesMatch() {
        std::vector<std::string> paths = {
            "META-INF/com.android.tools/proguard/coroutines.pro",
            "META-INF/proguard/coroutines.pro",
            "META-INF/com.android.tools/r8-upto-1.6.0/coroutines.pro"
        };

        std::map<std::string, std::set<std::string>> path_rule_sets;
        for (const auto& path : paths) {
            auto resource_stream = getClass().getClassLoader().getResourceAsStream(path);
            std::set<std::string> rule_set;
            // TODO: Convert Java streams filter/collect
            // bufferedReader().lines().filter { line ->
            //     line.isNotBlank() && !line.startsWith("#")
            // }.collect(Collectors.toSet())
            path_rule_sets[path] = rule_set;
        }

        // TODO: Convert sequence reduce
        // asSequence().reduce { acc, entry ->
        //     assertEquals(acc.value, entry.value, ...)
        //     entry
        // }
    }
};

DexFile as_dex_file(const File& file) {
    return DexFileFactory::loadDexFile(file, nullptr);
}

} // namespace android
} // namespace coroutines
} // namespace kotlinx
