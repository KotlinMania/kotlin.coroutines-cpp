# AST-distance enum receiver report repair

Date: 2026-10-07. This is a reproduced report defect, separate from the
CoroutineStart source algorithm replacement. A Kotlin enum instance method must
use a free function carrying the enum receiver in C++, because C++ enums cannot
contain member functions. The port uses this projection for CoroutineStart.

The isolated package-matched fixture in
`build/ir-recovery/enum-receiver-before` defines Mode.invoke and Mode.isLazy in
Kotlin and actual C++ invoke(Mode, int)/is_lazy(Mode) bodies. The preceding tool
records 0/1 function names and MISSING_SYMBOL for both the method and property
(`enum-receiver-before.log`). The corrected tool records 1/1 names and PRESENT
for both (`enum-receiver-after.log`). Both retain their actual scored signatures
and bodies. The fixture's 1/1 name presence is not full parameter/body parity.

The parser now records enum membership from the declaration's actual CST enum
keyword/modifier. The nearest class/object boundary matters: ordinary, nested
and anonymous-object methods are not enum methods. The callable matcher recognizes
an enum instance method only when its C++ namespace function has the matching
first parameter type and the expected user-argument count after that explicit
receiver. An existing suspend continuation carrier remains governed by its actual
source declaration. Deep name/owner presence also accepts the same enum receiver
for method/getter projection; it remains separate from overload and body parity.

Native extraction regressions verify actual enum identity and correct argument
projection, with negative checks for wrong receiver type, wrong arity, ordinary
class methods, nested classes and anonymous objects inside enum fields. The CLI
regression verifies actual method/getter presence and keeps wrong-enum method and
getter, and ordinary-class method, missing. All nine existing tool tests finish
with zero failures in 1.72 seconds (`build/ir-recovery/enum-receiver-tool-tests.log`).
The tool rebuild receipt is `enum-receiver-tool-build.log`.

Namespace/provenance constraints and body/signature scoring are not loosened.
This repair does not resolve the separate consumed Native namespace-projection
findings, and does not make unrelated class methods match namespace functions.
Both whole-root reports are refreshed after source and tool changes; their
measurements remain the oracle for the substantial remaining translation.
