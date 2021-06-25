#include "../../shared/utils/typedefs-wrappers.hpp"

std::unordered_map<void*, size_t> Counter::addrRefCount;
std::shared_mutex Counter::mutex;

#ifdef TEST_SAFEPTR
void testRef(SafePtr<int>& ref) {
    *ref = 55;
}
void testCopy(SafePtr<int> copy) {
    *copy = 1234;
}
void testLiteral(int* value) {
    *value = 5678;
}
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
        testRef(b);
        // Count should be unchanged
        assert(Counter::get(&x) == 1);
        assert(*b == 55);
        testCopy(b);
        // Value is changed since a copied safe ptr still points to the same location
        assert(*b == 1234);
        // Count is increased and then decreased for the copy
        assert(Counter::get(&x) == 1);
        // Should NOT BE USED! This will eventually warn for deprecation.
        testLiteral((int*)b);
        // Literal still points to the same underlying memory.
        assert(*b == 5678);
        // Final test to propagate out
        *b = 12;
        assert(*b == 12);
        assert(Counter::get(&x) == 1);
    }
    // Instance is dead, but not without having set x to 12.
    assert(x == 12);
    assert(Counter::get(&x) == 0);
    // NOTE THAT ANY int& or int* has a lifetime limited to the instance of the SafePtr it is from.
    // This means that any reference from *SafePtr<T> or (T*)SafePtr<T> has a limited lifetime and SHOULD NOT BE USED!
    // Instead, consider using -> explicitly, or passing SafePtr<T> instances either by reference (strongly suggested) or by value/move.
}

#include "../../shared/utils/il2cpp-utils-classes.hpp"
#include "il2cpp-object-internals.h"
void test_cast() {
    int x = 3;
    {
        SafePtr<int> a(&x);
        // We should then be able to cast to a SafePtr<float>, but in this case this will fail because we don't handle value type casts
        auto b = a.cast<float>();
        auto c = *a.try_cast<float>();
    }
    // If we have reference types, however, we can do that instead.
    {
        Il2CppObject inst{};
        SafePtr<Il2CppObject> a(&inst);
        // We could then cast this to whatever we wish, including an Il2CppReflectionType
        // (though the validity is obviously a failure with a null class)
        auto b = a.cast<Il2CppReflectionType>();
        auto c = *a.try_cast<Il2CppReflectionType>();

        // Just to make sure:
        auto* q = il2cpp_utils::cast<Il2CppReflectionType>(&inst);
        auto* p = *il2cpp_utils::try_cast<Il2CppObject>(q);
        assert(q == p && q == &inst);
        if (p != &inst) {
            // FAILSAFE TO AVOID UNUSUED p
            SAFE_ABORT();
        }
    }
}
#endif
