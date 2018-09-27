// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
//#include <iconv.h>
#include "Utils.h"

#include "WalletTool.h"
#include "BigIntFormat.h"
#include "BRCrypto.h"
#include "BRBIP39Mnemonic.h"

namespace Elastos {
	namespace ElaWallet {

		CMBlock WalletTool::GetRandom(size_t bits) {
			size_t bytes = 0 == bits % 8 ? bits / 8 : bits / 8 + 1;
			CMBlock out(bytes);
			for (size_t i = 0; i < bytes; i++) {
				out[i] = Utils::getRandomByte();
			}
			return out;
		}

		CMBlock WalletTool::GenerateSeed128() {
			return GetRandom(128);
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
		
	}
}