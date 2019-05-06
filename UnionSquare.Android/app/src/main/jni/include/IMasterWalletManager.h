// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
			  * Generate a mnemonic by random 128 entropy. We support English, Chinese, French, Italian, Japanese, and
			  * 	Spanish 6 types of mnemonic currently.
			  * @param language specify mnemonic language.
			  * @return a random mnemonic.
			  */
			virtual std::string GenerateMnemonic(const std::string &language) const = 0;

			/**
			 * Get public key for creating multi sign wallet with phrase.
			 * @param phrase is something like mnemonic generated from GenerateMnemonic().
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @return public key as expected.
			 */
			virtual std::string GetMultiSignPubKey(const std::string &phrase, const std::string &phrasePassword) const = 0;

			/**
			 * Get public key for creating multi sign wallet with private key.
			 * @param privKey private key to do the sign job of related multi-sign accounts.
			 * @return public key as expected.
			 */
			virtual std::string GetMultiSignPubKey(const std::string &privKey) const = 0;

			/**
			  * Create a new master wallet by mnemonic and phrase password, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param mnemonic use to generate seed which deriving the master private key and chain code.
			  * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param singleAddress singleAddress if true created wallet will have only one address inside, otherwise sub wallet will manager a chain of addresses for security.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress) = 0;

			/**
			  * Create a multi-sign master wallet by related co-signers, or return existing master wallet if current master wallet manager has the master wallet id. Note this creating method generate an readonly multi-sign account which can not append sign into a transaction.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param coSigners is an array of signers' public key.
			  * @param requiredSignCount specify minimum count to accomplish related transactions.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount) = 0;

			/**
			  * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param privKey private key to do the sign job of related multi-sign accounts.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param coSigners is an array of signers' public key.
			  * @param requiredSignCount specify minimum count to accomplish related transactions.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &privKey,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount) = 0;

			/**
			 * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic use to generate seed which deriving the master private key and chain code.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param coSigners is an array of signers' public key.
			 * @param requiredSignCount specify minimum count to accomplish related transactions.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount) = 0;

			/**
			 * Get manager existing master wallets.
			 * @return existing master wallet array.
			 */
			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const = 0;

			/**
			 * Get manager available master wallet ids.
			 * @return available ids array.
			 */
			virtual std::vector<std::string> GetAllMasterWalletIds() const = 0;

			/**
			 * Get a master wallet object by id.
			 * @param masterWalletId master wallet id.
			 * @return master wallet object.
			 */
			virtual IMasterWallet *GetWallet(
					const std::string &masterWalletId) const = 0;

			/**
			 * Destroy a master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 */
			virtual void DestroyWallet(const std::string &masterWalletId) = 0;

			/*
			 * To support old web keystore
			 */
			virtual IMasterWallet *ImportWalletWithKeystore(
				const std::string &masterWalletId,
				const nlohmann::json &keystoreContent,
				const std::string &backupPassword,
				const std::string &payPassword,
				const std::string &phrasePassword) = 0;

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
					const std::string &masterWalletId,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword) = 0;

			/**
			 * Import master wallet by mnemonic.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic for importing the master wallet.
			 * @param phrasePassword combine with mnemonic to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param singleAddress singleAddress if true created wallet will have only one address inside, otherwise sub wallet will manager a chain of addresses for security.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress) = 0;

			/**
			 * Export key store content of the master wallet in json format.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param backupPassword use to decrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return key store content in json format.
			 */
			virtual nlohmann::json ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &payPassword) const = 0;

			/**
			 * Export mnemonic of the master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return the mnemonic of master wallet.
			 */
			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &payPassword) const = 0;

			virtual std::string GetVersion() const = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLETMANAGER_H__
