// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLETMANAGER_H__
#define __ELASTOS_SDK_MASTERWALLETMANAGER_H__

#include <map>

#include "IMasterWalletManager.h"

namespace Elastos {
	namespace ElaWallet {

		class MasterWalletManager : public IMasterWalletManager {
		public:
			/**
			 * Constructor
			 * @param rootPath specify directory for all config files, including mnemonic config files and peer connection config files. Root should not be empty, otherwise will throw invalid argument exception.
			 */
			explicit MasterWalletManager(const std::string &rootPath);

			/**
			 * Virtual destructor
			 */
			virtual ~MasterWalletManager();

			/**
			 * Save local storage specifically. Note that the storage saving will be called automatically in destructor.
			 */
			void SaveConfigs();

			/**
			 * Generate a mnemonic by random 128 entropy. We support English, Chinese, French, Italian, Japanese, and
			 * 	Spanish 6 types of mnemonic currently.
			 * @param language specify mnemonic language.
			 * @return a random mnemonic.
			 */
			virtual std::string GenerateMnemonic(const std::string &language) const;

			/**
			 * Get public key for creating multi sign wallet with phrase.
			 * @param phrase is something like mnemonic generated from GenerateMnemonic().
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @return public key as expected.
			 */
			virtual std::string GetMultiSignPubKey(const std::string &phrase, const std::string &phrasePassword) const;

			/**
			 * Get public key for creating multi sign wallet with private key.
			 * @param privKey private key to do the sign job of related multi-sign accounts.
			 * @return public key as expected.
			 */
			virtual std::string GetMultiSignPubKey(const std::string &privKey) const;

			/**
			 * Create a new master wallet by mnemonic and phrase password, or return existing master wallet if current master wallet manager has the master wallet id.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic use to generate seed which deriving the master private key and chain code.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param singleAddress singleAddress if true created wallet will have only one address inside, otherwise sub wallet will manager a chain of addresses for security.
			 * @param language specify language of mnemonic, value of language should correspond to the language of \p mnemonic.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress);

			/**
			  * Create a multi-sign master wallet by mnemonic phrase password and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param coSigners is an array of signers' public key
			  * @param requiredSignCount specify minimum count to accomplish related transactions.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount);

			/**
			  * Create a multi-sign master wallet by mnemonic phrase password and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param privKey private key to do the sign job of related multi-sign accounts.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param coSigners is an array of signers' public key
			  * @param requiredSignCount specify minimum count to accomplish related transactions.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &privKey,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount);

			/**
			 * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic use to generate seed which deriving the master private key and chain code.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param coSigners is an array of signers' public key.
			 * @param requiredSignCount specify minimum count to accomplish related transactions.
			 * @param language specify language of mnemonic, value of language should correspond to the language of \p mnemonic.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount);


			/**
			 * Get manager existing master wallets.
			 * @return existing master wallet array.
			 */
			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const;

			/**
			 * Get manager available master wallet ids.
			 * @return available ids array.
			 */
			virtual std::vector<std::string> GetAllMasterWalletIds() const;

			/**
			 * Get a master wallet object by id.
			 * @param masterWalletId master wallet id.
			 * @return master wallet object.
			 */
			virtual IMasterWallet *GetWallet(
					const std::string &masterWalletId) const;

			/**
			 * Destroy a master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 */
			virtual void DestroyWallet(
					const std::string &masterWalletId);

			/*
			 * To support old web keystore
			 */
			virtual IMasterWallet *ImportWalletWithKeystore(
				const std::string &masterWalletId,
				const nlohmann::json &keystoreContent,
				const std::string &backupPassword,
				const std::string &payPassword,
				const std::string &phrasePassword);

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
					const std::string &payPassword);

			/**
			 * Import master wallet by mnemonic.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic for importing the master wallet.
			 * @param phrasePassword combine with mnemonic to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param singleAddress singleAddress if true created wallet will have only one address inside, otherwise sub wallet will manager a chain of addresses for security.
			 * @param language specify language of mnemonic. Language should not be empty, and exit corresponding language config file under the root path. The config begin with fixed prefix "mnemonic_" and end with ".txt" extension, for example mnemonic of Chinese config will be "mnemonic_chinese.txt".
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress);

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
					const std::string &payPassword) const;

			/**
			 * Export mnemonic of the master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return the mnemonic of master wallet.
			 */
			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &payPassword) const;

			virtual std::string GetVersion() const;

			nlohmann::json EncodeTransactionToString(const nlohmann::json &tx);

			nlohmann::json DecodeTransactionFromString(const nlohmann::json &cipher);

		protected:
			typedef std::map<std::string, IMasterWallet *> MasterWalletMap;

			MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath);

			void initMasterWallets();

			void removeWallet(const std::string &masterWalletId, bool saveMaster = true);

			void checkRedundant(IMasterWallet *wallet) const;

		protected:
			std::string _rootPath;
			bool _p2pEnable;
			mutable MasterWalletMap _masterWalletMap;
		};
	}
}

#endif //__ELASTOS_SDK_MASTERWALLETMANAGER_H__
