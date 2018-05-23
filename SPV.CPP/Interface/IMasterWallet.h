// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMASTERWALLET_H__
#define __ELASTOS_SDK_IMASTERWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
	namespace SDK {

		class IMasterWallet {
		public:
			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					uint64_t feePerKb = 0) = 0;

			virtual ISubWallet *RecoverSubWallet(
					const std::string &chainID,
					int coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					int limitGap,
					uint64_t feePerKb = 0) = 0;

			virtual void DestroyWallet(ISubWallet *wallet) = 0;

			virtual std::string GetPublicKey() = 0;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword) = 0;

			virtual nlohmann::json CheckSign(
					const std::string &address,
					const std::string &message,
					const std::string &signature) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLET_H__
