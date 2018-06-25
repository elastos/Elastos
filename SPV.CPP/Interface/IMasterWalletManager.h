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

			virtual std::string GenerateMnemonic(const std::string &language) = 0;

			/**
			 * Create an new master wallet, an random seed will be generated combined with phrase password to create
			 * 	root key and chain code.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param language specify language of mnemonic. Language should not be empty, and exit corresponding language config file under the root path. The config begin with fixed prefix "mnemonic_" and end with ".txt" extension, for example mnemonic of Chinese config will be "mnemonic_chinese.txt".
			 * @param rootPath specify directory for all config files, including mnemonic config files and peer connection config files. Root should not be empty, otherwise will throw invalid argument exception.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const std::string &language = "english") = 0;

			/**
			 * Get manager existing master wallets.
			 * @return existing master wallet array.
			 */
			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const = 0;

			/**
			 * Destroy a master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 */
			virtual void DestroyWallet(const std::string &masterWalletId) = 0;

			/**
			 * Import master wallet by key store file.
			 * @param keystorePath specify key store file path to import wallet. Key store path should not be empty and really exist.
			 * @param backupPassword use to encrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithKeystore(
					const std::string &masterWalletId,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword,
					const std::string &phrasePassword) = 0;

			/**
			 * Import master wallet by mnemonic.
			 * @param mnemonic for importing the master wallet.
			 * @param phrasePassword combine with mnemonic to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param language specify language of mnemonic. Language should not be empty, and exit corresponding language config file under the root path. The config begin with fixed prefix "mnemonic_" and end with ".txt" extension, for example mnemonic of Chinese config will be "mnemonic_chinese.txt".
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const std::string &language) = 0;

			/**
			 * Export key store of the master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param backupPassword use to decrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param keystorePath specify the key store path to export. Key store path should not be empty.
			 */
			virtual nlohmann::json ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &payPassword) = 0;

			/**
			 * Export mnemonic of the master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return the mnemonic of master wallet.
			 */
			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &payPassword) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLETMANAGER_H__
