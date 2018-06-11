// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETSTORE_H__
#define __ELASTOS_SDK_MASTERWALLETSTORE_H__

#include <vector>
#include <boost/filesystem.hpp>

#include "CMemBlock.h"
#include "CoinInfo.h"

namespace Elastos {
	namespace SDK {

		class MasterWalletStore {
		public:
			MasterWalletStore();

			~MasterWalletStore();

			void Load(const boost::filesystem::path &path);

			void Save(const boost::filesystem::path &path);

			const std::string &GetEncrpytedKey() const;

			void SetEncryptedKey(const std::string &data);

			const std::string &GetEncryptedMnemonic() const;

			void SetEncryptedMnemonic(const std::string &data);

			const std::string &GetEncrptedPhrasePassword() const;

			void SetEncryptedPhrasePassword(const std::string &data);

			const std::vector<CoinInfo> &GetSubWalletInfoList() const;

			void SetSubWalletInfoList(const std::vector<CoinInfo> &infoList);

		private:
			JSON_SM_LS(MasterWalletStore);
			JSON_SM_RS(MasterWalletStore);
			TO_JSON(MasterWalletStore);
			FROM_JSON(MasterWalletStore);

		private:
			std::string _encryptedKey;
			std::string _encryptedMnemonic;
			std::string _encryptedPhrasePass;
			std::vector<CoinInfo> _subWalletsInfoList;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLETSTORE_H__
