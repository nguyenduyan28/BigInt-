#include "BigInt.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>

BigInt::BigInt() : sign_(0) {}

BigInt::BigInt(int64_t v) : sign_(0) {
    if (v == 0) {
        return;
    }
    sign_ = (v < 0) ? -1 : 1;
    uint64_t value = (v < 0) ? static_cast<uint64_t>(-v) : static_cast<uint64_t>(v);
    while (value > 0) {
        limbs_.push_back(static_cast<uint32_t>(value % BASE));
        value /= BASE;
    }
}

BigInt BigInt::fromString(std::string_view text) {
    size_t start = 0;
    size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    if (start == end) {
        throw std::invalid_argument("empty integer");
    }

    int sign = 1;
    size_t pos = start;
    if (text[pos] == '+' || text[pos] == '-') {
        sign = (text[pos] == '-') ? -1 : 1;
        ++pos;
        if (pos >= end) {
            throw std::invalid_argument("expected digits after sign");
        }
    }
    if (!std::isdigit(static_cast<unsigned char>(text[pos]))) {
        throw std::invalid_argument("invalid digit in integer");
    }

    for (size_t i = pos; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            throw std::invalid_argument("invalid digit in integer");
        }
    }

    std::string digits(text.substr(pos, end - pos));
    size_t non_zero = digits.find_first_not_of('0');
    if (non_zero == std::string::npos) {
        return BigInt();
    }
    digits = digits.substr(non_zero);

    BigInt result;
    result.sign_ = sign;
    for (size_t i = digits.size(); i > 0;) {
        size_t chunk_start = (i >= 9) ? i - 9 : 0;
        size_t len = i - chunk_start;
        uint32_t limb = static_cast<uint32_t>(std::stoul(digits.substr(chunk_start, len)));
        result.limbs_.push_back(limb);
        if (chunk_start == 0) {
            break;
        }
        i -= len;
    }
    result.normalize();
    return result;
}

std::string BigInt::toString() const {
    if (sign_ == 0) {
        return "0";
    }
    std::string out;
    if (sign_ < 0) {
        out.push_back('-');
    }
    out += std::to_string(limbs_.empty() ? 0 : limbs_.back());
    for (size_t i = limbs_.size(); i-- > 0;) {
        if (i == limbs_.size() - 1) {
            continue; // highest limb already printed
        }
        std::string chunk = std::to_string(limbs_[i]);
        out.append(9 - chunk.size(), '0');
        out += chunk;
    }
    return out;
}

void BigInt::normalize() {
    while (!limbs_.empty() && limbs_.back() == 0) {
        limbs_.pop_back();
    }
    if (limbs_.empty()) {
        sign_ = 0;
    }
}

int BigInt::cmpAbs(const BigInt& a, const BigInt& b) {
    if (a.limbs_.size() != b.limbs_.size()) {
        return (a.limbs_.size() < b.limbs_.size()) ? -1 : 1;
    }
    for (size_t i = a.limbs_.size(); i-- > 0;) {
        if (a.limbs_[i] != b.limbs_[i]) {
            return (a.limbs_[i] < b.limbs_[i]) ? -1 : 1;
        }
    }
    return 0;
}

int BigInt::cmp(const BigInt& a, const BigInt& b) {
    if (a.sign_ != b.sign_) {
        return (a.sign_ < b.sign_) ? -1 : 1;
    }
    if (a.sign_ == 0) {
        return 0;
    }
    int abs_cmp = cmpAbs(a, b);
    return (a.sign_ > 0) ? abs_cmp : -abs_cmp;
}

BigInt BigInt::addAbs(const BigInt& a, const BigInt& b) {
    BigInt result;
    result.sign_ = 1;
    const size_t n = std::max(a.limbs_.size(), b.limbs_.size());
    result.limbs_.resize(n);
    uint64_t carry = 0;
    for (size_t i = 0; i < n; ++i) {
        uint64_t sum = carry;
        if (i < a.limbs_.size()) {
            sum += a.limbs_[i];
        }
        if (i < b.limbs_.size()) {
            sum += b.limbs_[i];
        }
        result.limbs_[i] = static_cast<uint32_t>(sum % BASE);
        carry = sum / BASE;
    }
    if (carry != 0) {
        result.limbs_.push_back(static_cast<uint32_t>(carry));
    }
    result.normalize();
    return result;
}

