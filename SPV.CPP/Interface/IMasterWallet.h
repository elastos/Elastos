// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IMASTERWALLET_H__
#define __ELASTOS_SDK_IMASTERWALLET_H__

#include "ISubWallet.h"
#include "IIDAgent.h"

namespace Elastos {
	namespace ElaWallet {

		class IMasterWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~IMasterWallet() noexcept {}

			/**
			 * Get the master wallet id.
			 * @return master wallet id.
			 */
			virtual std::string GetID() const = 0;

			/**
			 * Here is a example of standard wallet basic info:
			 * {"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"}
			 * @return basic information of current master wallet.
			 */
			virtual nlohmann::json GetBasicInfo() const = 0;

			/**
			 * Get wallet existing sub wallets.
			 * @return existing sub wallets by array.
			 */
			virtual std::vector<ISubWallet *> GetAllSubWallets() const = 0;

			/**
			 * Create a sub wallet by specifying wallet type.
			 * @param chainID unique identity of a sub wallet. Chain id should not be empty.
			 * @param feePerKb specify fee per kb to calculate fee by size of transaction. Fee per key default set to zero so that sub wallet will calculate by default "fee rate".
			 * @return If success will return a pointer of sub wallet interface.
			 */
			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					uint64_t feePerKB) = 0;

			/**
			 * Destroy a sub wallet created by the master wallet.
			 * @param wallet sub wallet object, should created by the master wallet.
			 */
			virtual void DestroyWallet(ISubWallet *wallet) = 0;

			/**
			 * Get public key ring of current wallet
			 * {"xPubKey":"xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg","requestPubKey":"03ec571bba042036f93c17c2507f6b03f82d5b364e525715c40f16f723d4e5fac1"}
			 * @return PublicKeyRing of json format
			 */
			virtual nlohmann::json GetOwnerPublicKeyRing() const = 0;

			/**
			 * Get all public key ring of current wallet
			 * [{"xPubKey":"xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg","requestPubKey":"03ec571bba042036f93c17c2507f6b03f82d5b364e525715c40f16f723d4e5fac1"}]
			 * @return PublicKeyRing of json format
			 */
			virtual nlohmann::json GetPublicKeyRing() const = 0;

			/**
			 * Sign message through root private key of the master wallet.
			 * @param message need to signed, it should not be empty.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return signed data of the message.
			 */
			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword) = 0;

			/**
			 * Verify signature by public key and raw message. This method can check signatures signed by any private keys not just the root private key of the master wallet.
			 * @param publicKey belong to the private key signed the signature.
			 * @param message raw data.
			 * @param signature signed data by a private key that correspond to the public key.
			 * @return true or false.
			 */
			virtual bool CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature) = 0;

			/**
			 * Verify an address which can be normal, multi-sign, cross chain, or id address.
			 * @param address to be verified.
			 * @return True if valid, otherwise return false.
			 */
			virtual bool IsAddressValid(const std::string &address) const = 0;

			/**
			 * Get all chain ids of supported chains.
			 * @return a list of chain id.
			 */
			virtual std::vector<std::string> GetSupportedChains() const = 0;

			/**
			 * Change pay password which encrypted private key and other important data in memory.
			 * @param oldPassword the old pay password.
			 * @param newPassword new pay password.
			 */
			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword) = 0;

			virtual IIDAgent *GetIIDAgent() = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLET_H__
