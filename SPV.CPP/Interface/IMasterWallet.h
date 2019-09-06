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
			 * Get basic info of master wallet
			 * @return basic information. Such as:
			 * {"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard", "HasPassPhrase": false}
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
			 * @return If success will return a pointer of sub wallet interface.
			 */
			virtual ISubWallet *CreateSubWallet(const std::string &chainID) = 0;

			/**
			 * Verify private key whether same as current wallet
			 * @param mnemonic
			 * @param passphrase
			 * @return
			 */
			virtual bool VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const = 0;

			/**
			 *
			 * @param passphrase
			 * @param payPasswd
			 * @return
			 */
			virtual bool VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const = 0;

			/**
			 *
			 * @param payPasswd
			 * @return
			 */
			virtual bool VerifyPayPassword(const std::string &payPasswd) const = 0;

			/**
			 * Destroy a sub wallet created by the master wallet.
			 * @param wallet sub wallet object, should created by the master wallet.
			 */
			virtual void DestroyWallet(ISubWallet *wallet) = 0;

			/**
			 * Get public key info
			 * @return public key info
			 */
			virtual nlohmann::json GetPubKeyInfo() const = 0;

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
