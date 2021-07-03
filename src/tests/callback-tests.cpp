#ifdef TEST_CALLBACKS
#include "../../shared/utils/typedefs-wrappers.hpp"
EventCallback<> asdf;

void test() {
    std::cout << "sklasjdfgkladgf" << std::endl;
}

void test2() {
    std::cout << "idk" << std::endl;
}

int main() {
    asdf.addCallback(test);
    asdf.addCallback(test2);
    asdf.invoke();
    asdf.removeCallback(test);
    asdf.invoke();
}
#endif