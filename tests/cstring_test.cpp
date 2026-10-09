#include <sys/cstring.hpp>
#include <sys/type_traits.hpp>
#include <sys/cstdarg.hpp>
#include <cassert>

static_assert(sys::is_same_v<decltype(sys::strchr(static_cast<char*>(nullptr), 0)), char*>);
static_assert(sys::is_same_v<decltype(sys::strchr(static_cast<const char*>(nullptr), 0)), const char*>);
int sum_arguments(int count, ...) {
    sys::va_list arguments, copy;
    va_start(arguments, count);
    va_copy(copy, arguments);
    int sum = 0;
    for (int i = 0; i < count; ++i) sum += va_arg(copy, int);
    va_end(copy);
    va_end(arguments);
    return sum;
}
int main() {
    assert(sum_arguments(3, 1, 2, 3) == 6);
    const char text[] = "aba";
    char buffer[8] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
    assert(sys::strlen(text) == 3 && sys::strlen("") == 0);
    assert(sys::strchr(text, 'a') == text);
    assert(sys::strchr(text, 0) == text + 3);
    assert(!sys::strchr(text, 'z'));
    assert(sys::strrchr(text, 'a') == text + 2);
    assert(sys::strrchr(text, 0) == text + 3);
    assert(!sys::strrchr(text, 'z'));
    assert(sys::strstr(text, "ba") == text + 1);
    assert(sys::strstr(text, "") == text && !sys::strstr(text, "abab"));
    assert(sys::strncpy(buffer, "ab", 5) == buffer);
    assert(buffer[0] == 'a' && buffer[1] == 'b' && buffer[2] == 0 && buffer[4] == 0 && buffer[5] == 'x');
    sys::strncpy(buffer, "12345", 3);
    assert(buffer[0] == '1' && buffer[2] == '3' && buffer[3] == 0);
    assert(sys::strcpy(buffer, text) == buffer && sys::strcmp(buffer, text) == 0);
    assert(sys::strncmp("abc", "abd", 2) == 0 && sys::strncmp("abc", "abd", 3) < 0);
    assert(sys::strncmp("", "a", 0) == 0 && sys::strncmp("", "a", 1) < 0);
    const char high[] = {static_cast<char>(0xff), 0};
    assert(sys::strcmp(high, "a") > 0);
    unsigned char bytes[] = {0, 255, 42};
    assert(sys::memchr(bytes, 511, 3) == bytes + 1);
    assert(!sys::memchr(bytes, 42, 2) && !sys::memchr(bytes, 0, 0));
    assert(sys::strchr(buffer, 0) == buffer + 3);
    sys::memset(buffer, 0, sizeof(buffer));
    sys::memcpy(buffer, text, sizeof(text));
    sys::memmove(buffer + 1, buffer, 3);
    assert(sys::memcmp(buffer, "aaba", 4) == 0);
}
