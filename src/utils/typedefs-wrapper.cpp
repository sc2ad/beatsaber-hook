#include "../../shared/utils/typedefs-wrappers.hpp"

std::unordered_map<void*, size_t> Counter::addrRefCount;
std::shared_mutex Counter::mutex;