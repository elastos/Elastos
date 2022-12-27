////////////////////////////////////////////////////////////////////////////////
//
// BigInt.h
//
// Copyright (c) 2011 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef __ELASTOS_SDK_BIGINT_H__
#define __ELASTOS_SDK_BIGINT_H__

#include "typedefs.h"
#include <openssl/crypto.h>
#include <openssl/bn.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace Elastos {
    namespace ElaWallet {

        class BigInt
        {
        protected:
            BIGNUM* bn;
            BN_CTX* ctx;
            bool autoclear;

            void allocate();

        public:
            // Allocation & Assignment
            BigInt();

            BigInt(const BigInt& bigint);

            BigInt(uint64_t num);

            BigInt(const std::vector<unsigned char>& bytes, bool bigEndian = false);

            BigInt(const std::string& inBase, unsigned int base = 16, const char* alphabet = "0123456789abcdef");

            ~BigInt();

            void setAutoclear(bool autoclear = true);

            void clear();

            // Assignment operations
            //BigInt& operator=(BN_ULONG rhs) { if (!BN_set_word(this->bn, rhs)) throw std::runtime_error("BIGNUM Error."); return *this; }
            BigInt& operator=(const BigInt& bigint);

            BigInt& operator=(uint64_t num);

            // Arithmetic Operations
            BigInt& operator+=(const BigInt& rhs);
            BigInt& operator-=(const BigInt& rhs);
            BigInt& operator*=(const BigInt& rhs);
            BigInt& operator/=(const BigInt& rhs);
            BigInt& operator%=(const BigInt& rhs);

            BigInt& operator+=(uint64_t rhs);
            BigInt& operator-=(uint64_t rhs);
            BigInt& operator*=(uint64_t rhs);
            BigInt& operator/=(uint64_t rhs);
            BigInt& operator%=(uint64_t rhs);

            BigInt operator+(const BigInt& rightOperand) const;
            BigInt operator-(const BigInt& rightOperand) const;
            BigInt operator*(const BigInt& rightOperand) const;
            BigInt operator/(const BigInt& rightOperand) const;
            BigInt operator%(const BigInt& rightOperand) const;

            BigInt operator+(uint64_t rightOperand) const;
            BigInt operator-(uint64_t rightOperand) const;
            BigInt operator*(uint64_t rightOperand) const;
            BigInt operator/(uint64_t rightOperand) const;
            BigInt operator%(uint64_t rightOperand) const;

            // Bitshift Operators
            BigInt& operator<<=(int rhs);
            BigInt& operator>>=(int rhs);

            BigInt operator<<(int rhs) const;
            BigInt operator>>(int rhs) const;

            // Comparison Operators
            bool operator==(const BigInt& rhs) const;
            bool operator!=(const BigInt& rhs) const;
            bool operator<(const BigInt& rhs) const;
            bool operator>(const BigInt& rhs) const;
            bool operator<=(const BigInt& rhs) const;
            bool operator>=(const BigInt& rhs) const;
            bool isZero() const;

            uint64_t getUint64() const;

            void setUint64(uint64_t num);

            // Accessor Methods
#if 0
            BN_ULONG getWord() const;
            void setWord(BN_ULONG num);
#endif

            void setRaw(BIGNUM *bn);

            bytes_t getHexBytes(bool littleEndian = false) const;

            void setHexBytes(bytes_t bytes, bool littleEndian = false);

            bytes_t getBytes(bool bigEndian = false) const;

            void setBytes(bytes_t bytes, bool bigEndian = false);

            int numBytes() const;

            std::string getHex() const;

            void setHex(const std::string& hex);

            void SetHex(const std::string& hex);

            std::string getDec() const;

            void setDec(const std::string& dec);

            std::string getInBase(unsigned int base, const char* alphabet) const;

            void setInBase(const std::string& inBase, unsigned int base, const char* alphabet);
        };

    }
}

#endif // __ELASTOS_SDK_BIGINT_H__
