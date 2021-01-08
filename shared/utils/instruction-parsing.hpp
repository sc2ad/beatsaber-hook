#pragma once
#include <array>
#include <bitset>
#include <optional>
#include <stack>
#include <unordered_map>
#include "logging.hpp"
#include "utils.h"

class Register {
public:
    inline static const uint_fast8_t TOTAL_NUMBER = 32;
    inline static const uint_fast8_t RLINK = 30;
    inline static const uint_fast8_t RZR = 31;
    inline static const uint_fast8_t SP = 31;
    static_assert(TOTAL_NUMBER > SP);

    uint_fast8_t num;
    bool r31_is_sp;
    Register(uint_fast8_t reg, bool r31) : num(reg), r31_is_sp(r31) {};
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Register& n);
};

class Instruction {
public:
    Instruction(const int32_t* inst);

    // ~~~ PUBLIC FIELDS ~~~
    // TODO: puzzle out and support 32-bit views of registers
    const int32_t* addr;  // the pointer to the instruction
    // Rd and Rs are capitalized in accordance with typical Register notation
    int_fast8_t Rd = -2;  // the destination register's number, or -1 if none
    int_fast8_t Rd2 = -1;  // the 2nd destination register's number, or -1 if none
    int_fast8_t numSourceRegisters = -1;  // the number of source registers this instruction has
    // TODO: divorce? Sometimes the source registers are for the label, not the destination
    union {
        int_fast8_t Rs[3] = { -1, -1, -1 };  // the number(s) of the source register(s), e.g. {12, 29} for X12 & X29
        int64_t result;  // iff numSourceRegisters is 0, the value that will be stored to Rd by this instruction
    };
    std::optional<int64_t> imm;  // the immediate, if applicable
    std::optional<const int32_t*> label;  // if present, a pointer to assembly to be calculated or jumped to

    // see https://developer.arm.com/docs/ddi0596/a/a64-shared-pseudocode-functions/aarch64-instrs-pseudocode#impl-aarch64.ShiftReg.3
    enum ShiftType {  // in most cases (exceptiing noShift) the amount is stored in imm and the shift is applied to Rm (Rs[1])
        LSL, LSR, ASR, ROR, noShift
    } shiftType = noShift;

    // see https://developer.arm.com/docs/ddi0596/a/a64-shared-pseudocode-functions/aarch64-instrs-pseudocode#impl-aarch64.ExtendReg.3
    enum ExtendType {  // in most cases (exceptiing noExtend) the amount is stored in imm and the extend is applied to Rm (Rs[1])
        UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX, noExtend
    } extendType = noExtend;
    static bool extendIsUnsigned(ExtendType extend) {
        return extend <= UXTX;
    }
    static uint_fast8_t extendGetLen(ExtendType extend) {
        if (extend == noExtend) return 0;
        return 8 << (extend % 4);
    }

    #define BRANCH_ENUM(DO) \
        DO(NOBRANCH,  "") \
        DO(DIRCALL,   "Direct Branch with link") \
        DO(INDCALL,   "Indirect Branch with link") \
        DO(ERET,      "Exception return (indirect)") \
        DO(DBGEXIT,   "Exit from Debug state") \
        DO(RET,       "Indirect branch with function return hint") \
        DO(DIR,       "Direct branch") \
        DO(INDIR,     "Indirect branch") \
        DO(EXCEPTION, "Exception entry") \
        DO(RESET,     "Reset") \
        DO(UNKNOWN,   "Other")

    enum BranchType {
        #define MAKE_ENUM(VAR, STR) VAR,  // if only we could add STR as a comment (for IDE purposes only)
        BRANCH_ENUM(MAKE_ENUM)
        #undef MAKE_ENUM
    } branchType = NOBRANCH;

    inline static const char* const branchTypeNames[] = {
        #define MAKE_NAMES(VAR, STR) #VAR,
        BRANCH_ENUM(MAKE_NAMES)
        #undef MAKE_NAMES
    };

    inline static const char* const branchTypeInfo[] = {
        #define MAKE_STRINGS(VAR, STR) STR,
        BRANCH_ENUM(MAKE_STRINGS)
        #undef MAKE_STRINGS
    };
    #undef BRANCH_ENUM

