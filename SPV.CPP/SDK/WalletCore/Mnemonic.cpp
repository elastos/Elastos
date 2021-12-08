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

#include "Mnemonic.h"
#include <utf8proc.h>

#include <Common/hash.h>
#include <Common/ErrorChecker.h>
#include <Common/uint256.h>
#include <Common/Utils.h>

#include <sstream>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

namespace Elastos {
    namespace ElaWallet {

        Mnemonic::Mnemonic() {
        }

        WordList Mnemonic::Create(const bytes_t &entropy, const Dictionary &lexicon) {
            if ((entropy.size() % MnemonicSeedMultiple) != 0)
                return {};

            const size_t entropyBits = (entropy.size() * ByteBits);
            const size_t checkBits = (entropyBits / EntropyBitDivisor);
            const size_t totalBits = (entropyBits + checkBits);
            const size_t wordCount = (totalBits / BitsPerWord);

            if ((totalBits % BitsPerWord) != 0 || (wordCount % MnemonicWordMultiple) != 0)
                return {};

            bytes_t data = entropy + sha256(entropy);

            size_t bit = 0;
            WordList words;

            for (size_t word = 0; word < wordCount; word++) {
                size_t position = 0;
                for (size_t loop = 0; loop < BitsPerWord; loop++) {
                    bit = (word * BitsPerWord + loop);
                    position <<= 1;

                    const auto byte = bit / ByteBits;

                    if ((data[byte] & Bip39Shift(bit)) > 0)
                        position++;
                }

                if (position >= DictionarySize)
                    return {};
                words.push_back(lexicon[position]);
            }

            if (words.size() != ((bit + 1) / BitsPerWord))
                return {};

            return words;
        }

        bytes_t Mnemonic::Entropy(const WordList &words, const Dictionary &lexicon) {
            const auto wordCount = words.size();
            if ((wordCount % MnemonicWordMultiple) != 0)
                return {};

            const auto totalBits = BitsPerWord * wordCount;
            const auto checkBits = totalBits / (EntropyBitDivisor + 1);
            const auto entropyBits = totalBits - checkBits;

            if ((entropyBits % ByteBits) != 0)
                return {};

            size_t bit = 0;
            bytes_t data((totalBits + ByteBits - 1) / ByteBits, 0);

            for (const auto& word: words) {
                const auto position = FindPosition(lexicon, word);
                if (position == -1)
                    return {};

                for (size_t loop = 0; loop < BitsPerWord; loop++, bit++)
                {
                    if (position & (1 << (BitsPerWord - loop - 1)))
                    {
                        const auto byte = bit / ByteBits;
                        data[byte] |= Bip39Shift(bit);
                    }
                }
            }

            data.resize(entropyBits / ByteBits);
            return data;
        }

        bool Mnemonic::Validate(const WordList &words, const Dictionary &lexicon) {
            bytes_t entropy = Entropy(words, lexicon);
            if (entropy.empty())
                return false;

            const auto mnemonic = Create(entropy, lexicon);
            return std::equal(mnemonic.begin(), mnemonic.end(), words.begin());
        }

        bool Mnemonic::Validate(const WordList &words, const DictionaryMap &lexicons) {
            for (const auto &lexicon : lexicons) {
                if (Validate(words, *lexicon.second))
                    return true;
            }

            return false;
        }

        std::string Mnemonic::Create(const std::string &language, WordCount wordCount) {
            std::string languageLowerCase = language;
            size_t entropyBits = 0;

            switch (wordCount) {
                case WORDS_12: entropyBits = 128; break;
                case WORDS_15: entropyBits = 160; break;
                case WORDS_18: entropyBits = 192; break;
                case WORDS_21: entropyBits = 224; break;
                case WORDS_24: entropyBits = 256; break;
                default:
                    ErrorChecker::ThrowParamException(Error::InvalidMnemonicWordCount, "invalid mnemonic word count");
            }

            std::transform(languageLowerCase.begin(), languageLowerCase.end(), languageLowerCase.begin(), ::tolower);
            bytes_t entropy = Utils::GetRandom(entropyBits / 8);

            auto it = Language::All.find(languageLowerCase);
            if (it == Language::All.end())
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid mnemonic language");

            WordList words = Create(entropy, *it->second);
            return boost::algorithm::join(words, " ");
        }

