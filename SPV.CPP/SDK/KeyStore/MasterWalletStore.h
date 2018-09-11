// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETSTORE_H__
#define __ELASTOS_SDK_MASTERWALLETSTORE_H__

#include <vector>
#include <boost/filesystem.hpp>

#include "SDK/Account/IAccount.h"
#include "CoinInfo.h"
#include "IdAgent/IdAgentImpl.h"

namespace Elastos {
	namespace ElaWallet {

		class MasterWalletStore {
		public:
			MasterWalletStore(const std::string &rootPath);

			~MasterWalletStore();

			void Load(const boost::filesystem::path &path);

			void Save(const boost::filesystem::path &path);

			const IdAgentInfo &GetIdAgentInfo() const;

			void SetIdAgentInfo(const IdAgentInfo &info);

			const std::vector<CoinInfo> &GetSubWalletInfoList() const;

			void SetSubWalletInfoList(const std::vector<CoinInfo> &infoList);

			IAccount *Account() const;

			void Reset(const std::string &phrase,
					   const std::string &language,
					   const std::string &phrasePassword,
					   const std::string &payPassword);

			void Reset(const nlohmann::json &coSigners,
					   const std::string &payPassword,
					   uint32_t requiredSignCount);

			void Reset(const std::string &privKey,
					   const nlohmann::json &coSigners,
					   const std::string &payPassword,
					   uint32_t requiredSignCount);

		private:
			JSON_SM_LS(MasterWalletStore);

			JSON_SM_RS(MasterWalletStore);

			TO_JSON(MasterWalletStore);

			FROM_JSON(MasterWalletStore);

		private:

			std::string _rootPath;
			AccountPtr _account;
			IdAgentInfo _idAgentInfo;
			std::vector<CoinInfo> _subWalletsInfoList;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLETSTORE_H__