BigInt BigInt::subAbs(const BigInt& a, const BigInt& b) {
    BigInt result;
    result.sign_ = 1;
    result.limbs_.resize(a.limbs_.size());
    int64_t borrow = 0;
    for (size_t i = 0; i < a.limbs_.size(); ++i) {
        int64_t diff = static_cast<int64_t>(a.limbs_[i]) - borrow;
        if (i < b.limbs_.size()) {
            diff -= b.limbs_[i];
        }
        if (diff < 0) {
            diff += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.limbs_[i] = static_cast<uint32_t>(diff);
    }
    result.normalize();
    return result;
}

BigInt BigInt::mulAbs(const BigInt& a, const BigInt& b) {
    if (a.sign_ == 0 || b.sign_ == 0) {
        return BigInt();
    }
    BigInt result;
    result.sign_ = 1;
    result.limbs_.assign(a.limbs_.size() + b.limbs_.size() + 1, 0);
    for (size_t i = 0; i < a.limbs_.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b.limbs_.size(); ++j) {
            uint64_t cur = result.limbs_[i + j] +
                           static_cast<uint64_t>(a.limbs_[i]) * b.limbs_[j] + carry;
            result.limbs_[i + j] = static_cast<uint32_t>(cur % BASE);
            carry = cur / BASE;
        }
        size_t k = i + b.limbs_.size();
        uint64_t cur = result.limbs_[k] + carry;
        result.limbs_[k] = static_cast<uint32_t>(cur % BASE);
        uint64_t carry2 = cur / BASE;
        while (carry2 != 0) {
            ++k;
            if (k >= result.limbs_.size()) {
                result.limbs_.push_back(0);
            }
            uint64_t cur2 = result.limbs_[k] + carry2;
            result.limbs_[k] = static_cast<uint32_t>(cur2 % BASE);
            carry2 = cur2 / BASE;
        }
    }
    result.normalize();
    return result;
}

BigInt BigInt::mulAbsByUint(const BigInt& a, uint32_t m) {
    if (a.sign_ == 0 || m == 0) {
        return BigInt();
    }
    BigInt result;
    result.sign_ = 1;
    result.limbs_.resize(a.limbs_.size());
    uint64_t carry = 0;
    for (size_t i = 0; i < a.limbs_.size(); ++i) {
        uint64_t cur = static_cast<uint64_t>(a.limbs_[i]) * m + carry;
        result.limbs_[i] = static_cast<uint32_t>(cur % BASE);
        carry = cur / BASE;
    }
    if (carry != 0) {
        result.limbs_.push_back(static_cast<uint32_t>(carry));
    }
    result.normalize();
    return result;
}

BigInt BigInt::absValue(const BigInt& v) {
    BigInt res = v;
    if (res.sign_ < 0) {
        res.sign_ = 1;
    }
    return res;
}

BigInt BigInt::modAbs(const BigInt& a, const BigInt& b) {
    if (b.sign_ == 0) {
        throw std::runtime_error("mod by zero");
    }
    BigInt dividend = absValue(a);
    BigInt divisor = absValue(b);

    if (cmpAbs(dividend, divisor) < 0) {
        return dividend;
    }

    size_t shift = dividend.limbs_.size() - divisor.limbs_.size();
    BigInt divisorShifted = divisor;
    divisorShifted.limbs_.insert(divisorShifted.limbs_.begin(), shift, 0);

    BigInt remainder = dividend;
    for (size_t i = shift + 1; i-- > 0;) {
        uint32_t low = 0;
        uint32_t high = BASE - 1;
        uint32_t best = 0;
        while (low <= high) {
            uint32_t mid = low + (high - low) / 2;
            BigInt prod = mulAbsByUint(divisorShifted, mid);
            int cmp_result = cmpAbs(prod, remainder);
            if (cmp_result <= 0) {
                best = mid;
                low = mid + 1;
            } else {
                if (mid == 0) {
                    break;
                }
                high = mid - 1;
            }
        }
        if (best != 0) {
            remainder = remainder - mulAbsByUint(divisorShifted, best);
        }
        if (!divisorShifted.limbs_.empty()) {
            divisorShifted.limbs_.erase(divisorShifted.limbs_.begin());
        }
    }
    remainder.normalize();
    return remainder;
}

BigInt BigInt::operator-() const {
    BigInt result(*this);
    if (result.sign_ != 0) {
        result.sign_ = -result.sign_;
    }
    return result;
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (sign_ == 0) {
        return other;
    }
    if (other.sign_ == 0) {
        return *this;
    }
    if (sign_ == other.sign_) {
        BigInt sum = addAbs(*this, other);
        sum.sign_ = sign_;
        sum.normalize();
        return sum;
    }
    int cmp_result = cmpAbs(*this, other);
    if (cmp_result == 0) {
        return BigInt();
    }
    if (cmp_result > 0) {
        BigInt diff = subAbs(*this, other);
        diff.sign_ = sign_;
        diff.normalize();
        return diff;
    }
    BigInt diff = subAbs(other, *this);
    diff.sign_ = other.sign_;
    diff.normalize();
    return diff;
}

BigInt BigInt::operator-(const BigInt& other) const {
    return *this + (-other);
}

BigInt BigInt::operator*(const BigInt& other) const {
    if (sign_ == 0 || other.sign_ == 0) {
        return BigInt();
    }
    BigInt product = mulAbs(*this, other);
    product.sign_ = sign_ * other.sign_;
    product.normalize();
    return product;
}

BigInt BigInt::operator%(const BigInt& other) const {
    if (other.sign_ == 0) {
        throw std::runtime_error("mod by zero");
    }
    if (sign_ == 0) {
        return BigInt();
    }
    BigInt remainder = modAbs(*this, other);
    if (remainder.sign_ != 0 && sign_ < 0) {
        remainder.sign_ = -1;
    }
    remainder.normalize();
    return remainder;
}

bool BigInt::operator==(const BigInt& other) const {
    return sign_ == other.sign_ && limbs_ == other.limbs_;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}
