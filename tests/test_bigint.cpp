#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

#include "../src/bigint/BigInt.h"

void checkRoundtrip(const std::string& input, const std::string& expected) {
    BigInt v = BigInt::fromString(input);
    assert(v.toString() == expected);
}

void testExamples() {
    BigInt a = BigInt::fromString("100000000000000000");
    BigInt b = BigInt::fromString("100000000000000004");
    assert((a + b).toString() == "200000000000000004");
    assert((a - b).toString() == "-4");

    BigInt m1 = BigInt::fromString("1234567891");
    BigInt m2 = BigInt::fromString("1234567892");
    assert((m1 * m2).toString() == "1524157878722755772");

    assert((b % a).toString() == "4");
}

void testSigns() {
    BigInt n5(-5);
    BigInt p5(5);
    BigInt p3(3);

    assert((n5 + n5).toString() == "-10");
    assert((n5 + p5).toString() == "0");
    assert((p5 + n5).toString() == "0");
    assert((p5 - n5).toString() == "10");
    assert((n5 - p5).toString() == "-10");
    assert((p5 * p3).toString() == "15");
    assert((p5 * n5).toString() == "-25");
    assert((n5 * n5).toString() == "25");
    assert((BigInt(-10) % BigInt(3)).toString() == "-1");
}

void testRandom() {
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<int64_t> dist(-1000000, 1000000);

    for (int i = 0; i < 200; ++i) {
        int64_t x = dist(rng);
        int64_t y = dist(rng);
        BigInt bx(x);
        BigInt by(y);

        assert((bx + by).toString() == std::to_string(x + y));
        assert((bx - by).toString() == std::to_string(x - y));
        assert((bx * by).toString() == std::to_string(x * y));
        if (y != 0) {
            assert((bx % by).toString() == std::to_string(x % y));
        }
    }
}

int main() {
    checkRoundtrip("0", "0");
    checkRoundtrip("-0", "0");
    checkRoundtrip("+0", "0");
    checkRoundtrip("000123", "123");
    checkRoundtrip("-000123", "-123");
    checkRoundtrip("   42  ", "42");

    testExamples();
    testSigns();
    testRandom();

    std::cout << "bigint tests passed\n";
    return 0;
}
