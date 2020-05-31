#pragma once

#include "common.hpp"

// No bounds checking
class Bitset {
public:
    static constexpr int WordCount = 2;
    static constexpr int BitCount = 64 * WordCount;

    Bitset() {}
    static Bitset empty() {
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = 0;
        }
        return ret;
    }
    static Bitset range(int n) {
        Bitset ret;
        int s = n >> 6;
        for(int w = 0; w < s; ++w) {
            ret.words_[w] = (uint64_t)-1;
        }
        if(n & 63) {
            ret.words_[s] = ((uint64_t)1 << (n & 63)) - (uint64_t)1;
            for(int w = s + 1; w < WordCount; ++w) {
                ret.words_[w] = 0;
            }
        } else {
            for(int w = s; w < WordCount; ++w) {
                ret.words_[w] = 0;
            }
        }
        return ret;
    }
    static Bitset singleton(int i) {
        Bitset ret = Bitset::empty();
        ret.add(i);
        return ret;
    }

    void add(int i) {
        words_[i >> 6] |= (uint64_t)1 << (i & 63);
    }
    void del(int i) {
        words_[i >> 6] &= ~((uint64_t)1 << (i & 63));
    }

    bool contains(int i) const {
        return (bool)(words_[i >> 6] & ((uint64_t)1 << (i & 63)));
    }
    bool isEmpty() const {
        for(int w = 0; w < WordCount; ++w) {
            if(words_[w]) {
                return false;
            }
        }
        return true;
    }

    int count() const {
        int ret = 0;
        for(int w = 0; w < WordCount; ++w) {
            ret += __builtin_popcountll(words_[w]);
        }
        return ret;
    }

    int min() const {
        for(int w = 0; w < WordCount; ++w) {
            if(words_[w]) {
                return (w << 6) + __builtin_ctzll(words_[w]);
            }
        }
        return -1;
    }

    template <typename F>
    bool iterateWhile(F f) const {
        Bitset left = *this;
        for(int w = 0; w < WordCount; ++w) {
            while(left.words_[w]) {
                int b = __builtin_ctzll(left.words_[w]);
                left.words_[w] ^= (uint64_t)1 << b;
                if(!f((const int)((w << 6) + b))) {
                    return false;
                }
            }
        }
        return true;
    }
    template <typename F>
    void iterate(F f) const {
        iterateWhile([&](int b) {
            f((const int)b);
            return true;
        });
    }

    template <typename F>
    bool iterateSubsetsWhile(F f) {
        Bitset subset = Bitset::empty();
        while(true) {
            if(!f((const Bitset)subset)) {
                return false;
            }
            if(subset == *this) {
                return true;
            }
            subset = subtract_(subset, *this).intersectWith(*this);
        }
    }

    template <typename F>
    bool iterateSubsetsOfSizeWhile(int size, F f) {
        int c = count();
        if(size > c) {
            return true;
        }
        Bitset currentSet = range(size);
        Bitset lastSet = range(c).minus(range(c - size));
        while(true) {
            Bitset X = pdep_(currentSet, *this);
            if(!f((const Bitset&)X)) {
                return false;
            }
            if(currentSet == lastSet) {
                break;
            }
            Bitset x = currentSet.unionWith(dec_(currentSet));
            currentSet = inc_(x).unionWith(
                shiftRight_(
                    dec_((complement_(x).intersectWith(inc_(x)))),
                    currentSet.min() + 1
                )
            );
        }
        return true;
    }

    Bitset with(int i) const {
        Bitset ret = *this;
        ret.add(i);
        return ret;
    }
    Bitset without(int i) const {
        Bitset ret = *this;
        ret.del(i);
        return ret;
    }

    Bitset minus(Bitset X) const {
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = words_[w] & ~X.words_[w];
        }
        return ret;
    }
    Bitset intersectWith(Bitset X) const {
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = words_[w] & X.words_[w];
        }
        return ret;
    }
    Bitset unionWith(Bitset X) const {
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = words_[w] | X.words_[w];
        }
        return ret;
    }

    bool isSubsetOf(Bitset X) const {
        return minus(X).isEmpty();
    }

    bool operator==(Bitset other) const {
        for(int w = 0; w < WordCount; ++w) {
            if(words_[w] != other.words_[w]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(Bitset other) const {
        return !(*this == other);
    }

private:
    uint64_t words_[WordCount];

    static Bitset complement_(Bitset a) {
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = ~a.words_[w];
        }
        return ret;
    }
    static Bitset subtract_(Bitset a, Bitset b) {
        bool carry = true;
        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            if(carry) {
                ret.words_[w] = a.words_[w] + ~b.words_[w] + (uint64_t)1;
                carry = ret.words_[w] <= a.words_[w];
            } else {
                ret.words_[w] = a.words_[w] + ~b.words_[w];
                carry = ret.words_[w] < a.words_[w];
            }
        }
        return ret;
    }
    static Bitset inc_(Bitset x) {
        Bitset ret = x;
        for(int w = 0; w < WordCount; ++w) {
            ++ret.words_[w];
            if(ret.words_[w]) {
                return ret;
            }
        }
        return ret;
    }
    static Bitset dec_(Bitset x) {
        Bitset ret = x;
        for(int w = 0; w < WordCount; ++w) {
            if(ret.words_[w]) {
                --ret.words_[w];
                return ret;
            } else {
                --ret.words_[w];
            }
        }
        return ret;
    }
    static Bitset shiftRight_(Bitset x, int d) {
        auto read = [&](int i) -> uint64_t {
            if(i < WordCount) {
                return x.words_[i];
            } else {
                return 0;
            }
        };

        int dWords = d >> 6;
        int dBits = d & 63;

        Bitset ret;
        if(dBits == 0) {
            for(int w = 0; w < WordCount; ++w) {
                ret.words_[w] = read(w + dWords);
            }
        } else {
            for(int w = 0; w < WordCount; ++w) {
                ret.words_[w] = (read(w + dWords) >> dBits) | (read(w + 1 + dWords) << (64 - dBits));
            }
        }
        return ret;
    }
    static Bitset pdep_(Bitset src, Bitset mask) {
        uint64_t srcWord = src.words_[0];
        int readWordIdx = 1;
        int readBitIdx = 0;

        auto readMore = [&](int count) {
            if(count == 64) {
                srcWord = 0;
            } else {
                srcWord >>= count;
            }
            int pos = 64 - count;
            while(pos < 64) {
                int chunkSize = ::min(64 - pos, 64 - readBitIdx);
                uint64_t readWord =
                    readWordIdx < WordCount ? src.words_[readWordIdx] : 0;
                if(chunkSize == 64) {
                    srcWord = readWord;
                } else {
                    uint64_t chunk = readWord >> readBitIdx;
                    chunk &= ((uint64_t)1 << chunkSize) - (uint64_t)1;
                    srcWord |= chunk << pos;
                }
                readBitIdx += chunkSize;
                if(readBitIdx == 64) {
                    readBitIdx = 0;
                    ++readWordIdx;
                }
                pos += chunkSize;
            }
        };

        Bitset ret;
        for(int w = 0; w < WordCount; ++w) {
            ret.words_[w] = _pdep_u64(srcWord, mask.words_[w]);
            readMore(__builtin_popcountll(mask.words_[w]));
        }
        return ret;
    }

    friend struct std::hash<Bitset>;
};

namespace std {
    template <>
    struct hash<Bitset> {
        inline size_t operator()(const Bitset& val) const {
            size_t x = 0;
            for(int w = 0; w < Bitset::WordCount; ++w) {
                hashCombine(x, val.words_[w]);
            }
            return x;
        }
    };
}