    bool parsed;  // whether the instruction was fully and successfully parsed
    bool valid = true;  // iff parsed, whether the instruction is a valid one
    // For LDR (literal), dictates the size of the result in bytes.
    uint8_t size;

    // ~~~ METHODS ~~~
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Instruction& inst);

    // e.g. ADR, ADRP
    bool isPcRelAdr();

    // is the Instruction an add or subtract immediate? (e.g. one source reg +- an immediate value)
    bool isAddOrSubImm();

    bool isLoadOrStore();
    bool isLoad();
    bool isStore();
    bool isHint();
    bool isNOP();

    // Does the instruction have an Immediate that might be an offset that applies to register "reg"?
    bool hasImmOffsetOnReg(uint_fast8_t reg);

    // pred should be a https://en.cppreference.com/w/cpp/named_req/FunctionObject that accepts an Instruction* and returns bool.
    // Returns the nth match of the predicate starting at this->addr, or nullptr if the nth match does not exist.
    // Use n=1 for the 1st match, not 0.
    // Stops search when a "ret" instruction (return) is encountered (unless the nth match is found first or "ret" matches pred).
    //   Will also disregard up to "rets" ret instructions that do not match pred, or an infinite number if rets is negative.
    // The search does not follow jumps.
    template<class UnaryPredicate>
    Instruction* findNth(int n, UnaryPredicate pred, int rets = 0) {
        auto inst = this;
        decltype(n) matches = 0;
        while (true) {
            if (pred(inst)) {
                matches++;
                if (matches == n) return inst;
            } else if ((rets >= 0) && inst->isReturn()) {
                if (rets == 0) {
                    Logger::get().debug("Breaking on offset %lX", ((intptr_t)inst->addr) - getRealOffset(0));
                    break;
                }
                rets--;
            }
            auto pc = inst->addr;
            if (inst != this) delete inst;
            inst = new Instruction(pc + 1);
        }
        Logger::get().error("Only found %i instructions matching this predicate!", matches);
        usleep(10000);
        return nullptr;
    }
    // e.g. BL(R). Unless the jump is indirect, the address which the instruction jumps to will be at ->imm.
    Instruction* findNthCall(int n, int rets = 0);
    // e.g. B(.eq/.ne)
    Instruction* findNthDirectBranchWithoutLink(int n, int rets = 0);
    // e.g. ADR(P)
    Instruction* findNthPcRelAdr(int n, int rets = 0);
    // Finds the nth (Load/store with an Immediate offset) or (Add/subtract immediate) that applies to register "reg".
    Instruction* findNthImmOffsetOnReg(int n, uint_fast8_t reg, int rets = 0);

    inline bool isBranch() const {
        return branchType != NOBRANCH;
    }
    inline bool isUnconditionalBranch() const {
        return isBranch() && cond == -1;
    }
    inline bool isReturn() const {
        return branchType == RET;
    }
    // B(L/.cond)/TB(N)Z (i.e. location to jump to is not read from a register/memory, but encoded directly into the instruction)
    inline bool isDirectBranch() const {
        return (branchType == DIRCALL) || (branchType == DIR);
    }
    // B(L)R
    inline bool isIndirectBranch() const {
        return (branchType != NOBRANCH) && (!isDirectBranch());
    }
    // BL(R)
    inline bool isCall() const {
        return (branchType == DIRCALL) || (branchType == INDCALL);
    }

private:
    const char* kind[3];  // strings describing the kind of instruction, from least to most specific
    uint8_t parseLevel;  // The lowest level we were able to parse at, 1-3 (subtract 1 for index of most specific string in 'kind')
    bool RdCanBeSP = false;
    bool Rs0CanBeSP = false;
    // For LDR/STR:
    bool wback;
    bool postindex;
    int_fast8_t cond = -1;  // if set, a 4-bit int. See https://developer.arm.com/docs/ddi0596/a/a64-shared-pseudocode-functions/shared-functions-pseudocode#impl-shared.ConditionHolds.1
};