        bytes_t Mnemonic::Entropy(const std::string &mnemonic, const Dictionary &lexicon) {
            utf8proc_uint8_t *mnemonicTmp = utf8proc_NFKD((const utf8proc_uint8_t *)mnemonic.c_str());
            std::string mnemonicFixed((char *)mnemonicTmp);
            free(mnemonicTmp);

            WordList words;
            boost::algorithm::split(words, mnemonicFixed, boost::is_any_of(" \n\r\t"), boost::token_compress_on);
            words.erase(std::remove(words.begin(), words.end(), ""), words.end());

            return Entropy(words, lexicon);
        }

        bool Mnemonic::Validate(const std::string &mnemonic) {
            utf8proc_uint8_t *mnemonicTmp = utf8proc_NFKD((const utf8proc_uint8_t *)mnemonic.c_str());
            std::string mnemonicFixed((char *)mnemonicTmp);
            free(mnemonicTmp);

            WordList words;
            boost::algorithm::split(words, mnemonicFixed, boost::is_any_of(" \n\r\t"), boost::token_compress_on);
            words.erase(std::remove(words.begin(), words.end(), ""), words.end());

            return Validate(words);
        }

        uint512 Mnemonic::DeriveSeed(const std::string &mnemonic, const std::string &passphrase) {
            utf8proc_uint8_t *mnemonicTmp = utf8proc_NFKD((const utf8proc_uint8_t *)mnemonic.c_str());
            std::string mnemonicFixed((char *)mnemonicTmp);
            free(mnemonicTmp);

            utf8proc_uint8_t *passphraseTmp = utf8proc_NFKD((const utf8proc_uint8_t *)passphrase.c_str());
            std::string passphraseFixed((char *)passphraseTmp);
            free(passphraseTmp);

            WordList words;
            boost::algorithm::split(words, mnemonicFixed, boost::is_any_of(" \n\r\t"), boost::token_compress_on);
            words.erase(std::remove(words.begin(), words.end(), ""), words.end());

            ErrorChecker::CheckLogic(!Validate(words), Error::Mnemonic, "invalid mnemonic");

            std::string sentence = boost::algorithm::join(words, " ");
            std::string salt = "mnemonic" + passphraseFixed;

            return PBKDF2(bytes_t(sentence.data(), sentence.size()), bytes_t(salt.data(), salt.size()), 2048);
        }

        uint512 Mnemonic::PBKDF2(const bytes_t &pw, const bytes_t &salt, unsigned int rounds) {
            bytes_t s(salt.size() + sizeof(uint32_t));
            uint32_t i, j;
            uint512 key;
            size_t length, keyLen = key.size();
            bytes_t U, T, k;

            assert(rounds > 0);

            memcpy(s.data(), salt.data(), salt.size());

            for (i = 0; keyLen > 0; i++) {
                s[salt.size() + 0] = (uint8_t)(((i + 1) >> 24) & 0xff);
                s[salt.size() + 1] = (uint8_t)(((i + 1) >> 16) & 0xff);
                s[salt.size() + 2] = (uint8_t)(((i + 1) >> 8) & 0xff);
                s[salt.size() + 3] = (uint8_t)((i + 1) & 0xff);

                U = hmac_sha512(pw, s); // U1 = hmac_hash(pw, salt || be32(i))
                T = U;

                for (unsigned int r = 1; r < rounds; r++) {
                    U = hmac_sha512(pw, U); // Urounds = hmac_hash(pw, Urounds-1)
                    for (j = 0; j < T.size(); j++) T[j] ^= U[j]; // Ti = U1 ^ U2 ^ ... ^ Urounds
                }

                // dk = T1 || T2 || ... || Tdklen/hlen
                length = keyLen < T.size() ? keyLen : T.size();
                k += T;
                keyLen -= length;
            }

            key = uint512(k);

            s.clean();
            U.clean();
            T.clean();
            k.clean();

            return key;
        }

    }
}