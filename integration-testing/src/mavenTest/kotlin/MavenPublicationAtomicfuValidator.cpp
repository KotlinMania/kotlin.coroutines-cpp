// Transliterated from: integration-testing/src/mavenTest/kotlin/MavenPublicationAtomicfuValidator.kt

// TODO: #include equivalent
// import org.junit.Test
// import org.objectweb.asm.*
// import org.objectweb.asm.ClassReader.*
// import org.objectweb.asm.ClassWriter.*
// import org.objectweb.asm.Opcodes.*
// import java.util.jar.*
// import kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace validator {

class MavenPublicationAtomicfuValidator {
private:
    const std::vector<uint8_t> ATOMIC_FU_REF = {'L', 'k', 'o', 't', 'l', 'i', 'n', 'x', '/', 'a', 't', 'o', 'm', 'i', 'c', 'f', 'u', '/'};
    const std::string KOTLIN_METADATA_DESC = "Lkotlin/Metadata;";

public:
    // @Test
    void test_no_atomicfu_in_classpath() {
        auto result = run_catching([]() {
            return Class::for_name("kotlinx.atomicfu.AtomicInt");
        });
        assert_true(result.exception_or_null() instanceof ClassNotFoundException);
    }

    // @Test
    void test_no_atomicfu_in_mpp_jar() {
        auto clazz = Class::for_name("kotlinx.coroutines.Job");
        JarFile(clazz.get_protection_domain().get_code_source().get_location().get_file()).check_for_atomic_fu();
    }

    // @Test
    void test_no_atomicfu_in_android_jar() {
        auto clazz = Class::for_name("kotlinx.coroutines.android.HandlerDispatcher");
        JarFile(clazz.get_protection_domain().get_code_source().get_location().get_file()).check_for_atomic_fu();
    }

private:
    void check_for_atomic_fu(JarFile& jar_file) {
        std::vector<std::string> found_classes;
        for (const auto& e : jar_file.entries()) {
            if (!e.get_name().ends_with(".class")) continue;
            auto bytes = jar_file.get_input_stream(e).use([](auto& stream) {
                return stream.read_bytes();
            });
            // The atomicfu compiler plugin does not remove atomic properties from metadata,
            // so for now we check that there are no ATOMIC_FU_REF left in the class bytecode excluding metadata.
            // This may be reverted after the fix in the compiler plugin transformer (for Kotlin 1.8.0).
            auto out_bytes = erase_metadata(bytes);
            if (check_bytes(out_bytes)) {
                found_classes.push_back(e.get_name()); // report error at the end with all class names
            }
        }
        if (!found_classes.empty()) {
            std::string error_msg = "Found references to atomicfu in jar file " + jar_file.get_name() +
                " in the following class files:";
            for (const auto& cls : found_classes) {
                error_msg += "\n\t\t" + cls;
            }
            throw std::runtime_error(error_msg);
        }
        jar_file.close();
    }

    bool check_bytes(const std::vector<uint8_t>& bytes) {
        for (size_t i = 0; i < bytes.size() - ATOMIC_FU_REF.size(); ++i) {
            bool match = true;
            for (size_t j = 0; j < ATOMIC_FU_REF.size(); ++j) {
                if (bytes[i + j] != ATOMIC_FU_REF[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        return false;
    }

    std::vector<uint8_t> erase_metadata(const std::vector<uint8_t>& bytes) {
        ClassWriter cw(COMPUTE_MAXS | COMPUTE_FRAMES);
        ClassReader(bytes).accept(
            // TODO: Create custom ClassVisitor
            // object : ClassVisitor(ASM9, cw)
            SKIP_FRAMES
        );
        return cw.to_byte_array();
    }
};

} // namespace validator
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement JAR file reading
// 2. Implement ASM ClassReader/ClassWriter integration
// 3. Implement bytecode scanning
// 4. Implement metadata erasure via ClassVisitor
// 5. Implement runCatching exception handling
// 6. Implement Class::forName reflection
// 7. Set up proper test framework
// 8. Handle label-based loop control (loop@)
