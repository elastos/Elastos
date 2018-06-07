// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <sstream>

#include "BRBIP39WordsEn.h"

#include "Mnemonic.h"

#define MNEMONIC_PREFIX "mnemonic_"
#define MNEMONIC_EXTENSION ".txt"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace SDK {

		Mnemonic::Mnemonic(const std::string &language) :
			_i18nPath("data") {
			setLanguage(language);
		}

		Mnemonic::Mnemonic(const std::string &language, const boost::filesystem::path &path) {
			setI18nPath(path);
			setLanguage(language);
		}

		void Mnemonic::setLanguage(const std::string &language) {
			_words.clear();

			if (language == "english") {
				for (std::string str : BRBIP39WordsEn) {
					_words.push_back(str);
				}
			} else {
				fs::path fileName = _i18nPath;
				fileName /= MNEMONIC_PREFIX + language + MNEMONIC_EXTENSION;
				loadLanguage(fileName);
			}

			assert(_words.size() == BIP39_WORDLIST_COUNT);
		}

		void Mnemonic::loadLanguage(const fs::path &path) {

			std::fstream infile(path.string());
			std::string line;
			while (std::getline(infile, line)) {
				_words.push_back(line);
			}
		}

		void Mnemonic::setI18nPath(const fs::path &path) {
			_i18nPath = path;
		}

		const std::vector<std::string> &Mnemonic::words() const {
			return _words;
		}

	}
}