struct RegisterSet {
    int_fast64_t regs[Register::TOTAL_NUMBER] = {0};
};

struct PSTATE {
    // https://developer.arm.com/docs/ddi0595/e/aarch64-system-registers/nzcv
    // https://developer.arm.com/docs/den0024/latest/the-a64-instruction-set/data-processing-instructions/conditional-instructions
    bool N;  // 1 == result was negative
    bool Z;  // 1 == result was zero
    bool C;  // 1 == result had unsigned overflow (carry)
    bool V;  // 1 == result had signed overflow
    bool D, A, I, F;
    bool SS, IL, nRW, SP;
    int_fast8_t EL;  // 0, 1, 2, or 3
};

struct ProgramState {
    RegisterSet regSet;
    PSTATE pstate;
};

struct ParseState;

class InstructionTree : Instruction {
  public:
    InstructionTree *noBranch;
    InstructionTree *branch;

    InstructionTree(const int32_t* inst);
    // Note: this must be done in a stack frontier fashion to prevent StackOverflow
    void PopulateChildren(ParseState& parseState);
    ~InstructionTree() {
        if (noBranch) delete noBranch;
        if (branch) delete branch;
    }
};

struct ParseState {
    // Note: this uses twice the memory that a 2D bitset would, but is far easier/faster to edit
    std::array<std::bitset<Register::TOTAL_NUMBER>, Register::TOTAL_NUMBER> dependencyMap;
    std::unordered_map<const int32_t*, InstructionTree*> codeToInstTree;  // the set of all instructions and their corresponding trees
    std::unordered_map<const int32_t*, decltype(dependencyMap)> functionCandidates;  // the set of instructions that were jumped to
    std::unordered_map<const int32_t*, decltype(dependencyMap)> otherJumps;
    std::stack<std::pair<InstructionTree*, decltype(dependencyMap)>> frontier;  // points that still need ProcessRegisterDependencies run on them

    ParseState() {
        for (uint_fast8_t i = 0; i < dependencyMap.size(); i++) {
            dependencyMap[i] = decltype(dependencyMap)::value_type(1 << i);
        }
    }
};

class AssemblyFunction {
public:
    InstructionTree* instructions;

    AssemblyFunction(const int32_t* inst);
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const AssemblyFunction& func);
private:
    ParseState parseState;
};

#define ONES(N) ((1ull << (N)) - 1ull)

// Truncates the given integer to its least significant N bits.
template<class T>
T trunc(T bits, int N) {
    if (N == sizeof(T)*CHAR_BIT) return bits;
    CRASH_UNLESS(N < (long)sizeof(T)*CHAR_BIT);
    return bits & ONES(N);
}

// Transforms the given integer (with M denoting the true number of significant bits) into an unsigned number of type To.
template<class To, class From>
To ZeroExtend(From bits, [[maybe_unused]] int M) {
    static_assert(std::is_unsigned_v<From>);
    return static_cast<To>(bits);
}

// Transforms the given integer (with M denoting the true number of significant bits) into a properly signed number of type To.
template<class To, class From>
To SignExtend(From bits, int M) {
    static_assert(std::is_signed_v<To>);
    constexpr int N = sizeof(To) * CHAR_BIT;
    assert(N >= M);
    To prep = ((To)bits) << (N - M);
    return (prep >> (N - M));
}

// Replicates the given bits (with M denoting the true number of significant bits) until they fill N bits.
template<class To, class From>
To Replicate(From bits, int M, int N = sizeof(To) * CHAR_BIT) {
    CRASH_UNLESS(N % M == 0);
    To replicatedBits = bits;
    auto repSize = M;
    while (repSize * 2 <= N) {
        replicatedBits |= (replicatedBits << repSize);
        repSize *= 2;
    }
    // The overlap should all match since N % M == 0 means that N - M*a % M == 0
    replicatedBits |= (replicatedBits << (N - repSize));
    if (N != M) {
        Logger::get().debug("Replicate %s * %i = %s", std::bitset<sizeof(From)*CHAR_BIT>(bits).to_string().c_str(), N / M,
            std::bitset<sizeof(To)*CHAR_BIT>(replicatedBits).to_string().c_str());
    }
    return replicatedBits;
}

