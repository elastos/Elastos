/*
 * Copyright (c) 2021 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_MNEMONICS_H__
#define __ELASTOS_SDK_MNEMONICS_H__

#include "Dictionary.h"
#include <Common/uint256.h>

#include <string>

namespace Elastos {
    namespace ElaWallet {

#define MnemonicSeedMultiple 4
#define ByteBits 8
#define EntropyBitDivisor 32
#define BitsPerWord 11
#define MnemonicWordMultiple 3

        typedef std::vector<std::string> WordList;

        class Mnemonic {
        public:
            enum WordCount {
                WORDS_12 = 12,
                WORDS_15 = 15,
                WORDS_18 = 18,
                WORDS_21 = 21,
                WORDS_24 = 24
            };
        public:
            Mnemonic();

            static WordList Create(const bytes_t &entropy, const Dictionary &lexicon);

            static bytes_t Entropy(const WordList &words, const Dictionary &lexicon);

            static bool Validate(const WordList &words, const Dictionary &lexicon);

            static bool Validate(const WordList &words, const DictionaryMap &lexicons = Language::All);

            static std::string Create(const std::string &language, WordCount wordCount = WORDS_12);

            static bytes_t Entropy(const std::string &mnemonic, const Dictionary &lexicon);

            static bool Validate(const std::string &mnemonic);

            static uint512 DeriveSeed(const std::string &mnemonic, const std::string &passphrase);

        private:
            static inline uint8_t Bip39Shift(size_t bit) {
                return (1 << (ByteBits - (bit % ByteBits) - 1));
            }

            template <typename Element, typename Container>
            static int FindPosition(const Container& list, const Element& value)
            {
                const auto it = std::find(std::begin(list), std::end(list), value);

                if (it == std::end(list))
                    return -1;

                // Unsafe for use with lists greater than max_int32 in size.
                if (list.size() > INT32_MAX)
                    return -1;
                return static_cast<int>(std::distance(list.begin(), it));
            }

            static uint512 PBKDF2(const bytes_t &pw, const bytes_t &salt, unsigned int rounds);

        };

    }
}

#endif //__ELASTOS_SDK_MNEMONICS_H__
