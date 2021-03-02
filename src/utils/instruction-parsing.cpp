#include "../../shared/utils/instruction-parsing.hpp"
#include "../../shared/utils/utils.h"
#include <functional>
#include <new>
#include <sstream>
#include "../../shared/utils/logging.hpp"

static const char* unalloc = "UNALLOCATED";
static const char* pcRelAddr = "PC-rel. addressing";
static const char* ldSt = "Loads and Stores";
static const char* addSubImm = "Add/subtract (immediate)";
static const char* nopSt = "NOP";
static const char* hintSt = "HINT";

// https://developer.arm.com/docs/ddi0596/a/a64-shared-pseudocode-functions/aarch64-instrs-pseudocode#impl-aarch64.DecodeBitMasks
// Explanation at https://dinfuehr.github.io/blog/encoding-of-immediate-values-on-aarch64/
std::optional<std::pair<uint64_t, uint64_t>> DecodeBitMasks(unsigned N, unsigned imms, unsigned immr, unsigned regSize, bool logical) {
    auto len = HighestSetBit((N << 6) | trunc(~imms, 6), 7);
    if (len < 1) return std::nullopt;

    unsigned size = 1 << len;
    assert(regSize >= size);
    unsigned levels = size - 1;  // a real bitmask of the rightmost "size" bits
    unsigned R = immr & levels;
    unsigned S = imms & levels;
    // For logical immediates an all-ones value of S is reserved since it would generate a useless all-ones result
    if (logical && (S == levels)) return std::nullopt;

    auto diff = S - R;  // "6-bit subtract with borrow"
    unsigned d = trunc(diff, len);

    uint64_t welem = ONES(S + 1);  // # of sig bits = size
    uint64_t telem = ONES(d + 1);  // # of sig bits = size
    welem = ROR(welem, size, R);
    // Replicate the patterns to fill regSize
    auto wmask = Replicate<uint64_t>(welem, size, regSize);
    auto tmask = Replicate<uint64_t>(telem, size, regSize);
    return std::pair{wmask, tmask};
}

decltype(Instruction::result) ExtractAddress(Instruction* instWithResultAdr, Instruction* instWithImmOffset) {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("ExtractAddress");
    RET_0_UNLESS(logger, instWithImmOffset->imm);
    auto jmpOff = instWithResultAdr->result;
    auto offset = *(instWithImmOffset->imm);

    auto jmp = jmpOff + offset;
    logger.debug("offset: %lX, jmp: %lX (offset %lX)", offset, jmp, asOffset(jmp));
    return jmp;
}

decltype(Instruction::result) ExtractAddress(const int32_t* addr, int pcRelN, int offsetN) {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("ExtractAddress");
    Instruction funcInst(addr);
    auto instAdrp = funcInst.findNthPcRelAdr(pcRelN);
    RET_0_UNLESS(logger, instAdrp);
    auto instOff = instAdrp->findNthImmOffsetOnReg(offsetN, instAdrp->Rd);
    RET_0_UNLESS(logger, instOff);
    logger.debug("adrp idx: %lu, offset instruction idx: %lu", instAdrp->addr - funcInst.addr, instOff->addr - funcInst.addr);
    logger.debug("instAdrp: %s", instAdrp->toString().c_str());
    logger.debug("instOff:  %s", instOff->toString().c_str());
    return ExtractAddress(instAdrp, instOff);
}

decltype(Instruction::result) ExtractAddressFixed(const int32_t* inst, int idxOfInstWithResultAdr, int idxOfInstWithImmOffset) {
    auto instWithResultAdr = Instruction(&inst[idxOfInstWithResultAdr]);
    auto instWithImmOffset = Instruction(&inst[idxOfInstWithImmOffset]);
    return ExtractAddress(&instWithResultAdr, &instWithImmOffset);
}

Instruction* EvalSwitch(const uint32_t* switchTable, int switchCaseValue) {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("EvalSwitch");
    auto stOffset = SignExtend<int64_t>(switchTable[switchCaseValue - 1], 32);
    auto jmpAddr = (int64_t)switchTable + stOffset;
    logger.debug("jmp offset from switch table: %lX (-%lX); jmp: %lX (offset %lX)",
        stOffset, -stOffset, jmpAddr, asOffset(jmpAddr));
    return new Instruction((const int32_t*)jmpAddr);
}

Instruction* EvalSwitch(const int32_t* inst, int pcRelN, int offsetN, int switchCaseValue) {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("EvalSwitch");
    auto switchTable = (const uint32_t*)ExtractAddress(inst, pcRelN, offsetN);
    RET_0_UNLESS(logger, switchTable);
    return EvalSwitch(switchTable, switchCaseValue);
}

static const auto &SP = Register::SP;
static const auto &RZR = Register::RZR;
static const auto &RLINK = Register::RLINK;
std::ostream& operator<<(std::ostream& os, const Register& regName) {
    auto reg = regName.num;
    if (reg == SP || reg == RZR) {
        if (regName.r31_is_sp) {
            os << "SP";
        } else {
            os << "XZR";
        }
    } else if (reg == RLINK) {
        os << "R30 (link register)";
    } else if (reg == 29) {
        os << "R29 (frame ptr)";
    } else {
        os << "R" << std::dec << +reg;
        if (reg < 0 || reg >= 32) os << " (invalid register!)";
    }
    return os;
}

std::string Register::toString() const {
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Instruction& inst) {
    os << std::showbase;
    if (inst.parseLevel <= 0) return os << "Unparsable";
    os << '"' << inst.kind[inst.parseLevel - 1] << '"';
    if (inst.valid) {
        if (inst.Rd >= 0) os << ", destination register: " << Register(inst.Rd, inst.RdCanBeSP);
        if (inst.Rd2 >= 0) os << ", destination register 2: " << Register(inst.Rd2, inst.RdCanBeSP);
        if (inst.imm) os << ", imm: " << std::hex << *inst.imm;
        if (inst.label) os << ", label: " << std::hex << asOffset(*inst.label) << " (" << *inst.label << ")";
        if (inst.branchType != Instruction::NOBRANCH) {
            os << ", branch type: " << Instruction::branchTypeInfo[inst.branchType];
        }
        if (inst.numSourceRegisters == 0) {
            if (inst.Rd >= 0) os << ", result: " << std::hex << inst.result;
        } else if (inst.numSourceRegisters > 0) {
            os << ", source registers: ";
            for (int j = 0; j < inst.numSourceRegisters; j++) {
                if (j != 0) os << ", ";
                os << Register(inst.Rs[j], j ? false : inst.Rs0CanBeSP);
            }
        }
    } else {
        os << ", marked invalid!";
    }
    return os << std::noshowbase;
}

