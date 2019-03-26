// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPVSDK_SUBACCOUNTUTILS_H
#define SPVSDK_SUBACCOUNTUTILS_H

#include "ISubAccount.h"

#include <SDK/SpvService/CoinInfo.h>

namespace Elastos {
	namespace ElaWallet {

		class SubAccountGenerator {
		public:
			SubAccountGenerator();

			~SubAccountGenerator();

			ISubAccount *Generate();

			void SetCoinInfo(const CoinInfo &coinInfo);

			void SetParentAccount(IAccount *account);

			void SetMasterPubKey(const HDKeychain &masterPubKey);

			void SetVotePubKey(const bytes_t &pubKey);

			void Clean();

			const bytes_t &GetResultPublicKey() const;

			const bytes_t &GetResultChainCode() const;

			static HDKeychain
			GenerateMasterPubKey(IAccount *account, uint32_t coinIndex, const std::string &payPassword);

			static bytes_t GenerateVotePubKey(IAccount *account, uint32_t coinIndex, const std::string &payPasswd);

		private:
			ISubAccount *GenerateFromCoinInfo(IAccount *account, const CoinInfo &coinInfo);

			ISubAccount *GenerateFromHDPath(IAccount *account, uint32_t coinIndex);

		private:
			CoinInfo _coinInfo;
			IAccount *_parentAccount;

			bytes_t _votePubKey;
			bytes_t _resultPubKey;
			bytes_t _resultChainCode;
			HDKeychain _masterPubKey;
		};

	}
}

#endif //SPVSDK_SUBACCOUNTUTILS_H
