// Transliterated from: integration-testing/src/jvmCoreTest/kotlin/ListAllCoroutineThrowableSubclassesTest.kt

// TODO: #include equivalent
// import com.google.common.reflect.*
// import kotlinx.coroutines.*
// import org.junit.Test
// import java.io.Serializable
// import java.lang.reflect.Modifier
// import kotlin.test.*

#include <set>

namespace kotlinx {
namespace coroutines {

class ListAllCoroutineThrowableSubclassesTest {
private:
    /*
     * These are all the known throwables in kotlinx.coroutines.
     * If you add one, this test will fail to make
     * you ensure your exception type is java.io.Serializable.
     *
     * We do not have means to check it automatically, so checks are delegated to humans.
     *
     * See #3328 for serialization rationale.
     */
    const std::set<std::string> KNOWN_THROWABLES = {
        "kotlinx.coroutines.TimeoutCancellationException",
        "kotlinx.coroutines.JobCancellationException",
        "kotlinx.coroutines.internal.UndeliveredElementException",
        "kotlinx.coroutines.CompletionHandlerException",
        "kotlinx.coroutines.internal.DiagnosticCoroutineContextException",
        "kotlinx.coroutines.internal.ExceptionSuccessfullyProcessed",
        "kotlinx.coroutines.CoroutinesInternalError",
        "kotlinx.coroutines.DispatchException",
        "kotlinx.coroutines.channels.ClosedSendChannelException",
        "kotlinx.coroutines.channels.ClosedReceiveChannelException",
        "kotlinx.coroutines.flow.internal.ChildCancelledException",
        "kotlinx.coroutines.flow.internal.AbortFlowException",
        "kotlinx.coroutines.debug.junit5.CoroutinesTimeoutException",
    };

public:
    // @Test
    void test_throwable_subclasses_are_serializable() {
        auto classes = ClassPath::from(this->get_java_class().get_class_loader())
            .get_top_level_classes_recursive("kotlinx.coroutines")
            // Not in the classpath: requires explicit dependency
            .filter([](const auto& it) {
                return it.name() != "kotlinx.coroutines.debug.CoroutinesBlockHoundIntegration"
                    && it.name() != "kotlinx.coroutines.debug.junit5.CoroutinesTimeoutExtension";
            });
        auto throwables = classes.filter([](const auto& it) {
            return Throwable::class_java().is_assignable_from(it.load());
        }).map([](const auto& it) {
            return it.to_string();
        });
        for (const auto& throwable : throwables) {
            for (const auto& field : throwable.get_java_class().get_declared_fields()) {
                if (Modifier::is_static(field.get_modifiers())) continue;
                auto type = field.get_type();
                assert_true(type.is_primitive() || Serializable::class_java().is_assignable_from(type),
                    "Throwable " + throwable + " has non-serializable field " + field);
            }
        }
        assert_equals(KNOWN_THROWABLES.sorted(), throwables.sorted());
    }
};

} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement reflection/classpath scanning mechanism
// 2. Implement ClassPath::from and getTopLevelClassesRecursive
// 3. Implement filter, map operations on class collections
// 4. Implement Throwable hierarchy checking
// 5. Implement Modifier::isStatic
// 6. Implement Serializable checking
// 7. Set up proper test assertions
// 8. Handle sorted() comparison
