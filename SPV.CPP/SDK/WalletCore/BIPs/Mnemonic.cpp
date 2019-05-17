// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Mnemonic.h"
#include "WordLists/English.h"
#include "BIP39.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/uint256.h>
#include <SDK/Common/Utils.h>

#include <fstream>
#include <sstream>
#include <SDK/Common/hash.h>

#define MNEMONIC_PREFIX "mnemonic_"
#define MNEMONIC_EXTENSION ".txt"
#define DEFAULT_LANGUAGE "english"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		Mnemonic::Mnemonic(const fs::path &rootPath) :
			_rootPath(rootPath) {
		}

		std::string Mnemonic::Create(const std::string &language, WordCount words) const {
			std::string lan = language;
			size_t entropyBits = 0;

			std::transform(lan.begin(), lan.end(), lan.begin(), ::tolower);

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

			if (lan == "english") {
				return BIP39::Encode(EnglistWordLists, entropy);
			}

			fs::path filePath = _rootPath / (MNEMONIC_PREFIX + lan + MNEMONIC_EXTENSION);
			ErrorChecker::CheckLogic(!fs::exists(filePath), Error::Mnemonic, "unsupport language " + language);

			std::vector<std::string> wordLists;

			LoadPath(filePath, wordLists);

			return BIP39::Encode(wordLists, entropy);
		}

		bool Mnemonic::Validate(const std::string &mnemonic) const {
			bool valid = false;
			bytes_t entropy = BIP39::Decode(EnglistWordLists, mnemonic);

			if (entropy.empty()) {
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
			} else {
				valid = true;
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