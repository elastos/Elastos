// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETSTORE_H__
#define __ELASTOS_SDK_MASTERWALLETSTORE_H__

#include <vector>
#include <boost/filesystem.hpp>

#include "CMemBlock.h"
#include "CoinInfo.h"
#include "IdAgent/IdAgentImpl.h"

namespace Elastos {
	namespace SDK {

		class MasterWalletStore {
		public:
			MasterWalletStore();

			~MasterWalletStore();

			void Load(const boost::filesystem::path &path);

			void Save(const boost::filesystem::path &path);

			const CMBlock &GetEncrpytedKey() const;

			void SetEncryptedKey(const CMBlock &data);

			const CMBlock &GetEncryptedMnemonic() const;

			void SetEncryptedMnemonic(const CMBlock &data);

			const CMBlock &GetEncrptedPhrasePassword() const;

			void SetEncryptedPhrasePassword(const CMBlock &data);

			const std::string &GetLanguage() const;

			void SetLanguage(const std::string &language);

			const IdAgentInfo &GetIdAgentInfo() const;

			void SetIdAgentInfo(const IdAgentInfo &info);

			const std::vector<CoinInfo> &GetSubWalletInfoList() const;

			void SetSubWalletInfoList(const std::vector<CoinInfo> &infoList);

		private:
			JSON_SM_LS(MasterWalletStore);
			JSON_SM_RS(MasterWalletStore);
			TO_JSON(MasterWalletStore);
			FROM_JSON(MasterWalletStore);

		private:
			CMBlock _encryptedKey;
			CMBlock _encryptedMnemonic;
			CMBlock _encryptedPhrasePass;
			std::string _language;
			IdAgentInfo _idAgentInfo;
			std::vector<CoinInfo> _subWalletsInfoList;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLETSTORE_H__
