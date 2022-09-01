#include <bitset>
#include <fmt/format.h>


struct Register {
    enum Reg {
        RAX,
        RBX,
        RCX,
        RDX,
        RSP,
        RBP,
        RSI,
        RDI,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        MAX_COUNT,
    };

    Register() {
        for (idx = Reg::R8; idx <= Reg::R15; ++idx) {
            if (!Register::m_registers.test(idx)) {
                Register::m_registers.set(idx);
                break;
            }
        }
    }

    Register(Reg reg) {
        idx = reg;
        Register::m_registers.set(idx);
    }

    ~Register() {
        Register::m_registers.flip(idx);
    }

    friend struct fmt::formatter<Register>;

private:
    static constexpr std::array registerNames64 {
        "rax",
        "rbx",
        "rcx",
        "rdx",
        "rsp",
        "rbp",
        "rsi",
        "rdi",
        "r8",
        "r9",
        "r10",
        "r11",
        "r12",
        "r13",
        "r14",
        "r15",
        "MAX_COUNT",
    };

    static constexpr std::array registerNames32 {
        "eax",
        "ebx",
        "ecx",
        "edx",
        "esp",
        "ebp",
        "esi",
        "edi",
        "r8d",
        "r9d",
        "r10d",
        "r11d",
        "r12d",
        "r13d",
        "r14d",
        "r15d",
        "MAX_COUNT",
    };

    static constexpr std::array registerNames16 {
        "ax",
        "bx",
        "cx",
        "dx",
        "sp",
        "bp",
        "si",
        "di",
        "r8w",
        "r9w",
        "r10w",
        "r11w",
        "r12w",
        "r13w",
        "r14w",
        "r15w",
        "MAX_COUNT",
    };

    static constexpr std::array registerNames8 {
        "al",
        "bl",
        "cl",
        "dl",
        "spl",
        "bpl",
        "sil",
        "dil",
        "r8b",
        "r9b",
        "r10b",
        "r11b",
        "r12b",
        "r13b",
        "r14b",
        "r15b",
        "MAX_COUNT",
    };

    static std::bitset<15> m_registers;

    std::size_t idx { 0 };
};

template<>
struct fmt::formatter<Register> {
    int representation = 0;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == '8' || *it == '1' || *it == '3' || *it == '6'))
            representation = ((unsigned char)*it++) - 48;

        if (it != end && (*it == '4' || *it == '2' || *it == '6')) {
            representation *= 10;
            representation += ((unsigned char)*it++) - 48;
        }

        if (representation != 8 && representation != 16 && representation != 32 && representation != 64)
            throw format_error("invalid format");
        return it;
    }

    template<typename FormatContext>
    auto format(const Register& type, FormatContext& ctx) {
        if (representation == 64)
            return fmt::format_to(ctx.out(), "{}", Register::registerNames64[type.idx]);
        if (representation == 32)
            return fmt::format_to(ctx.out(), "{}", Register::registerNames32[type.idx]);
        if (representation == 16)
            return fmt::format_to(ctx.out(), "{}", Register::registerNames16[type.idx]);
        if (representation == 8)
            return fmt::format_to(ctx.out(), "{}", Register::registerNames8[type.idx]);
        return fmt::format_to(ctx.out(), "{} {}", "hallo", representation);
    }
};
