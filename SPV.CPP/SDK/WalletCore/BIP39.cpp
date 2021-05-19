// Created by Aaron Voisine on 9/7/15.
// Copyright (c) 2015 breadwallet LLC
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#include "BIP39.h"

#include <Common/hash.h>
#include <Common/ErrorChecker.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		uint512 BIP39::PBKDF2(const bytes_t &pw, const bytes_t &salt, unsigned int rounds) {
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

		uint512 BIP39::DeriveSeed(const std::string &mnemonic, const std::string &passphrase) {
			std::vector<std::string> wordList;
			boost::algorithm::split(wordList, mnemonic, boost::is_any_of(" \n\r\t"), boost::token_compress_on);

			wordList.erase(std::remove(wordList.begin(), wordList.end(), ""), wordList.end());

			ErrorChecker::CheckLogic(wordList.size() % 3 != 0, Error::Mnemonic,
									 "invalid mnemonic word count = " + std::to_string(wordList.size()));

			std::string sentence = boost::algorithm::join(wordList, " ");
			std::string salt = "mnemonic" + passphrase;

			return PBKDF2(bytes_t(sentence.data(), sentence.size()), bytes_t(salt.data(), salt.size()), 2048);
		}

		std::string BIP39::Encode(const std::vector<std::string> &dictionary, const bytes_t &entropy) {
			uint32_t x;
			std::string word, mnemonic;
			size_t i;
			bytes_t buf = entropy;

			assert(entropy.size() > 0 && (entropy.size() % 4) == 0);

			if ((entropy.size() % 4) != 0)
				return std::string(); // data length must be a multiple of 32 bits

			buf += sha256(buf); // append SHA256 checksum

			for (i = 0; i < entropy.size()*3/4; i++) {
				x = ((uint32_t)buf[i * 11 / 8] << 24);
				x |= ((uint32_t)buf[i * 11 / 8 + 1] << 16);
				x |= ((uint32_t)buf[i * 11 / 8 + 2] << 8);
				x |= ((uint32_t)buf[i * 11 / 8 + 3]);
				word = dictionary[(x >> (32 - (11 + ((i*11) % 8)))) % BIP39_WORDLIST_COUNT];
				if (i > 0)
					mnemonic += " ";
				mnemonic += word;
			}

			x = 0;
			buf.clean();
			return mnemonic;
		}

		bytes_t BIP39::Decode(const std::vector<std::string> &dictionary, const std::string &mnemonic) {
			uint32_t x, y, count = 0, idx[24], i;
			uint8_t b = 0;
			bytes_t entropy;
			std::vector<std::string> words;

			boost::algorithm::split(words, mnemonic, boost::is_any_of(" \n\r\t"), boost::token_compress_on);
			words.erase(std::remove(words.begin(), words.end(), ""), words.end());

			for (const auto &word: words) {
				for (i = 0; i < dictionary.size(); ++i) {
					std::string dictWord = dictionary[i];

					dictWord.erase(std::remove_if(dictWord.begin(), dictWord.end(), [](char &c) {
						return c == '\t' || c == '\r' || c == ' ' || c == '\n';
					}), dictWord.end());

					if (dictWord == word) {
						idx[count++] = i;
						break;
					}
				}

				if (i >= dictionary.size()) {
					return bytes_t();
				}
			}

			if ((count % 3) == 0) { // check that phrase has correct number of words
				uint8_t buf[(count*11 + 7)/8];

				for (i = 0; i < (count*11 + 7)/8; i++) {
					x = idx[i*8/11];
					y = (i*8/11 + 1 < count) ? idx[i*8/11 + 1] : 0;
					b = ((x*BIP39_WORDLIST_COUNT + y) >> ((i*8/11 + 2)*11 - (i + 1)*8)) & 0xff;
					buf[i] = b;
				}

				bytes_t hash = sha256(bytes_t(buf, count * 4 / 3));

				if (b >> (8 - count/3) == (hash[0] >> (8 - count/3))) { // verify checksum
					entropy = bytes_t(buf, count * 4 / 3);
				}

				memset(buf, 0, sizeof(buf));
			}

			b = 0;
			x = y = 0;
			memset(idx, 0, sizeof(idx));
			return entropy;
		}

	}
}