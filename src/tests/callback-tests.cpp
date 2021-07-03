#ifdef TEST_CALLBACKS
#include "../../shared/utils/typedefs-wrappers.hpp"
EventCallback<> asdf;

static void test() {
    std::cout << "sklasjdfgkladgf" << std::endl;
}

static void test2() {
    std::cout << "idk" << std::endl;
}

static void randomRunner() {
    asdf.addCallback(test);
    asdf.addCallback(test2);
    asdf.invoke();
    asdf.removeCallback(test);
    asdf.invoke();
}
#endif