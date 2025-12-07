// Original file: kotlinx-coroutines-debug/test/DebugTestBase.kt
// TODO: Convert imports to C++ includes
// TODO: Implement TestBase base class
// TODO: Convert @JvmField, @Rule, @Before, @After annotations
// TODO: Implement CoroutinesTimeout rule
// TODO: Implement DebugProbes API

namespace kotlinx {
namespace coroutines {
namespace debug {

class DebugTestBase : public TestBase {
public:
    // @JvmField
    // @Rule
    // TODO: val timeout = CoroutinesTimeout.seconds(60)

    // @Before
    virtual void set_up() {
        // TODO: DebugProbes.sanitizeStackTraces = false
        // TODO: DebugProbes.enableCreationStackTraces = false
        // TODO: DebugProbes.install()
    }

    // @After
    void tear_down() {
        // TODO: DebugProbes.uninstall()
    }
};

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
