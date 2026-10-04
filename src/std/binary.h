#ifndef STDLIB_BINARY_H
    #define STDLIB_BINARY_H
    #include <climits>
    #include <cstdint>
    #include <string>
    typedef std::uint8_t _obn_byte;
    constexpr unsigned _obn_binaryAnd(unsigned a, unsigned b)
    {
        return a & b;
    }
    constexpr unsigned _obn_binaryOr(unsigned a, unsigned b)
    {
        return a | b;
    }
    constexpr unsigned _obn_binaryXor(unsigned a, unsigned b)
    {
        return a ^ b;
    }
    constexpr unsigned _obn_binaryNot(unsigned a)
    {
        return ~a;
    }
    constexpr unsigned _obn_binaryLeftShift(unsigned a, unsigned amount)
    {
        return a << amount;
    }
    constexpr unsigned _obn_binaryRightShift(unsigned a, unsigned amount)
    {
        return a >> amount;
    }
    constexpr unsigned _obn_toggleBit(unsigned a, unsigned position)
    {
        return a ^ (1 << position);
    }
    constexpr unsigned _obn_setBit(unsigned a, unsigned position)
    {
        return a | (1 << position);
    }
    constexpr unsigned _obn_clearBit(unsigned a, unsigned position)
    {
        return a & ~(1 << position);
    }
    constexpr bool _obn_testBit(unsigned a, unsigned position)
    {
        return (a & (1 << position)) != 0;
    }
    constexpr unsigned _obn_countSetBits(unsigned a)
    {
        unsigned count = 0;
        while (a)
        {
            count += (a & 1);
            a >>= 1;
        }
        return count;
    }
    constexpr unsigned countLeadingZeros(unsigned a) {
        if (a == 0) return sizeof(int) * CHAR_BIT;
        unsigned count = 0;
        while ((a & INT_MAX) == 0)
        {
            count++;
            a <<= 1;
        }
        return count;
    }
    constexpr int _obn_maxIntValue = INT_MAX;
    constexpr int _obn_minIntValue = INT_MIN;
#endif