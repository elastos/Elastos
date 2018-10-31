// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MNEMONICS_H__
#define __ELASTOS_SDK_MNEMONICS_H__

#include <string>
#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Mnemonic {
		public:
			Mnemonic(const std::string &language, const boost::filesystem::path &path);

			Mnemonic(const boost::filesystem::path &path, const std::string &phrase);

			Mnemonic(const boost::filesystem::path &path);

			const std::vector<std::string> &words() const;

			void LoadLanguage(const std::string &language);

			const std::string &GetLanguage() const;

		private:
			void LoadPath(const boost::filesystem::path &filePath);

			bool DetectLanguageLoad(const std::string &phrase);

			bool IsPhraseValid(const std::string &phrase);

		private:
			std::string _language;
			boost::filesystem::path _i18nPath;
			std::vector<std::string> _words;
		};

	}
}

#endif //SPVSDK_MNEMONICS_H
