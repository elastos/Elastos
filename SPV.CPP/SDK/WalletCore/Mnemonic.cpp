// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BIP39.h"
#include "Mnemonic.h"
#include "WordLists/Chinese.h"
#include "WordLists/English.h"
#include "WordLists/French.h"
#include "WordLists/Italian.h"
#include "WordLists/Japanese.h"
#include "WordLists/Spanish.h"

#include <Common/ErrorChecker.h>
#include <Common/uint256.h>
#include <Common/Utils.h>

#include <sstream>
#include <Common/hash.h>

#define MNEMONIC_PREFIX "mnemonic_"
#define MNEMONIC_EXTENSION ".txt"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		Mnemonic::Mnemonic(const fs::path &rootPath) :
			_rootPath(rootPath) {
		}

		std::string Mnemonic::Create(const std::string &language, WordCount words) const {
			std::string languageLowerCase = language;
			size_t entropyBits = 0;

			std::transform(languageLowerCase.begin(), languageLowerCase.end(), languageLowerCase.begin(), ::tolower);

			switch (words) {
				case WORDS_12:
					entropyBits = 128;
					break;

				case WORDS_15:
					entropyBits = 160;
					break;

				case WORDS_18:
					entropyBits = 192;
					break;

				case WORDS_21:
					entropyBits = 224;
					break;

				case WORDS_24:
					entropyBits = 256;
					break;

				default:
					ErrorChecker::ThrowParamException(Error::InvalidMnemonicWordCount, "invalid mnemonic word count");
			}

			bytes_t entropy = Utils::GetRandom(entropyBits / 8);

			if (languageLowerCase == "english") {
				return BIP39::Encode(EnglishWordLists, entropy);
			} else if (languageLowerCase == "chinese") {
				return BIP39::Encode(ChineseWordLists, entropy);
			} else if (languageLowerCase == "french") {
				return BIP39::Encode(FrenchWordLists, entropy);
			} else if (languageLowerCase == "italian") {
				return BIP39::Encode(ItalianWordLists, entropy);
			} else if (languageLowerCase == "japanese") {
				return BIP39::Encode(JapaneseWordLists, entropy);
			} else if (languageLowerCase == "spanish") {
				return BIP39::Encode(SpanishWordLists, entropy);
			}

			fs::path filePath = _rootPath / (MNEMONIC_PREFIX + languageLowerCase + MNEMONIC_EXTENSION);
			ErrorChecker::CheckLogic(!fs::exists(filePath), Error::Mnemonic, "unsupport language " + language);

			std::vector<std::string> wordLists;
			LoadPath(filePath, wordLists);
			return BIP39::Encode(wordLists, entropy);
		}

		bool Mnemonic::Validate(const std::string &mnemonic) const {
			bytes_t entropy = BIP39::Decode(EnglishWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			entropy = BIP39::Decode(ChineseWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			entropy = BIP39::Decode(FrenchWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			entropy = BIP39::Decode(ItalianWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			entropy = BIP39::Decode(JapaneseWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			entropy = BIP39::Decode(SpanishWordLists, mnemonic);
			if (!entropy.empty()) {
				entropy.clean();
				return true;
			}

			bool valid = false;
			std::vector<std::string> wordLists;
			wordLists.reserve(BIP39_WORDLIST_COUNT);
			ErrorChecker::CheckPathExists(_rootPath);

			for (fs::directory_iterator it{_rootPath}; it != fs::directory_iterator{}; ++it) {
				fs::path filePath = *it;
				if (fs::is_regular_file(filePath) &&
					filePath.filename().string().find(MNEMONIC_PREFIX) == 0 &&
					filePath.extension().string() == MNEMONIC_EXTENSION) {
					LoadPath(filePath, wordLists);
					entropy = BIP39::Decode(wordLists, mnemonic);
					if (!entropy.empty()) {
						valid = true;
						break;
					}
				}
			}

			entropy.clean();
			return valid;
		}

		uint512 Mnemonic::DeriveSeed(const std::string &mnemonic, const std::string &passphrase) const {
			ErrorChecker::CheckLogic(!Validate(mnemonic), Error::Mnemonic, "invalid mnemonic");

			return BIP39::DeriveSeed(mnemonic, passphrase);
		}


		void Mnemonic::LoadPath(const fs::path &filePath, std::vector<std::string> &wordLists) const {
			std::fstream in(filePath.string());
			std::string line;

			wordLists.clear();
			wordLists.reserve(BIP39_WORDLIST_COUNT);

			while (std::getline(in, line)) {
				wordLists.push_back(line);
			}

			ErrorChecker::CheckLogic(wordLists.size() != BIP39_WORDLIST_COUNT, Error::Mnemonic, "invalid word lists");
		}

	}
}