#include "../../shared/utils/il2cpp-utils-exceptions.hpp"
#include "../../shared/utils/il2cpp-functions.hpp"

namespace il2cpp_utils {
    // Init all of the usable il2cpp API, if it has yet to be initialized
    // Maximum length of characters of an exception message - 1
    #ifndef EXCEPTION_MESSAGE_SIE
    #define EXCEPTION_MESSAGE_SIZE 4096
    #endif
    // Returns a legible string from an Il2CppException*
    std::string ExceptionToString(Il2CppException* exp) noexcept {
        il2cpp_functions::Init();

        char msg[EXCEPTION_MESSAGE_SIZE];
        il2cpp_functions::format_exception(exp, msg, EXCEPTION_MESSAGE_SIZE);
        // auto exception_message = csstrtostr(exp->message);
        // return to_utf8(exception_message);
        return msg;
    }
}