// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MNEMONICS_H__
#define __ELASTOS_SDK_MNEMONICS_H__

#include <SDK/Common/uint256.h>

#include <string>
#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Mnemonic {
		public:
			Mnemonic(const boost::filesystem::path &rootPath);

			std::string Create(const std::string &language);

			uint512 DeriveSeed(const std::string &mnemonic, const std::string &passphrase);

		private:
			void LoadPath(const boost::filesystem::path &filePath, std::vector<std::string> &wordLists);

		private:
			boost::filesystem::path _rootPath;
		};

	}
}

#endif //__ELASTOS_SDK_MNEMONICS_H__
