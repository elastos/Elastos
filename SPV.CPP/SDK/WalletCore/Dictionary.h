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

#ifndef ELASTOS_SPVSDK_DICTIONARY_H
#define ELASTOS_SPVSDK_DICTIONARY_H

#include <string>
#include <array>
#include <vector>
#include <map>

namespace Elastos {
    namespace ElaWallet {

        /**
         * A valid mnemonic Dictionary has exactly this many words.
         */
#define DictionarySize 2048

        /**
         * Dictionary definitions for creating mnemonics.
         * The bip39 spec calls this a "wordlist".
         * This is a POD type, which means the compiler can write it directly
         * to static memory with no run-time overhead.
         */
        typedef std::array<const char*, DictionarySize> Dictionary;

        /**
         * A collection of candidate dictionaries for mnemonic metadata.
         */
        typedef std::map<std::string, const Dictionary *> DictionaryMap;

        namespace Language {
            // Individual built-in languages:
            extern const Dictionary English;
            extern const Dictionary Spanish;
            extern const Dictionary French;
            extern const Dictionary Italian;
            extern const Dictionary Japanese;
            extern const Dictionary Korean;
            extern const Dictionary ChineseSimplified;
            extern const Dictionary ChineseTraditional;
            extern const Dictionary Czech;
            extern const Dictionary Portuguese;

            // Word lists from:
            // github.com/bitcoin/bips/blob/master/bip-0039/bip-0039-wordlists.md
            const DictionaryMap All {
                    {"english", &English},
                    {"spanish", &Spanish},
                    {"french", &French},
                    {"italian", &Italian},
                    {"japanese", &Japanese},
                    {"korean", &Korean},
                    {"chinesesimplified", &ChineseSimplified},
                    {"chinesetraditional", &ChineseTraditional},
                    {"czech", &Czech},
                    {"portuguese", &Portuguese}
            };
        }

    }
}

#endif //ELASTOS_SPVSDK_DICTIONARY_H
