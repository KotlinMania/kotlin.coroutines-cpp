import os

roots = [
    "kotlinx-coroutines-core/common/src",
    "kotlinx-coroutines-core/concurrent/src",
    "kotlinx-coroutines-core/native/src"
]

files_to_add = []

for root in roots:
    for dirpath, dirnames, filenames in os.walk(root):
        for f in filenames:
            if f.endswith(".cpp"):
                path = os.path.join(dirpath, f)
                # Remove "kotlinx-coroutines-core/" prefix for CMake relative path
                rel_path = path.replace("kotlinx-coroutines-core/", "", 1)
                files_to_add.append(rel_path)

files_to_add.sort()

print("set(CORE_SOURCES")
for f in files_to_add:
    print(f"    {f}")
print(")")
