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

	}
}