#include "../../shared/utils/typedefs-wrappers.hpp"

std::unordered_map<void*, size_t> Counter::addrRefCount;
std::shared_mutex Counter::mutex;

#ifdef TEST_SAFEPTR
void test() {
    int x = 3;
    SafePtr<int> a;
    {
        SafePtr<int> b(&x);
        assert(Counter::get(&x) == 1);
        {
            CountPointer<int> cptr(&x);
            assert(cptr.count() == 2);
        }
        assert(Counter::get(&x) == 1);
        // Should be valid even though we destroyed a CountPointer.
        *b = 32;
        {
            // Create a temporary copy
            SafePtr<int> c(b);
            assert(Counter::get(&x) == 2);
            assert(*c == 32);
        }
        assert(Counter::get(&x) == 1);
        // Should be valid even though we destroyed a SafePtr.
        *b = 12;
        assert(*b == 12);
    }
    // Instance is dead, but not without having set x to 12.
    assert(x == 12);
    assert(Counter::get(&x) == 0);
}
#endif
