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

#ifndef __ELASTOS_SDK_IMASTERWALLETMANAGER_H__
#define __ELASTOS_SDK_IMASTERWALLETMANAGER_H__

#include "IMasterWallet.h"

namespace Elastos {
	namespace ElaWallet {

		class IMasterWalletManager {
		public:
			/**
			  * Virtual destructor
			  */
			virtual ~IMasterWalletManager() noexcept {}

			/**
			  * Generate a mnemonic by random entropy. We support English, Chinese, French, Italian, Japanese, and
			  * 	Spanish 6 types of mnemonic currently.
			  * @param language specify mnemonic language.
			  * @param wordCount value can only be one of {12, 15, 18, 21, 24}.
			  * @return a random mnemonic.
			  */
			virtual std::string GenerateMnemonic(const std::string &language, int wordCount = 12) const = 0;

			/**
			  * Create a new master wallet by mnemonic and phrase password, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletID is the unique identification of a master wallet object.
			  * @param mnemonic use to generate seed which deriving the master private key and chain code.
			  * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param singleAddress if true, the created wallet will only contain one address, otherwise wallet will manager a chain of addresses.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMasterWallet(
					const std::string &masterWalletID,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress) = 0;

			/**
			  * Create a multi-sign master wallet by related co-signers, or return existing master wallet if current master wallet manager has the master wallet id. Note this creating method generate an readonly multi-sign account which can not append sign into a transaction.
			  * @param masterWalletID is the unique identification of a master wallet object.
			  * @param cosigners JSON array of signer's extend public key. Such as: ["xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg", "xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc"]
			  * @param m specify minimum count of signature to accomplish related transaction.
			  * @param singleAddress if true, the created wallet will only contain one address, otherwise wallet will manager a chain of addresses.
			  * @param compatible if true, will compatible with web multi-sign wallet.
			  * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletID,
					const nlohmann::json &cosigners,
					uint32_t m,
					bool singleAddress,
					bool compatible = false,
					time_t timestamp = 0) = 0;

			/**
			  * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletID is the unique identification of a master wallet object.
			  * @param xprv root extend private key of wallet.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param cosigners JSON array of signer's extend public key. Such as: ["xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg", "xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc"]
			  * @param m specify minimum count of signature to accomplish related transaction.
			  * @param singleAddress if true, the created wallet will only contain one address, otherwise wallet will manager a chain of addresses.
			  * @param compatible if true, will compatible with web multi-sign wallet.
			  * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletID,
					const std::string &xprv,
					const std::string &payPassword,
					const nlohmann::json &cosigners,
					uint32_t m,
					bool singleAddress,
					bool compatible = false,
					time_t timestamp = 0) = 0;

			/**
			 * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			 * @param masterWalletID is the unique identification of a master wallet object.
			 * @param mnemonic use to generate seed which deriving the master private key and chain code.
			 * @param passphrase combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param cosigners JSON array of signer's extend public key. Such as: ["xpub6CLgvYFxzqHDJCWyGDCRQzc5cwCFp4HJ6QuVJsAZqURxmW9QKWQ7hVKzZEaHgCQWCq1aNtqmE4yQ63Yh7frXWUW3LfLuJWBtDtsndGyxAQg", "xpub6CWEYpNZ3qLG1z2dxuaNGz9QQX58wor9ax8AiKBvRytdWfEifXXio1BgaVcT4t7ouP34mnabcvpJLp9rPJPjPx2m6izpHmjHkZAHAHZDyrc"]
			 * @param m specify minimum count of signature to accomplish related transactions.
			 * @param singleAddress if true, the created wallet will only contain one address, otherwise wallet will manager a chain of addresses.
			 * @param compatible if true, will compatible with web multi-sign wallet.
			 * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletID,
					const std::string &mnemonic,
					const std::string &passphrase,
					const std::string &payPassword,
					const nlohmann::json &cosigners,
					uint32_t m,
					bool singleAddress,
					bool compatible = false,
					time_t timestamp = 0) = 0;

			/**
			 * Get manager existing master wallets.
			 * @return existing master wallet array.
			 */
			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const = 0;

			/**
			 * Get manager available master wallet ID
			 * @return available id array
			 */
			virtual std::vector<std::string> GetAllMasterWalletID() const = 0;

			/**
			 * Get status indicating whether wallet loaded
			 * @return return true or false
			 */
			 virtual bool WalletLoaded(const std::string &masterWalletID) const = 0;

			/**
			 * Get a master wallet object by id.
			 * @param masterWalletID master wallet id.
			 * @return master wallet object.
			 */
			virtual IMasterWallet *GetMasterWallet(
					const std::string &masterWalletID) const = 0;

			/**
			 * Destroy a master wallet.
			 * @param masterWalletID A pointer of master wallet interface create or imported by wallet factory object.
			 */
			virtual void DestroyWallet(const std::string &masterWalletID) = 0;

			/**
			 * Import master wallet by key store file.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param keystoreContent specify key store content in json format.
			 * @param backupPassword use to encrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithKeystore(
					const std::string &masterWalletID,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword) = 0;

			/**
			 * Import master wallet by mnemonic.
			 * @param masterWalletID is the unique identification of a master wallet object.
			 * @param mnemonic for importing the master wallet.
			 * @param phrasePassword combine with mnemonic to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param singleAddress singleAddress if true created wallet will have only one address inside, otherwise sub wallet will manager a chain of addresses for security.
			 * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &masterWalletID,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress,
					time_t timestamp = 0) = 0;

			/**
			 * Import read-only(watch) wallet which does not contain any private keys.
			 * @param masterWalletID is the unique identification of a master wallet object.
			 * @param walletJson generate by IMasterWallet::ExportReadonlyWallet().
			 */
			virtual IMasterWallet *ImportReadonlyWallet(
				const std::string &masterWalletID,
				const nlohmann::json &walletJson) = 0;

			/**
			 * Get version
			 * @return SPV SDK version
			 */
			virtual std::string GetVersion() const = 0;

			/**
			 * Flush data into disk before destructions
			 */
			virtual void FlushData() = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLETMANAGER_H__
