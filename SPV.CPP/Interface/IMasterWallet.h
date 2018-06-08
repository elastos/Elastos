// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMASTERWALLET_H__
#define __ELASTOS_SDK_IMASTERWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
	namespace SDK {

		enum SubWalletType{
			Normal = 0,
			Mainchain,
			Sidechain,
			Idchain
		};

		class IMasterWallet {
		public:
			virtual ~IMasterWallet() noexcept {}

			virtual ISubWallet *CreateSubWallet(
					SubWalletType type,
					const std::string &chainID,
					uint32_t coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					uint64_t feePerKb = 0) = 0;

			virtual ISubWallet *RecoverSubWallet(
					SubWalletType type,
					const std::string &chainID,
					uint32_t coinTypeIndex,
					const std::string &payPassword,
					bool singleAddress,
					uint32_t limitGap,
					uint64_t feePerKb = 0) = 0;

			virtual void DestroyWallet(ISubWallet *wallet) = 0;

			virtual std::string GetPublicKey() = 0;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword) = 0;

			virtual nlohmann::json CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature) = 0;

			virtual bool DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index,
					const std::string &payPassword,
					std::string &id,
					std::string &key) = 0;

			virtual bool IsIdValid(const std::string &id) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLET_H__
