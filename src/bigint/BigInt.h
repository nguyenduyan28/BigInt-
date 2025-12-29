// BigInt core: arbitrary-size signed integer with base 1e9 limbs.
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class BigInt {
public:
    BigInt();                    // zero
    explicit BigInt(int64_t v);  // convenience for tests/utility

    static BigInt fromString(std::string_view text);
    std::string toString() const;

    BigInt operator-() const;

    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;

    int sign() const { return sign_; }

private:
    static constexpr uint32_t BASE = 1000000000; // 1e9

    int sign_;                  // -1, 0, or +1
    std::vector<uint32_t> limbs_; // little-endian limbs

    void normalize();
    static int cmpAbs(const BigInt& a, const BigInt& b);
    static int cmp(const BigInt& a, const BigInt& b);

    static BigInt addAbs(const BigInt& a, const BigInt& b);
    static BigInt subAbs(const BigInt& a, const BigInt& b); // assumes |a| >= |b|
    static BigInt mulAbs(const BigInt& a, const BigInt& b);
    static BigInt mulAbsByUint(const BigInt& a, uint32_t m);
    static BigInt absValue(const BigInt& v);
    static BigInt modAbs(const BigInt& a, const BigInt& b);
};
