// NOTE(port): Ordinary lambdas and local classes cannot inherit suspension
// permission from an enclosing suspend function.
[[suspend]] void* source();
[[suspend]] void* outer() {
    auto callable = [] { return source(); };
    struct Local {
        void* value = source();
        void* method() { return source(); }
    };
    return nullptr;
}
