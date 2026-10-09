# zstl

Minimal C++ STL replacement under `namespace sys`, suitable for freestanding /
OS-kernel use where the system `<vector>`, `<string>`, `<unordered_map>` etc.
are not available.

This library is header-only. It is consumed by [mini-cocos](https://github.com/leafvmaple/mini-cocos)
through the `mstd` switcher header (`src/base/ZCStd.h`).

## Scope

Implemented (minimal, just enough for mini-cocos):

- `sys/cstddef.hpp`     - `size_t`, `ptrdiff_t`, `nullptr_t`
- `sys/cstdint.hpp`     - fixed-width integer typedefs
- `sys/type_traits.hpp` - core type traits
- `sys/utility.hpp`     - `move`, `forward`, `swap`, `exchange`, `pair`
- `sys/new.hpp`         - placement new, `nothrow_t`, `nothrow`
- `sys/initializer_list.hpp` - compiler-magic forwarder
- `sys/iterator.hpp`    - `begin`, `end`, `distance`
- `sys/limits.hpp`      - `numeric_limits<T>` (integers + float)
- `sys/algorithm.hpp`   - `min`, `max`, `clamp`, `sort`, `stable_sort`, `find`, `find_if`, `remove_if`, `for_each`, ...
- `sys/functional.hpp`  - `function<R(Args...)>`, `hash`, `less`, `equal_to`
- `sys/memory.hpp`      - `unique_ptr`, `make_unique`, `default_delete`
- `sys/array.hpp`       - `array<T, N>`
- `sys/vector.hpp`      - `vector<T>`
- `sys/string.hpp`      - `string` (and basic `wstring`)
- `sys/unordered_map.hpp` - chaining hash map
- `sys/set.hpp`         - small sorted-vector backed `set`
- `sys/string_conv.hpp` - `to_string(int)` and friends

Not implemented (intentionally):

- Exceptions: this STL is freestanding-friendly; failure modes are fail-fast.
- Streams (`iostream` / `fstream`).
- `<filesystem>`, `<system_error>`, `<chrono>`.

## Convention

- Namespace: `sys` (not `std`).
- Headers under `include/sys/*.hpp`, no extension collision with C headers.
- No exceptions are thrown; bad allocation triggers a trap (`__builtin_trap`
  / abort).

## Freestanding integration

Use the same `include` root and `ZSTL_FREESTANDING` definition for every consumer
in one program. `sys/new.hpp` provides placement forms and allocation declarations;
the embedding runtime implements ordinary and `sys::nothrow` allocation/deallocation.
Ordinary allocation must fail fast on exhaustion; the nothrow form may return null.
The size/integer type headers and freestanding initializer-list ABI do not depend
on system headers. Hosted mode continues to use the host `<new>` and
`<initializer_list>` adapters. Other hosted wrapper headers still require their
corresponding runtime integration when used without the system include paths.

`unique_ptr` preserves stateful deleters during moves and constrained conversions.
Stateless non-final deleters use C++17 empty-base optimization. Single objects
and unbounded arrays use their respective deletion forms; borrowing does not extend
lifetime, and the `[[nodiscard]]` release operation transfers cleanup responsibility.
This remains a minimal implementation: custom pointer typedefs and reference-type
deleters are not supplied.

Optional tests: configure with `-DZSTL_BUILD_TESTS=ON`, build and run CTest. The
freestanding smoke target compiles memory, integer, array and vector/initializer-list
headers with `-nostdinc -nostdinc++ -fno-exceptions -fno-rtti`.