// N is the true number of significant bits in x.
// Returns the index of the most significant ON bit in x.
template<class T>
int HighestSetBit(T x, int N) {
    for (int i = N - 1; i >= 0; i--) {
        if (x & (1 << i)) return i;
    }
    return -1;
}

// For all shifts (LSL, LSR, ASR, ROR): N is the true number of significant bits in x.
// Left shift
template<class T>
T LSL(T x, int N, unsigned shift) {
    return trunc(x << shift, N);
}

// Right shift, taking x as unsigned.
template<class T>
T LSR(T x, int N, unsigned shift) {
    return trunc(x >> shift, N - shift);
}

// Right shift, taking x as signed.
template<class T>
T ASR(T x, int N, unsigned shift) {
    typedef typename std::make_signed<T>::type signedT;
    return trunc(SignExtend<signedT>(x, N) >> shift, N);
}

// Right shift, but bits that "fall off" move to the front instead
template<class T>
T ROR(T x, int N, unsigned shift) {
    shift %= N;
    T ret = LSR(x, N, shift) | LSL(x, N, N - shift);
    if ((ret == 0) && (x != 0)) {
        Logger::get().debug("%s ROR %i (-%i) returned %s!", std::bitset<sizeof(T)*CHAR_BIT>(x).to_string().c_str(), shift, N - shift,
            std::bitset<sizeof(T)*CHAR_BIT>(ret).to_string().c_str());
        SAFE_ABORT();
    }
    return ret;
}

// Returns the value of the bits in x at index high through low inclusive, where the LSB is index 0 and the MSB's index >= high.
template<class T>
T bits(T x, int high, int low) {
    return trunc(x >> low, high - low + 1);
}

// Uses instWithResultAdr's result and instWithImmOffset's imm to calculate a jump address
decltype(Instruction::result) ExtractAddress(Instruction* instWithResultAdr, Instruction* instWithImmOffset);

// Starting at addr, finds the pcRelNth adr(p), then the offsetNth instruction after that with an immediate offset
// which applies to the adr(p)'s destination register, and finally calls the ExtractAddress with 2 Instruction*'s
decltype(Instruction::result) ExtractAddress(const int32_t* addr, int pcRelN, int offsetN);

// Wrapper for const int32_t* addr version that converts from a function pointer for you
template<typename TRet, typename ...TFuncArgs, typename ...TArgs>
decltype(Instruction::result) ExtractAddress(function_ptr_t<TRet, TFuncArgs...> func, TArgs... args) {
    return ExtractAddress(reinterpret_cast<const int32_t*>(func), args...);
}

// Makes Instructions from the idxOfInstWithResultAdr-th and idxOfInstWithImmOffset-th instructions after inst,
// then passes the Instruction*'s to ExtractAddress
decltype(Instruction::result) ExtractAddressFixed(const int32_t* inst, int idxOfInstWithResultAdr, int idxOfInstWithImmOffset);

// Wrapper for const int32_t* inst version that converts from a function pointer for you
template<typename TRet, typename ...TFuncArgs, typename ...TArgs>
decltype(Instruction::result) ExtractAddressFixed(function_ptr_t<TRet, TFuncArgs...> func, TArgs... args) {
    return ExtractAddressFixed(reinterpret_cast<const int32_t*>(func), args...);
}

// Reads a switchTable to find the Instruction to jump to for the given case: value.
Instruction* EvalSwitch(const uint32_t* switchTable, int switchCaseValue);

// Does ExtractAddress(inst, pcRelN, offsetN) for you to get const uint32_t* switchTable
Instruction* EvalSwitch(const int32_t* inst, int pcRelN, int offsetN, int switchCaseValue);

// Wrapper for const int32_t* inst version that converts from a function pointer for you
template<typename TRet, typename ...TFuncArgs, typename ...TArgs>
Instruction* EvalSwitch(function_ptr_t<TRet, TFuncArgs...> func, TArgs... args) {
    return EvalSwitch(reinterpret_cast<const int32_t*>(func), args...);
}
