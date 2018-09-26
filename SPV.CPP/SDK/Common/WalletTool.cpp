// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
//#include <iconv.h>
#include "Utils.h"

#include "WalletTool.h"
#include "BTCBase58.h"
#include "BigIntFormat.h"
#include "BRCrypto.h"
#include "BRBIP39Mnemonic.h"

namespace Elastos {
	namespace ElaWallet {
		CMemBlock<char>
		WalletTool::_Code_Convert(const std::string from_charset, const std::string to_charset,
								   const CMemBlock<char> &input) {
			CMemBlock<char> output;
			if (true == input) {
//				iconv_t cd;
//				char **pin = &input;
//				static char sc_tmp[MAX_BUFFER] = {0};
//				char *out[1] = {sc_tmp};
//				size_t inlen = input.GetSize(), outlen = MAX_BUFFER;
//
//				cd = iconv_open(to_charset.c_str(), from_charset.c_str());
//				if (nullptr != cd) {
//					if (-1 != iconv(cd, pin, &inlen, &out[0], &outlen) && 0 < outlen) {
//						output.Resize(size_t(MAX_BUFFER - outlen));
//						memcpy(output, sc_tmp, output.GetSize());
//					}
//					iconv_close(cd);
//				}
			}

			return output;
		}

		CMBlock WalletTool::GetRandom(size_t bits) {
			size_t bytes = 0 == bits % 8 ? bits / 8 : bits / 8 + 1;
			CMBlock out(bytes);
			for (size_t i = 0; i < bytes; i++) {
				out[i] = Utils::getRandomByte();
			}
			return out;
		}

		CMemBlock<char> WalletTool::U8ToU16(const CMemBlock<char> &input) {
			return _Code_Convert("utf-8", "utf-16", input);
		}

		CMemBlock<char> WalletTool::U16ToU8(const CMemBlock<char> &input) {
			return _Code_Convert("utf-16", "utf-8", input);
		}

		CMBlock WalletTool::GenerateSeed128() {
			return GetRandom(128);
		}

		std::string WalletTool::GenerateEntropySource(const CMBlock &seed) {
			std::string out = "";
			if (true == seed) {
				uint8_t t[32];
				BRSHA256_2(t, seed, seed.GetSize());
				CMBlock tmp;
				tmp.SetMemFixed(t, sizeof(t));
				CMemBlock<char> cb_entropy = Hex2Str(tmp);
				out = (const char *) cb_entropy;
			}
			return out;
		}

		std::string WalletTool::getDeriveKeyFromEntropySource_base58(const std::string &seed) {
			std::string out = "";
			if (seed != "") {
				CMemBlock<char> cbSeed;
				cbSeed.SetMemFixed(seed.c_str(), seed.size() + 1);
				CMBlock entropySource = Str2Hex(cbSeed);
				if (true == entropySource) {
					uint8_t key64[64] = {0};
					uint8_t key[] = "reqPrivKey";
					BRHMAC(key64, BRSHA512, 512 / 8, key, strlen((const char *) key), entropySource,
						   entropySource.GetSize());
					CMBlock tmp;
					tmp.SetMemFixed(key64, sizeof(key64));
					out = BTCBase58::EncodeBase58((unsigned char *) key64, sizeof(key64));
				}
			}
			return out;
		}

		std::string WalletTool::getStringFromSeed(const CMBlock &seed) {
			std::string out = "";
			if (true == seed) {
				CMBlock seeds;
				seeds.SetMemFixed(seed, seed.GetSize());
				CMemBlock<char> str = Hex2Str(seeds);
				out = (const char *) str;
			}
			return out;
		}

		CMBlock WalletTool::getSeedFromString(const std::string &str_seed) {
			CMBlock out;
			if (str_seed != "") {
				CMemBlock<char> str_chg;
				str_chg.SetMemFixed(str_seed.c_str(), str_seed.size() + 1);
				out = Str2Hex(str_chg);
			}
			return out;
		}

		CMemBlock<char>
		WalletTool::GeneratePhraseFromSeed(const CMBlock &seed, const std::vector<std::string> &WordList) {
			CMemBlock<char> out;
			if (true == seed && 0 < WordList.size()) {
				const char *wordList[WordList.size()];
				memset(wordList, 0, sizeof(wordList));
				for (size_t i = 0; i < WordList.size(); i++) {
					wordList[i] = WordList[i].c_str();
				}
				size_t phraselen = BRBIP39Encode(nullptr, 0, wordList, seed, seed.GetSize());
				out.Resize(phraselen);
				phraselen = BRBIP39Encode(out, phraselen, wordList, seed, seed.GetSize());
			}
			return out;
		}

		bool WalletTool::PhraseIsValid(const CMemBlock<char> &phrase, const std::vector<std::string> &WordList) {
			bool out = false;
			if (true == phrase && 0 < WordList.size()) {
				const char *wordList[WordList.size()];
				memset(wordList, 0, sizeof(wordList));
				for (size_t i = 0; i < WordList.size(); i++) {
					wordList[i] = WordList[i].c_str();
				}
				out = 1 == BRBIP39PhraseIsValid(wordList, phrase) ? true : false;
			}
			return out;
		}

		CMBlock
		WalletTool::getSeedFromPhrase(const CMemBlock<char> &phrase, const std::vector<std::string> &WordList) {
			CMBlock out;
			if (true == phrase && 0 < WordList.size()) {
				const char *wordList[WordList.size()];
				memset(wordList, 0, sizeof(wordList));
				for (size_t i = 0; i < WordList.size(); i++) {
					wordList[i] = WordList[i].c_str();
				}
				size_t datalen = BRBIP39Decode(nullptr, 0, wordList, phrase);
				out.Resize(datalen);
				datalen = BRBIP39Decode(out, datalen, wordList, phrase);
			}
			return out;
		}

		std::string WalletTool::getDeriveKey_base58(const CMemBlock<char> &phrase, const std::string &passphrase) {
			uint8_t prikey[64] = {0};
			BRBIP39DeriveKey(prikey, phrase, passphrase.c_str());
			return BTCBase58::EncodeBase58((unsigned char *) prikey, 64);
		}
	}
}