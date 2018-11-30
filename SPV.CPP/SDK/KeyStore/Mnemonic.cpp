// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <sstream>
#include <Core/BRInt.h>

#include "BRBIP39WordsEn.h"

#include "Mnemonic.h"

#define MNEMONIC_PREFIX "mnemonic_"
#define MNEMONIC_EXTENSION ".txt"
#define DEFAULT_LANGUAGE "english"
#include "SDK/Common/ParamChecker.h"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		Mnemonic::Mnemonic(const std::string &language, const fs::path &rootPath) :
			_language(language),
			_i18nPath(rootPath) {
			LoadLanguage(language);
		}

		Mnemonic::Mnemonic(const boost::filesystem::path &rootPath) :
			_language(DEFAULT_LANGUAGE),
			_i18nPath(rootPath) {
		}

		void Mnemonic::LoadPath(const boost::filesystem::path &filePath) {
			ParamChecker::checkCondition(!boost::filesystem::exists(filePath),
										 Error::Mnemonic, "load mnemonic: " + filePath.string() + " do not exist!");

			std::fstream infile(filePath.string());
			std::string line;
			while (std::getline(infile, line)) {
				_words.push_back(line);
			}

			ParamChecker::checkCondition(_words.size() != BIP39_WORDLIST_COUNT, Error::Mnemonic,
										 "Mnemonic words count is " + std::to_string(_words.size()) +
										 ", expected " + std::to_string(BIP39_WORDLIST_COUNT));
		}

		std::string Mnemonic::PhraseCheck(const std::string &phrase) {
			const char *wordList[BIP39_WORDLIST_COUNT];
			for (size_t i = 0; i < BIP39_WORDLIST_COUNT; ++i) {
				wordList[i] = _words[i].c_str();
			}

			UInt128 entropy = UINT128_ZERO;
			size_t entropyLen = BRBIP39Decode(entropy.u8, sizeof(entropy), wordList, phrase.c_str());
			if (entropyLen > 0) {
				char standardPhrase[BRBIP39Encode(NULL, 0, wordList, entropy.u8, sizeof(entropy))];
				BRBIP39Encode(standardPhrase, sizeof(standardPhrase), wordList, entropy.u8, sizeof(entropy));
				return std::string(standardPhrase);
			}
			return std::string();
		}


		bool Mnemonic::PhraseIsValid(const std::string &phrase, std::string &standardPhrase) {
			LoadLanguage(DEFAULT_LANGUAGE);
			standardPhrase = PhraseCheck(phrase);
			if (!standardPhrase.empty()) {
				return true;
			}

			ParamChecker::checkPathExists(_i18nPath);

			for (fs::directory_iterator it{_i18nPath}; it != fs::directory_iterator{}; ++it) {

				fs::path filePath = *it;
				if (fs::is_regular_file(filePath) &&
					filePath.filename().string().find(MNEMONIC_PREFIX) == 0 &&
					filePath.extension().string() == MNEMONIC_EXTENSION) {
					std::string language = filePath.stem().string().substr(strlen(MNEMONIC_PREFIX));
					ParamChecker::checkCondition(language.empty(), Error::Mnemonic,
												 "load mnemonic: " + filePath.string() + " filename invalid");
					LoadLanguage(language);
					standardPhrase = PhraseCheck(phrase);
					if (!standardPhrase.empty()) {
						return true;
					}
				}
			}

			return false;
		}

		void Mnemonic::LoadLanguage(const std::string &language) {
			_words.clear();
			_words.reserve(BIP39_WORDLIST_COUNT);
			_language = language;

			if (language == DEFAULT_LANGUAGE || language.empty()) {
				for (std::string str : BRBIP39WordsEn) {
					_words.push_back(str);
				}
			} else {
				fs::path filePath = _i18nPath / (MNEMONIC_PREFIX + language + MNEMONIC_EXTENSION);
				LoadPath(filePath);
			}
		}

		const std::string &Mnemonic::GetLanguage() const {
			return _language;
		}

		const std::vector<std::string> &Mnemonic::words() const {
			return _words;
		}

	}
}