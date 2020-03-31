/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_IMASTERWALLET_H__
#define __ELASTOS_SDK_IMASTERWALLET_H__

#include "ISubWallet.h"

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
			 * Get a sub wallet of chainID.
			 * @param chainID unique identity of a sub wallet. Chain id should not be empty.
			 * @return If success will return a pointer of sub wallet interface.
			 */
			virtual ISubWallet *GetSubWallet(const std::string &chainID) const = 0;

			/**
			 * Create a sub wallet of chainID.
			 * @param chainID unique identity of a sub wallet. Chain id should not be empty.
			 * @return If success will return a pointer of sub wallet interface.
			 */
			virtual ISubWallet *CreateSubWallet(const std::string &chainID) = 0;

			/**
			 * Export Keystore of the current wallet in JSON format.
			 * @param backupPassword use to decrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return key store content in json format.
			 */
			virtual nlohmann::json ExportKeystore(
				const std::string &backupPassword,
				const std::string &payPassword) const = 0;

			/**
			 * Export mnemonic of the current wallet.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return the mnemonic of master wallet.
			 */
			virtual std::string ExportMnemonic(const std::string &payPassword) const = 0;

			/**
			 * Export wallet info except private keys.
			 * @return If success, readonly wallet will be returned.
			 */
			virtual nlohmann::json ExportReadonlyWallet() const = 0;

			/**
			 * Export root private key of the current wallet.
			 * @param payPasswd use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return root private key.
			 */
			virtual std::string ExportPrivateKey(const std::string &payPasswd) const = 0;

			/**
			 * Export master public key.
			 * @return master public key
			 */
			virtual std::string ExportMasterPublicKey() const = 0;

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
			 * @param chainID chain ID of subWallet.
			 */
			virtual void DestroyWallet(const std::string &chainID) = 0;

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

		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLET_H__