std::string Instruction::toString() const {
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

bool Instruction::isPcRelAdr() {
    return this->kind[1] == pcRelAddr;
}

bool Instruction::isAddOrSubImm() {
    return this->kind[1] == addSubImm;
}

bool Instruction::isLoadOrStore() {
    return this->kind[0] == ldSt;
}
bool Instruction::isLoad() {
    return isLoadOrStore() && (strncmp(this->kind[2], "LD", 2) == 0);
}
bool Instruction::isStore() {
    return isLoadOrStore() && (strncmp(this->kind[2], "ST", 2) == 0);
}

bool Instruction::isHint() {
    return kind[0] == hintSt;
}

bool Instruction::isNOP() {
    return kind[parseLevel - 1] == nopSt;
}

bool Instruction::hasImmOffsetOnReg(uint_fast8_t reg) {
    if (!imm) return false;
    if (! (isLoadOrStore() || isAddOrSubImm()) ) return false;  // the immediate would not be simply "added"
    if (isStore()) {
        return (Rd == reg) || (Rd2 == reg);
    }
    for (uint_fast8_t i = 0; i < numSourceRegisters; i++) {
        if (Rs[i] == reg) return true;
    }
    return false;
}

Instruction* Instruction::findNthCall(int n, int rets) {
    return this->findNth(n, std::mem_fn(&Instruction::isCall), rets);
}

Instruction* Instruction::findNthDirectBranchWithoutLink(int n, int rets) {
    return this->findNth(n, [](Instruction* inst){return inst->branchType == DIR;}, rets);
}

Instruction* Instruction::findNthPcRelAdr(int n, int rets) {
    return this->findNth(n, std::mem_fn(&Instruction::isPcRelAdr), rets);
}

Instruction* Instruction::findNthImmOffsetOnReg(int n, uint_fast8_t reg, int rets) {
    return this->findNth(n, [reg](Instruction* inst){return inst->hasImmOffsetOnReg(reg);}, rets);
}

Instruction::Instruction(const int32_t* inst) {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("Instruction");
    addr = inst;
    auto pc = (uintptr_t)inst;
    auto base = getBase(pc);
    if (!base) {
        logger.critical("Instruction::Instruction: Could not get the .so base for pointer %p. "
            "It is likely not a valid pointer at all!", inst);
        return;
    }

    parseLevel = 0;
    parsed = false;
    auto code = *inst;
    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64#top
    uint_fast8_t top0 = bits(code, 28, 25);  // op0 for top-level only
    logger.debug("inst: ptr = 0x%lX (offset 0x%lX), bytes = %s (%X), top-level op0: %i",
        pc, pc - base, std::bitset<32>(code).to_string().c_str(), code, top0);
    // Bit patterns like 1x0x where x is any bit and all other bits must match are implemented by:
    // 1. (a & [1's where pattern has non-x]) == [pattern with x's as 0]
    // 2. (a | [1's where pattern has x])     == [pattern with x's as 1]
    //   2. is usually the shorter option when x's are mostly on the right, otherwise 1.
    if (top0 <= 3) {
        for (size_t i = 0; i < sizeof(kind) / sizeof(kind[0]); i++) {
            kind[parseLevel++] = "Invalid instruction";
            valid = false;
        }
    } else if ((top0 & 0b111) == 0b101) {  // x101
        // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register
        kind[parseLevel++] = "Data Processing -- Register";
        bool op0 = bits(code, 30, 30);
        bool op1 = bits(code, 28, 28);
        uint_fast8_t op2 = bits(code, 24, 21);
        uint_fast8_t op3 = bits(code, 15, 10);
        // not listed at the top level but all subcategories have it
        bool sf = bits(code, 31, 31);
        if (op1 == 0) {
            numSourceRegisters = 2;
            Rd = bits(code, 4, 0);
            auto Rn = Rs[0] = bits(code, 9, 5);
            auto Rm = Rs[1] = bits(code, 20, 16);
            if ((op2 & 0b1000) == 0) {  // 0xxx
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register#log_shift
                kind[parseLevel++] = "Logical (shifted register)";
                RdCanBeSP = Rs0CanBeSP = false;
                imm = bits(code, 15, 10);  // "imm6"
                shiftType = static_cast<ShiftType>(bits(code, 23, 22));  // "shift"

                bool N = bits(code, 21, 21);
                uint_fast8_t opc = bits(code, 30, 29);
                if (opc == 1) {
                    if (N == 0) {
                        if ((shiftType == LSL) && (*imm == 0) && Rn == RZR) {
                            kind[parseLevel++] = "MOV (register)";  // preferred alias
                            if (Rm == RZR) {
                                numSourceRegisters = 0;
                                result = 0;
                            } else {
                                Rs[0] = Rs[1];
                                numSourceRegisters = 1;
                            }
                        } else {
                            if (sf == 0) {
                                kind[parseLevel++] = "ORR (shifted register) - 32-bit";
                            } else {
                                kind[parseLevel++] = "ORR (shifted register) - 64-bit";
                            }
                        }
                    }
                }
            } else {  // op2 == 1xxx
                bool op = bits(code, 30, 30);
                bool S = bits(code, 29, 29);
                if ((op2 & 0b1) == 0) {  // 1xx0
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register#addsub_shift
                    kind[parseLevel++] = "Add/subtract (shifted register)";
                    RdCanBeSP = Rs0CanBeSP = false;
                    imm = bits(code, 15, 10);  // "imm6"
                    shiftType = static_cast<ShiftType>(bits(code, 23, 22));  // "shift"

                    if ((shiftType == ROR) || ((sf == 0) && ((*imm & 0b100000) != 0))) {
                        kind[parseLevel++] = unalloc;
                    } else {
                        // TODO: if sf == 0, all regs are 32-bit views
                        if (op == 0) {
                            if (S == 0) {
                                kind[parseLevel++] = sf ? "ADD (shifted register) - 64-bit" : "ADD (shifted register) - 32-bit";
                            } else {
                                if (Rd == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmn-shifted-register-compare-negative-shifted-register-an-alias-of-adds-shifted-register
                                    kind[parseLevel++] = "CMN (shifted register)";  // preferred alias
                                    Rd = -1;
                                } else {
                                    kind[parseLevel++] = sf ? "ADDS (shifted register) - 64-bit" : "ADDS (shifted register) - 32-bit";
                                }
                            }
                        } else {
                            if (S == 0) {
                                if (Rn == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/neg-shifted-register-negate-shifted-register-an-alias-of-sub-shifted-register
                                    kind[parseLevel++] = "NEG (shifted register)";  // preferred alias
                                    Rs[0] = Rs[1];
                                    numSourceRegisters = 1;
                                } else {
                                    kind[parseLevel++] = sf ? "SUB (shifted register) - 64-bit" : "SUB (shifted register) - 32-bit";
                                }
                            } else {
                                if (Rd == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmp-shifted-register-compare-shifted-register-an-alias-of-subs-shifted-register
                                    kind[parseLevel++] = "CMP (shifted register)";  // preferred alias
                                    Rd = -1;
                                } else if (Rn == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/negs-negate-setting-flags-an-alias-of-subs-shifted-register
                                    kind[parseLevel++] = "NEGS";  // preferred alias
                                    Rs[0] = Rs[1];
                                    numSourceRegisters = 1;
                                } else {
                                    kind[parseLevel++] = sf ? "SUBS (shifted register) - 64-bit" : "SUBS (shifted register) - 32-bit";
                                }
                            }
                        }
                    }
                    logger.debug("op1 = 0, op0: %i, op2: %i (1xxx), op3: %i", op0, op2, op3);
                } else {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register#addsub_ext
                    kind[parseLevel++] = "Add/subtract (extended register)";
                    RdCanBeSP = (!S);
                    Rs0CanBeSP = true;
                    uint_fast8_t opt = bits(code, 23, 22);
                    extendType = static_cast<ExtendType>(bits(code, 15, 13));  // "option"
                    imm = bits(code, 12, 10);  // "imm3"

                    if (Rn == SP) {
                        if (sf == 0) {
                            if (extendType == UXTW) shiftType = LSL;
                        } else {
                            if (extendType == UXTX) shiftType = LSL;
                        }
                    }

                    if ((opt != 0) || (*imm > 4)) {
                        kind[parseLevel++] = unalloc;
                    } else {
                        // TODO: if sf == 0, all regs are 32-bit views; else all are 64-bit except Rm is a 32-bit view iff option != x11
                        if (op == 0) {
                            if (S == 0) {
                                kind[parseLevel++] = sf ? "ADD (shifted register) - 64-bit" : "ADD (shifted register) - 32-bit";
                            } else {
                                if (Rd == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmn-shifted-register-compare-negative-shifted-register-an-alias-of-adds-shifted-register
                                    kind[parseLevel++] = "CMN (shifted register)";  // preferred alias
                                    Rd = -1;
                                } else {
                                    kind[parseLevel++] = sf ? "ADDS (shifted register) - 64-bit" : "ADDS (shifted register) - 32-bit";
                                }
                            }
                        } else {
                            if (S == 0) {
                                if (Rn == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/neg-shifted-register-negate-shifted-register-an-alias-of-sub-shifted-register
                                    kind[parseLevel++] = "NEG (shifted register)";  // preferred alias
                                    Rs[0] = Rs[1];
                                    numSourceRegisters = 1;
                                } else {
                                    kind[parseLevel++] = sf ? "SUB (shifted register) - 64-bit" : "SUB (shifted register) - 32-bit";
                                }
                            } else {
                                if (Rd == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmp-shifted-register-compare-shifted-register-an-alias-of-subs-shifted-register
                                    kind[parseLevel++] = "CMP (shifted register)";  // preferred alias
                                    Rd = -1;
                                } else if (Rn == RZR) {
                                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/negs-negate-setting-flags-an-alias-of-subs-shifted-register
                                    kind[parseLevel++] = "NEGS";  // preferred alias
                                    Rs[0] = Rs[1];
                                    numSourceRegisters = 1;
                                } else {
                                    kind[parseLevel++] = sf ? "SUBS (shifted register) - 64-bit" : "SUBS (shifted register) - 32-bit";
                                }
                            }
                        }
                    }
                }
            }
        } else {  // op1 == 1
            if (op2 == 0b100) {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register#condsel
                kind[parseLevel++] = "Conditional select";
                RdCanBeSP = Rs0CanBeSP = false;
                numSourceRegisters = 2;
                Rd = bits(code, 4, 0);
                Rs[0] = bits(code, 9, 5);  // Rn
                cond = bits(code, 15, 12);
                Rs[1] = bits(code, 20, 16);  // Rm

                bool op = op0;
                bool S = bits(code, 29, 29);
                op2 = bits(code, 11, 10);

                if (S || ((op2 & 0b10) != 0)) {  // 1x
                    kind[parseLevel++] = unalloc;
                } else if (op == 0) {
                    if (op2 == 0) {
                        kind[parseLevel++] = sf ? "CSEL - 64-bit" : "CSEL - 32-bit";
                    } else {
                        kind[parseLevel++] = sf ? "CSINC - 64-bit" : "CSINC - 32-bit";
                    }
                } else {  // op == 1
                    if (op2 == 0) {
                        kind[parseLevel++] = sf ? "CSINV - 64-bit" : "CSINV - 32-bit";
                    } else {
                        kind[parseLevel++] = sf ? "CSNEG - 64-bit" : "CSNEG - 32-bit";
                    }
                }
            } else if ((op2 & 0b1000) == 0b1000) {  // 1xxx
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-register#dp_3src
                kind[parseLevel++] = "Data-processing (3 source)";
                RdCanBeSP = Rs0CanBeSP = false;
                numSourceRegisters = 3;
                Rd = bits(code, 4, 0);
                Rs[0] = bits(code, 9, 5);  // Rn
                Rs[1] = bits(code, 20, 16);  // Rm
                auto Ra = Rs[2] = bits(code, 14, 10);
                if (Ra == RZR) {
                    numSourceRegisters = 2;
                }

                bool o0 = bits(code, 15, 15);
                auto op31 = bits(code, 23, 21);  // basically bits 2-0 of op
                auto op54 = bits(code, 30, 29);  // bits 4-3 of op
                if ((op54 != 0) || (op31 == 0b11) || (op31 == 0b100) || (op31 == 0b111) ||
                    ((o0 == 1) && ((op31 == 0b10) || (op31 == 0b110)) ) ||
                    ((sf == 0) && (op31 != 0))) {
                    kind[parseLevel++] = unalloc;
                } else if (op31 == 0) {
                    // TODO: All registers are viewed as 32-bit if sf == 0, otherwise 64-bit
                    if (o0 == 0) {
                        if (Ra == RZR) {
                            // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/mul-multiply-an-alias-of-madd
                            kind[parseLevel++] = "MUL";  // preferred alias
                        } else {
                            kind[parseLevel++] = sf ? "MADD - 64-bit" : "MADD - 32-bit";
                        }
                    } else {
                        if (Ra == RZR) {
                            // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/mneg-multiply-negate-an-alias-of-msub
                            kind[parseLevel++] = "MNEG";  // preferred alias
                        } else {
                            kind[parseLevel++] = sf ? "MSUB - 64-bit" : "MSUB - 32-bit";
                        }
                    }
                } else {
                    bool U = bits(code, 23, 23);
                    if ((op31 & 0b11) == 0b1) {  // 001, 101
                        // TODO: Rm and Rn are viewed as 32-bit (all others as 64-bit)
                        if (o0 == 0) {
                            if (Ra == RZR) {
                                // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/smull-signed-multiply-long-an-alias-of-smaddl
                                // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/umull-unsigned-multiply-long-an-alias-of-umaddl
                                kind[parseLevel++] = U ? "UMULL" : "SMULL";  // preferred alias
                            } else {
                                kind[parseLevel++] = U ? "UMADDL" : "SMADDL";
                            }
                        } else {
                            if (Ra == RZR) {
                                // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/smnegl-signed-multiply-negate-long-an-alias-of-smsubl
                                // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/umnegl-unsigned-multiply-negate-long-an-alias-of-umsubl
                                kind[parseLevel++] = U ? "UMNEGL" : "SMNEGL";  // preferred alias
                            } else {
                                kind[parseLevel++] = U ? "UMSUBL" : "SMSUBL";
                            }
                        }
                    } else {  // op31 == 010, 110
                        assert((op31 & 0b11) == 0b10);
                        // All registers are viewed as 64-bit
                        if (Ra == RZR) {
                            kind[parseLevel++] = U ? "UMULH" : "SMULH";  // the only name
                        } else {
                            kind[parseLevel++] = unalloc;
                        }
                    }
                }
            } else {
                logger.debug("op1 = 1, op0: %i, op2: %i (0xxx), op3: %i", op0, op2, op3);
            }
        }
    } else if ((top0 & 0b111) == 0b111) {  // x111
        // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-scalar-floating-point-and-advanced-simd
        kind[parseLevel++] = "Data Processing -- Scalar Floating-Point and Advanced SIMD";
    } else if ((top0 | 0b1) == 0b1001) {  // 100x
        // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate
        kind[parseLevel++] = "Data Processing -- Immediate";
        Rd = bits(code, 4, 0);
        bool sf = bits(code, 31, 31);
        uint_fast8_t op0 = bits(code, 25, 24);
        uint_fast8_t op1 = bits(code, 23, 22);
        if (op0 == 0) {
            // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#pcreladdr
            kind[parseLevel++] = pcRelAddr;  // "PC-rel. addressing"
            RdCanBeSP = false;
            numSourceRegisters = 0;
            bool op = sf;
            const uint_fast8_t ilh = 30, ill = 29, ihh = 23, ihl = 5;
            uint_fast8_t immlo = bits(code, ilh, ill);
            auto immhi = bits(code, ihh, ihl);
            logger.debug("immhi: 0x%X (%i), immlo: 0x%X (%i)", immhi, immhi, immlo, immlo);
            auto immI = (immhi << (ilh - ill + 1)) + immlo;
            uint_fast8_t immINumBits = ihh - ihl + 1 + ilh - ill + 1;
            if (op == 0b1) {
                kind[parseLevel++] = "ADRP";
                const uint_fast8_t zeros = 12;
                immI <<= zeros;
                immINumBits += zeros;
                pc = (pc >> zeros) << zeros;  // zero out the last 12 bits
            } else {
                kind[parseLevel++] = "ADR";
            }
            logger.debug("imm initial: 0x%X (%i); immNumBits: %i", immI, immI, immINumBits);
            // the documentation calls this imm, but it's not exposed in the instruction string
            auto privateImm = SignExtend<int64_t>(immI, immINumBits);
            result = pc + privateImm;
            label = (decltype(label)::value_type) result;
            logger.debug("imm: 0x%lX; result: 0x%lX (offset 0x%lX)", privateImm, result, result - base);
        } else if (op0 == 0b1) {
            numSourceRegisters = 1;
            Rs0CanBeSP = true;
            Rs[0] = bits(code, 9, 5);
            bool op = bits(code, 30, 30);
            bool S = bits(code, 29, 29);
            if ((op1 | 0b1) == 0b11) {  // 1x
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#addsub_immtags
                kind[parseLevel++] = "Add/subtract (immediate, with tags)";
                RdCanBeSP = true;
                if ((sf == 0) || ((sf == 1) && (S == 1))) {
                    kind[parseLevel++] = unalloc;
                }
            } else {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#addsub_imm
                kind[parseLevel++] = addSubImm;
                RdCanBeSP = !S;
                auto shift = op1;
                uint_fast16_t imm12 = bits(code, 21, 10);
                imm = ZeroExtend<int64_t>(imm12, 12) << 12 * shift;
                if (op == 0) {
                    if (S == 0) {
                        if ((*imm == 0) && ((Rd == SP) || (Rs[0] == SP))) {
                            // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/mov-tofrom-sp-move-between-register-and-stack-pointer-an-alias-of-add-immediate
                            kind[parseLevel++] = "MOV (to/from SP)";  // preferred alias
                        } else {
                            kind[parseLevel++] = sf ? "ADD (immediate) - 64-bit" : "ADD (immediate) - 32-bit";
                        }
                    } else {
                        if (Rd == RZR) {
                            // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmn-immediate-compare-negative-immediate-an-alias-of-adds-immediate
                            kind[parseLevel++] = "CMN (immediate)";  // preferred alias
                            Rd = -1;
                        } else {
                            kind[parseLevel++] = sf ? "ADDS (immediate) - 64-bit" : "ADDS (immediate) - 32-bit";
                        }
                    }
                } else {
                    imm = -*imm;  // the immediate should be subtracted for sub
                    if (S == 0) {
                        kind[parseLevel++] = sf ? "SUB (immediate) - 64-bit" : "SUB (immediate) - 32-bit";
                    } else {
                        if (Rd == RZR) {
                            // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/cmp-immediate-compare-immediate-an-alias-of-subs-immediate
                            kind[parseLevel++] = "CMP (immediate)";  // preferred alias
                            Rd = -1;
                        } else {
                            kind[parseLevel++] = sf ? "SUBS (immediate) - 64-bit" : "SUBS (immediate) - 32-bit";
                        }
                    }
                }
            }
        } else if ((op1 & 0b10) == 0) {  // 0x
            uint_fast8_t opc = bits(code, 30, 29);
            bool N = bits(code, 22, 22);
            uint_fast8_t immr = bits(code, 21, 16);
            uint_fast8_t imms = bits(code, 15, 10);
            uint_fast8_t Rn = bits(code, 9, 5);
            // TODO: both registers are viewed as 32-bit iff sf == 0, else 64-bit

            bool logical = (op0 == 0b10);
            auto masks = DecodeBitMasks(N, imms, immr, sf ? 64 : 32, logical);
            if (masks) {
                logger.debug("N: %i, immr: 0x%X (%u), imms: 0x%X, wmask: 0x%lX, tmask: 0x%lX", N, immr, immr, imms, masks->first, masks->second);
            } else {
                logger.debug("N: %i, immr: 0x%X (%u), imms: 0x%X, invalid bitmasks", N, immr, immr, imms);
                valid = false;
            }

            if (logical) {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#log_imm
                kind[parseLevel++] = "Logical (immediate)";
                RdCanBeSP = (opc != 0b11);  // for all but ANDS
                Rs0CanBeSP = false;
                if (masks) {
                    imm = masks->first;
                }

                if (Rn == RZR) {
                    numSourceRegisters = 0;
                } else {
                    numSourceRegisters = 1;
                    Rs[0] = Rn;
                }

                if ((sf == 0) && (N == 1)) {
                    kind[parseLevel++] = unalloc;
                } else if (opc == 0) {
                    if (Rn == RZR) result = 0;
                    kind[parseLevel++] = sf ? "AND (immediate) - 64-bit" : "AND (immediate) - 32-bit";
                } else if (opc == 0b1) {
                    if (Rn == RZR) {
                        // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/mov-bitmask-immediate-move-bitmask-immediate-an-alias-of-orr-immediate
                        kind[parseLevel++] = sf ? "MOV (bitmask immediate) - 64-bit" : "MOV (bitmask immediate) - 32-bit";
                        if (imm) {
                            result = *imm;
                            if (HighestSetBit(result, sizeof(result)*CHAR_BIT) < 16) {
                                logger.error("This instruction should have been assembled as MOVZ or MOVN?!");
                            }
                        }
                    } else {
                        kind[parseLevel++] = sf ? "ORR (immediate) - 64-bit" : "ORR (immediate) - 32-bit";
                    }
                } else if (opc == 0b10) {
                    if (imm && (Rn == RZR)) result = *imm;
                    kind[parseLevel++] = sf ? "EOR (immediate) - 64-bit" : "EOR (immediate) - 32-bit";
                } else {  // opc == 0b11
                    if (Rn == RZR) result = 0;
                    if (Rd == RZR) {
                        // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/tst-immediate-test-bits-immediate-an-alias-of-ands-immediate
                        kind[parseLevel++] = sf ? "TST (immediate) - 64-bit" : "TST (immediate) - 32-bit";  // preferred alias
                        Rd = -1;
                    } else {
                        kind[parseLevel++] = sf ? "ANDS (immediate) - 64-bit" : "ANDS (immediate) - 32-bit";
                    }
                }
            } else {
                assert(op0 == 0b11);  // logic error, there should be no other options!
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#bitfield
                kind[parseLevel++] = "Bitfield";
                RdCanBeSP = Rs0CanBeSP = false;

                /*  If <imms> >= <immr>, [these copy] a bitfield of (<imms>-<immr>+1) bits
                starting from bit position <immr> in Rn to the LSBs of Rd.
                    If <imms> < <immr>, [these copy] a bitfield of (<imms>+1) bits
                from the LSBs of Rn to bit position (regsize-<immr>) of Rd, where regsize is 32 iff sf == 0, else 64 bits. */
                if ((opc == 0b11) || (sf != N)) {
                    kind[parseLevel++] = unalloc;
                } else if (opc == 0) {
                    // In both cases the Rd bits below the bitfield are set to zero, and the bits above are sign extended.
                    // TODO https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/sbfm-signed-bitfield-move#commonps

                    // kind[parseLevel++] = sf ? "SBFM - 64-bit" : "SBFM - 32-bit";
                    // SBFM exclusively goes by aliases
                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/sbfm-signed-bitfield-move#aliasconditions
                    if (imms == (sf ? ONES(6) : ONES(5))) {
                        kind[parseLevel++] = sf ? "ASR (immediate) - 64-bit" : "ASR (immediate) - 32-bit";
                    } else if (imms < immr) {
                        kind[parseLevel++] = sf ? "SBFIZ - 64-bit" : "SBFIZ - 32-bit";
                    } else if ((immr == 0) && (imms == ONES(3))) {
                        kind[parseLevel++] = sf ? "SXTB - 64-bit" : "SXTB - 32-bit";
                    } else if ((immr == 0) && (imms == ONES(4))) {
                        kind[parseLevel++] = sf ? "SXTH - 64-bit" : "SXTH - 32-bit";
                    } else if (sf && (immr == 0) && (imms == ONES(5))) {
                        kind[parseLevel++] = "SXTW";  // only 64-bit
                    } else {
                        kind[parseLevel++] = sf ? "SBFX - 64-bit" : "SBFX - 32-bit";
                    }
                } else if (opc == 0b1) {
                    // In both cases the other bits of Rd remain unchanged.
                    // TODO https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/bfm-bitfield-move#commonps

                    // kind[parseLevel++] = sf ? "BFM - 64-bit" : "BFM - 32-bit";
                    // BFM exclusively goes by aliases
                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/bfm-bitfield-move#aliasconditions
                    if (imms >= immr) {
                        kind[parseLevel++] = sf ? "BFXIL - 64-bit" : "BFXIL - 32-bit";
                    } else if (Rn == RZR) {
                        kind[parseLevel++] = sf ? "BFC - 64-bit" : "BFC - 32-bit";
                        numSourceRegisters = 0;
                    } else {
                        kind[parseLevel++] = sf ? "BFI - 64-bit" : "BFI - 32-bit";
                    }
                } else {  // opc == 0b10
                    assert(opc == 0b10);
                    // In both cases the Rd bits below and above the bitfield are set to zero.
                    // TODO https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/ubfm-unsigned-bitfield-move#commonps

                    // kind[parseLevel++] = sf ? "UBFM - 64-bit" : "UBFM - 32-bit";
                    // UBFM exclusively goes by aliases
                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/ubfm-unsigned-bitfield-move#aliasconditions
                    if (imms == (sf ? ONES(6) : ONES(5))) {
                        kind[parseLevel++] = sf ? "LSR (immediate) - 64-bit" : "LSR (immediate) - 32-bit";
                    } else if (imms + 1 == immr) {
                        kind[parseLevel++] = sf ? "LSL (immediate) - 64-bit" : "LSL (immediate) - 32-bit";
                    } else if (imms < immr) {
                        kind[parseLevel++] = sf ? "UBFIZ - 64-bit" : "UBFIZ - 32-bit";
                    } else if (!sf && (immr == 0) && (imms == ONES(3))) {
                        kind[parseLevel++] = "UXTB";  // only 32-bit
                    } else if (!sf && (immr == 0) && (imms == ONES(4))) {
                        kind[parseLevel++] = "UXTH";  // only 32-bit
                    } else {
                        kind[parseLevel++] = sf ? "UBFX - 64-bit" : "UBFX - 32-bit";
                    }
                }
                logger.debug("sf == N == %i, opc: %i", sf, opc);
            }
        } else {  // op1 == 1x
            if (op0 == 0b10) {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#movewide
                kind[parseLevel++] = "Move wide (immediate)";
                // auto opc = bits(code, 30, 29);  // op21 for Extract
            } else {  // op0 == 11
                assert(op0 == 0b11);  // logic error, there should be no other options!
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/data-processing-immediate#extract
                kind[parseLevel++] = "Extract";
                // bool N = bits(code, 22, 22);
                // auto op21 = bits(code, 30, 29);
            }
        }
    } else if ((top0 | 0b1) == 0b1011) {  // 101x
        // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions
        kind[parseLevel++] = "Branches, Exception Generating and System instructions";
        Rd = -1;
        RdCanBeSP = Rs0CanBeSP = false;
        uint_fast8_t op0 = bits(code, 31, 29);
        uint_fast16_t op1 = bits(code, 25, 12);
        uint_fast8_t op2 = bits(code, 4, 0);
        if (op0 == 0b10) {
            if ((op1 & 0b10000000000000) != 0) {  // 1xxxxxxxxxxxxx
                kind[parseLevel++] = unalloc;
            } else {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions#condbranch
                kind[parseLevel++] = "Conditional branch (immediate)";
                numSourceRegisters = 0;
                bool o1 = bits(code, 24, 24);
                int_fast32_t imm19 = bits(code, 23, 5);
                bool o0 = bits(code, 4, 4);
                cond = bits(code, 3, 0);
                if (o0 || o1) {
                    kind[parseLevel++] = unalloc;
                } else {
                    kind[parseLevel++] = "B.cond";
                    label = (decltype(label)::value_type)(pc + (SignExtend<int64_t>(imm19, 19) << 2));
                    logger.debug("label: %lX", ((decltype(base))*label) - base);
                    branchType = DIR;
                }
            }
        } else if (op0 == 0b110) {
            if ((op1 & 0b10000000000000) != 0) {  // 1xxxxxxxxxxxxx
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions#branch_reg
                kind[parseLevel++] = "Unconditional branch (register)";
                numSourceRegisters = 1;
                Rs[0] = (uint8_t)bits(code, 9, 5);

                uint_fast8_t opc = bits(code, 24, 21);
                uint_fast8_t op2 = bits(code, 20, 16);
                uint_fast8_t op3 = bits(code, 15, 10);
                uint_fast8_t op4 = bits(code, 4, 0);
                if (op2 != 0b11111) {
                    kind[parseLevel++] = unalloc;
                } else if (opc == 0) {
                    branchType = INDIR;
                    if (op3 == 0) {
                        if (op4 != 0) {
                            kind[parseLevel++] = unalloc;
                        } else {
                            kind[parseLevel++] = "BR";
                        }
                    } else {
                        logger.debug("TODO: BRA[A/AZ/B/BZ]! opc = 0, op3: %i, op4: %i", op3, op4);
                    }
                } else if (opc == 0b1) {
                    branchType = INDCALL;
                    Rd = RLINK;
                    imm = pc + 4;  // CANNOT WRITE TO result AS IT OVERWRITES Rs[0]
                    if (op3 == 0) {
                        if (op4 != 0) {
                            kind[parseLevel++] = unalloc;
                        } else {
                            kind[parseLevel++] = "BLR";
                        }
                    } else {
                        logger.debug("TODO: BLRA[A/AZ/B/BZ]! opc = 0, op3: %i, op4: %i", op3, op4);
                    }
                } else if (opc == 0b10) {
                    branchType = RET;
                    if (op3 == 0) {
                        if (op4 != 0) {
                            kind[parseLevel++] = unalloc;
                        } else {
                            kind[parseLevel++] = "RET";
                        }
                    } else {
                        logger.debug("TODO: RETAA/RETAB! opc = 0b10, op3: %i, op4: %i", op3, op4);
                    }
                } else {
                    logger.debug("opc: %i, op3: %i, op4: %i", opc, op3, op4);
                }
            } else if (op1 == 0b01000000110010) {
                if (op2 == 0b11111) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions
                    // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/hint-hint-instruction
                    kind[parseLevel++] = hintSt;
                    uint_fast8_t CRm = bits(code, 11, 8);
                    op2 = bits(code, 7, 5);
                    if (CRm == 0 && op2 == 0) {
                        // NOP
                        kind[parseLevel++] = nopSt;
                    } else {
                        logger.debug("HINT instruction, CRm = %i, op2 = %i", CRm, op2);
                    }
                } else {
                    kind[parseLevel++] = unalloc;
                }
            } else {
                logger.debug("op0 = 0b110, op1: %lu", op1);
            }
        } else if ((op0 & 0b11) == 0) {  // x00
            // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions#branch_imm
            kind[parseLevel++] = "Unconditional branch (immediate)";
            numSourceRegisters = 0;
            bool op = bits(code, 31, 31);
            uint_fast32_t imm26 = bits(code, 25, 0);
            auto offset = SignExtend<int64_t>(imm26, 26) << 2;
            label = (decltype(label)::value_type)(pc + offset);

            int64_t off = ((decltype(base))*label) - base;
            logger.debug("label: %lX", off);
            if ((off < 0) || (off >= 0x03000000)) {
                logger.error("0x%lX is probably not a valid offset! Please investigate!", off);
            }
            if (!op) {
                kind[parseLevel++] = "B";
                branchType = DIR;
            } else {
                kind[parseLevel++] = "BL";
                branchType = DIRCALL;
                Rd = RLINK;
                result = pc + 4;
            }
        } else if ((op0 & 0b11) == 0b01) {  // x01
            if ((op1 & 0b10000000000000) == 0) {  // 0xxxxxxxxxxxxx
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions#compbranch
                kind[parseLevel++] = "Compare and branch (immediate)";
                numSourceRegisters = 1;
                Rs[0] = bits(code, 4, 0);  // Rt

                bool sf = bits(code, 31, 31);
                bool op = bits(code, 24, 24);
                auto imm19 = bits(code, 23, 5);
                // TODO: the only register (Rt) is viewed as 32-bit iff sf == 0
                if (op == 0) {
                    kind[parseLevel++] = sf ? "CBZ - 64-bit" : "CBZ - 32-bit";
                } else {
                    kind[parseLevel++] = sf ? "CBNZ - 64-bit" : "CBNZ - 32-bit";
                }
                label = (decltype(label)::value_type)(SignExtend<int64_t>(imm19, 19) << 2);
            } else {  // op1 == 1xxxxxxxxxxxxx
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/branches-exception-generating-and-system-instructions#testbranch
                kind[parseLevel++] = "Test and branch (immediate)";
                numSourceRegisters = 1;
                Rs[0] = bits(code, 4, 0);  // Rt

                bool b5 = bits(code, 31, 31);
                bool op = bits(code, 24, 24);
                auto b40 = bits(code, 23, 19);
                auto imm14 = bits(code, 18, 5);
                // TODO: the only register (Rt) is viewed as 32-bit iff b5 == 0 (only matters in this case for pretty printing)

                imm = (((int)b5) << 4) | ((int)b40);  // the bit index in Rt to test
                branchType = DIR;
                label = (decltype(label)::value_type)(pc + (SignExtend<int64_t>(imm14, 14) << 2));
                kind[parseLevel++] = op ? "TBNZ" : "TBZ";
            }
        } else {
            logger.debug("op0: %u, op1: %s, op2: %u", op0, std::bitset<14>(op1).to_string().c_str(), op2);
        }
    } else if ((top0 & 0b101) == 0b100) {  // x1x0
        // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores
        kind[parseLevel++] = ldSt;
        uint_fast8_t op0 = bits(code, 31, 28);
        bool op1 = bits(code, 26, 26);
        uint_fast8_t op2 = bits(code, 24, 23);
        uint_fast8_t op3 = bits(code, 21, 16);
        uint_fast8_t op4 = bits(code, 11, 10);
        if ((op0 & 0b11) == 0b11) {  // xx11
            uint_fast8_t size = bits(code, 31, 30);
            auto V = op1;
            uint_fast8_t opc = bits(code, 23, 22);

            uint_fast8_t Rt = bits(code, 4, 0);  // cannot be SP
            uint_fast8_t Rn = bits(code, 9, 5);  // can be SP

            bool hasImmOffset = false;
            if ((op2 | 0b1) == 0b11) {  // 1x
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldst_pos
                kind[parseLevel++] = "Load/store register (unsigned immediate)";
                uint_fast16_t imm12 = bits(code, 21, 10);
                logger.debug("size: %i; imm12: 0x%lX", size, imm12);
                imm = ZeroExtend<int64_t>(imm12, 12) << size;
                wback = false;
                postindex = false;
                hasImmOffset = true;
            } else if ((op3 & 0b100000) == 0) {  // 0xxxxx
                uint_fast16_t imm9 = bits(code, 20, 12);
                logger.debug("size: %i; imm9: 0x%lX", size, imm9);
                imm = SignExtend<int64_t>(imm9, 9);

                if (op4 == 0b11) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldst_immpre
                    kind[parseLevel++] = "Load/store register (immediate pre-indexed)";
                    wback = true;
                    postindex = false;
                    hasImmOffset = true;
                } else if (op4 == 0b1) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldst_immpost
                    kind[parseLevel++] = "Load/store register (immediate post-indexed)";
                    wback = true;
                    postindex = true;
                    hasImmOffset = true;
                } else {
                    logger.debug("op0 = xx11, op2 = 0x, op3 = 0xxxxx, op4: %i", op4);
                }
            } else {
                if (op4 == 0b10) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldst_regoff
                    kind[parseLevel++] = "Load/store register (register offset)";
                    numSourceRegisters = 2;
                    Rs[1] = bits(code, 20, 16);  // Rm, cannot be SP

                    extendType = static_cast<ExtendType>(bits(code, 15, 13));  // <extend>, encoded in "option"
                    bool S = bits(code, 12, 12);
                    bool shifted = (extendType == UXTX);
                    if (shifted) shiftType = LSL;
                    imm = (S ? size : 0);  // <amount>

                    // TODO: when option & 0b1 == 0, Rm is a 32-bit view, else 64
                    // TODO: when opc & 0b10 == 0, Rt is a 32-bit view, else 32 iff opc & 0b1 == 1

                    if ((extendType & 0b10) == 0 || ( ((size & 0b1) == 0b1) && V == 1 && ((opc & 0b10) != 0) )) {
                        kind[parseLevel++] = unalloc;
                    } else if (V == 0) {
                        if (opc == 0) {
                            Rs[0] = Rt; Rs0CanBeSP = false;
                            Rd = Rn; RdCanBeSP = true;
                            if (size == 3) {
                                kind[parseLevel++] = "STR (register) - 64-bit";
                            } else if (size == 2) {
                                kind[parseLevel++] = "STR (register) - 32-bit";
                            } else if (size == 1) {
                                kind[parseLevel++] = "STRH (register)";
                            } else if (shifted) {
                                kind[parseLevel++] = "STRB (register) - shifted register";
                            } else {
                                kind[parseLevel++] = "STRB (register) - extended register";
                            }
                        } else {
                            Rs[0] = Rn; Rs0CanBeSP = true;
                            Rd = Rt; RdCanBeSP = false;
                            if (opc == 0b1) {
                                if (size == 3) {
                                    kind[parseLevel++] = "LDR (register) - 64-bit";
                                } else if (size == 2) {
                                    kind[parseLevel++] = "LDR (register) - 32-bit";
                                } else if (size == 1) {
                                    kind[parseLevel++] = "LDRH (register)";
                                } else if (shifted) {
                                    kind[parseLevel++] = "LDRB (register) - shifted register";
                                } else {
                                    kind[parseLevel++] = "LDRB (register) - extended register";
                                }
                            } else {  // opc == 10 or 11
                                bool opc64 = (opc == 0b10);
                                if (size == 3) {
                                    kind[parseLevel++] = opc64 ? "PRFM (register)" : unalloc;
                                } else if (size == 2) {
                                    kind[parseLevel++] = opc64 ? "LDRSW (register)" : unalloc;
                                } else if (size == 1) {
                                    kind[parseLevel++] = opc64 ? "LDRSH (register) - 64-bit" : "LDRSH (register) - 32-bit";
                                } else {
                                    // TODO: Rt is a 32-bit view iff opc == 0b11;
                                    if (opc64) {
                                        if (shifted) {
                                            kind[parseLevel++] = "LDRSB (register) - 64-bit with shifted register offset";
                                        } else {
                                            kind[parseLevel++] = "LDRSB (register) - 64-bit with extended register offset";
                                        }
                                    } else {
                                        if (shifted) {
                                            kind[parseLevel++] = "LDRSB (register) - 32-bit with shifted register offset";
                                        } else {
                                            kind[parseLevel++] = "LDRSB (register) - 32-bit with extended register offset";
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        logger.debug("TODO: STR/LDR (register, SIMD&FP)");
                    }
                } else {
                    logger.debug("op0 = xx11, op2 = 0x, op3 = 1xxxxx, op4: %i", op4);
                }
            }

            if (hasImmOffset) {
                numSourceRegisters = 1;

                if (V == 0) {
                    if (opc == 0) {
                        Rs[0] = Rt; Rs0CanBeSP = false;
                        Rd = Rn; RdCanBeSP = true;
                        if (size == 3) {
                            kind[parseLevel++] = "STR (immediate) - 64-bit";
                        } else if (size == 2) {
                            kind[parseLevel++] = "STR (immediate) - 32-bit";
                        } else if (size == 1) {
                            kind[parseLevel++] = "STRH (immediate)";
                        } else {
                            kind[parseLevel++] = "STRB (immediate)";
                        }
                    } else {
                        Rs[0] = Rn; Rs0CanBeSP = true;
                        Rd = Rt; RdCanBeSP = false;
                        if (opc == 0b1) {
                            if (size == 3) {
                                kind[parseLevel++] = "LDR (immediate) - 64-bit";
                            } else if (size == 2) {
                                kind[parseLevel++] = "LDR (immediate) - 32-bit";
                            } else if (size == 1) {
                                kind[parseLevel++] = "LDRH (immediate)";
                            } else {
                                kind[parseLevel++] = "LDRB (immediate)";
                            }
                        } else {  // opc == 10 or 11
                            bool opc64 = (opc == 0b10);
                            if (size == 3) {
                                kind[parseLevel++] = opc64 ? "PRFM (immediate)" : unalloc;
                            } else if (size == 2) {
                                kind[parseLevel++] = opc64 ? "LDRSW (immediate)" : unalloc;
                            } else if (size == 1) {
                                kind[parseLevel++] = opc64 ? "LDRSH (immediate) - 64-bit" : "LDRSH (immediate) - 32-bit";
                            } else {
                                kind[parseLevel++] = opc64 ? "LDRSB (immediate) - 64-bit" : "LDRSB (immediate) - 32-bit";
                            }
                        }
                    }
                } else {
                    logger.debug("V: %i (TODO: SIMD&FP STR/LDR)", V);
                }
            }
        } else if ((op0 & 0b11) == 0b10) {  // xx10
            uint_fast8_t opc = bits(code, 31, 30);
            auto V = op1;
            bool L = bits(code, 22, 22);
            int_fast8_t imm7 = bits(code, 21, 15);
            uint_fast8_t Rt = bits(code, 4, 0);  // cannot be SP
            uint_fast8_t Rn = bits(code, 9, 5);  // can be SP
            uint_fast8_t Rt2 = bits(code, 14, 10);  // cannot be SP

            static constexpr int scaleArr[] = {2, 4, 3};
            int scale = scaleArr[opc];
            imm = SignExtend<int64_t>(imm7, 7) << scale;

            if (op2 == 0) {
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldstnapair_offs
                kind[parseLevel++] = "Load/store no-allocate pair (offset)";
                logger.debug("opc: %i, V: %i, L: %i", opc, V, L);
            } else {
                if (op2 == 0b1) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldstpair_post
                    kind[parseLevel++] = "Load/store register pair (post-indexed)";
                    wback = true;
                    postindex = true;
                } else if (op2 == 0b10) {
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldstpair_off
                    kind[parseLevel++] = "Load/store register pair (offset)";
                    wback = false;
                    postindex = false;
                } else {  // 11
                    // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#ldstpair_pre
                    kind[parseLevel++] = "Load/store register pair (pre-indexed)";
                    wback = true;
                    postindex = false;
                }

                if (opc == 0b11) {
                    kind[parseLevel++] = unalloc;
                } else if (!V && !L) {
                    numSourceRegisters = 2;
                    Rs[0] = Rt; Rs0CanBeSP = true;
                    Rs[1] = Rt2;
                    Rd = Rn; RdCanBeSP = true;
                    if (opc == 0) {
                        kind[parseLevel++] = "STP - 32-bit";
                    } else if (opc == 0b1) {
                        kind[parseLevel++] = "STGP";
                    } else {  // 10
                        kind[parseLevel++] = "STP - 64-bit";
                    }
                } else if (!V && L) {
                    numSourceRegisters = 1;
                    Rs[0] = Rn; Rs0CanBeSP = true;
                    Rd = Rt; RdCanBeSP = false;
                    Rd2 = Rt2;
                    if (opc == 0) {
                        kind[parseLevel++] = "LDP - 32-bit";
                    } else if (opc == 0b1) {
                        kind[parseLevel++] = "LDPSW";
                    } else {  // 10
                        kind[parseLevel++] = "LDP - 64-bit";
                    }
                } else {
                    logger.debug("TODO: SIMD&FP LDP/STP. opc: %i, V: %i, L: %i", opc, V, L);
                }
            }
        } else if (op0 == 0b1001 && op1 == 0 && (op2 & 0b10) != 0 && (op3 & 0b100000) != 0) {
            // This is an unalloc, specific for op0 = 5
            kind[parseLevel++] = unalloc;
        } else if ((op0 & 0b11) == 0b01) { // xx01
            if ((op2 & 0b10) < 0b10) { // 0x
                // https://developer.arm.com/docs/ddi0596/a/top-level-encodings-for-a64/loads-and-stores#loadlit
                kind[parseLevel++] = "Load register (literal)";
                uint_fast8_t opc = bits(code, 31, 30);
                uint_fast8_t V = bits(code, 26, 26);
                uint_fast32_t imm19 = bits(code, 23, 5);
                uint_fast8_t rt = bits(code, 4, 0);
                logger.debug("opc: %i, V: %i, imm19: %lu, rt: %i", opc, V, imm19, rt);

                // inst: ptr = 0x7F6D60C400 (offset 0x1A15400), bytes = 01011000000000000000000001010001 (58000051), top-level op0: 12
                // op0: 5, op2: 0, op3: 0, op4: 0
                if (V == 1) {
                    if (opc == 0b11) {
                        kind[parseLevel++] = unalloc;
                    } else {
                        logger.debug("TODO: SIMD Load register literal");
                    }
                } else {
                    if (opc == 0b00) {
                        // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/ldr-literal-load-register-literal
                        kind[parseLevel++] = "LDR (literal) - 32-bit";
                        Rd = rt;
                        RdCanBeSP = true;
                        label = reinterpret_cast<const int32_t*>(reinterpret_cast<int64_t>(addr) + SignExtend<int64_t>(imm19 << 2, 32));
                        // Actual data is located at PC + label, is 32 bits.
                        size = 4;
                    } else if (opc == 0b01) {
                        // https://developer.arm.com/docs/ddi0596/a/a64-base-instructions-alphabetic-order/ldr-literal-load-register-literal
                        kind[parseLevel++] = "LDR (literal) - 64-bit";
                        Rd = rt;
                        RdCanBeSP = true;
                        label = reinterpret_cast<const int32_t*>(reinterpret_cast<int64_t>(addr) + SignExtend<int64_t>(imm19 << 2, 32));
                        size = 8;
                        // Actual data is located at PC + label, is 64 bits.
                    } else if (opc == 0b10) {
                        kind[parseLevel++] = "LDRSW (literal)";
                    } else if (opc == 0b11) {
                        kind[parseLevel++] = "PRFM (literal)";
                    }
                }
            }
        } else {
            logger.debug("op0: %i, op2: %i, op3: %i, op4: %i", op0, op2, op3, op4);
        }
    } else {
        logger.error("Our top-level bit patterns have a gap!");
        SAFE_ABORT();
    }
    if (parseLevel < sizeof(kind) / sizeof(kind[0])) {
        logger.warning("Could not complete parsing of 0x%lX (offset %lX) - need more handling for kind '%s'!", pc, pc - base, kind[parseLevel - 1]);
    } else {
        parsed = true;
        if (kind[parseLevel - 1] == unalloc) {
            valid = false;
        }
    }
}

/*
void InstructionTree::Eval(ProgramState* state) {
    // TODO: if instruction is indirect branch, use state to populate branch
}
*/

InstructionTree* FindOrCreateInstruction(const int32_t* pc, ParseState& parseState, const char* msg) {
    auto p = parseState.codeToInstTree.find(pc);
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("FindOrCreateInstruction");
    if (p != parseState.codeToInstTree.end()) {
        logger.debug("not recursing: InstructionTree for %p (offset %lX) already exists", pc, asOffset((uintptr_t)pc));
        return p->second;
    } else {
        logger.debug("%s (pc %p, offset %lX)", msg, pc, asOffset((uintptr_t)pc));
        auto inst = new (std::nothrow) InstructionTree(pc);
        parseState.frontier.push({inst, parseState.dependencyMap});  // the inserted depMap is a copy
        parseState.codeToInstTree[pc] = inst;
        return inst;
    }
}

void ProcessRegisterDependencies(Instruction* inst, uint_fast8_t Rd, decltype(ParseState::dependencyMap)& depMap) {
    auto newDeps = decltype(ParseState::dependencyMap)::value_type();  // storage for the dependencies of a single register
    for (uint_fast8_t i = 0; i < inst->numSourceRegisters; i++) {
        auto Rs = inst->Rs[i];
        if ((Rs < 0) || ((size_t)Rs >= depMap.max_size())) {
            static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("ProcessRegisterDependencies");
            logger.critical("Instruction is wrong! numSourceRegisters = %i but Rs[%i] = %i\n%s", inst->numSourceRegisters, i, Rs,
                inst->toString().c_str());
            SAFE_ABORT();
        }
        newDeps |= depMap[Rs];
    }
    depMap[Rd] = std::move(newDeps);
}

void ProcessRegisterDependencies(Instruction* inst, decltype(ParseState::dependencyMap)& depMap) {
    if (inst->Rd >= 0) ProcessRegisterDependencies(inst, inst->Rd, depMap);
    if (inst->Rd2 >= 0) ProcessRegisterDependencies(inst, inst->Rd2, depMap);
}

bool OnlySelfDeps(uint_fast8_t reg, decltype(ParseState::dependencyMap)& depMap) {
    auto deps = depMap[reg];
    return ((deps.count() == 1) && deps[reg]);
}

std::string DepMapToString(decltype(ParseState::dependencyMap)& depMap) {
    std::ostringstream ss;
    ss << "{self deps: [";
    for (uint_fast8_t i = 0; i < depMap.max_size(); i++) {
        if ((i != 0) && ((i % 8) == 0)) {
            ss << "|";
        }
        if (depMap[i].none()) {
            ss << " ";
        } else if (OnlySelfDeps(i, depMap)) {
            ss << "O";  // a loop
        } else {
            ss << ">";  // deps are listed on right
        }
    }
    ss << "]; ";
    bool first = true;
    for (uint_fast8_t i = 0; i < depMap.max_size(); i++) {
        if (!OnlySelfDeps(i, depMap) && depMap[i].any()) {
            if (!first) ss << "; ";
            ss << Register(i, true) << " deps: ";
            bool innerFirst = true;
            for (uint_fast8_t dep = 0; dep < depMap[i].size(); dep++) {
                if (!depMap[i][dep]) continue;
                if (!innerFirst) ss << ", ";
                ss << Register(dep, true);
                innerFirst = false;
            }
            first = false;
        }
    }
    ss << "}" << std::flush;
    return ss.str();
}

void InstructionTree::PopulateChildren(ParseState& parseState) {
    auto pc = this->addr;
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("PopulateChildren");
    logger.debug("InstructionTree: %p, %s", pc, this->toString().c_str());
    // If instruction was not fully parsed, stop.
    if (!parsed || !valid) return;

    if ((this->numSourceRegisters < 0) || (this->Rs[this->numSourceRegisters - 1] < 0)) {
        logger.error("The Instruction constructor did not properly parse this instruction's source registers! Please fix!");
    }

    // if instruction is return, stop parsing.
    if (isReturn()) return;

    // populate parseState.dependencyMap using this' dest and source registers
    ProcessRegisterDependencies(this, parseState.dependencyMap);

    // if instruction is direct branch, address to jump to is in "label"
    if (isDirectBranch()) {
        CRASH_UNLESS(label);
        const int32_t* addr = *label;
        // Note: these do not edit the dependency map so there is no need to take a backup of it ourselves
        branch = FindOrCreateInstruction(addr, parseState, "InstructionTree: recursing to branch location");
        if (isCall()) {
            parseState.functionCandidates.insert({addr, parseState.dependencyMap});  // the inserted depMap is a copy
        }
    } else {
        parseState.otherJumps.insert({pc, parseState.dependencyMap});
    }
    // unless instruction is unconditional branch, populate noBranch with next instruction in memory
    if (!isUnconditionalBranch()) {
        noBranch = FindOrCreateInstruction(pc + 1, parseState, "InstructionTree: recursing to next instruction");
    }
}

InstructionTree::InstructionTree(const int32_t* pc): Instruction(pc) {
}

std::ostream& operator<<(std::ostream& os, const AssemblyFunction& func) {
    os << std::hex << std::uppercase;
    os << "Function candidates: " << std::endl;
    for (auto p : func.parseState.functionCandidates) {
        os << "0x" << uintptr_t(p.first) << " (offset 0x" << asOffset((uintptr_t)p.first) << "): depMap "
            << DepMapToString(p.second) << std::endl;
    }
    os << "Other jumps: " << std::endl;
    for (auto p : func.parseState.otherJumps) {
        os << "0x" << uintptr_t(p.first) << " (offset 0x" << asOffset((uintptr_t)p.first) << "): depMap "
            << DepMapToString(p.second) << std::endl;
    }
    return os << std::nouppercase << std::dec;
}

std::string AssemblyFunction::toString() const {
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

AssemblyFunction::AssemblyFunction(const int32_t* pc): parseState() {
    static auto logger = Logger::get().WithContext("instruction-parsing").WithContext("AssemblyFunction");
    logger.debug("Starting dependency map: %s", DepMapToString(parseState.dependencyMap).c_str());
    auto root = new InstructionTree(pc);
    parseState.frontier.push({root, std::move(parseState.dependencyMap)});
    while (!parseState.frontier.empty()) {
        auto p = parseState.frontier.top();
        parseState.frontier.pop();

        parseState.dependencyMap = std::move(p.second);
        p.first->PopulateChildren(parseState);
    }
}
