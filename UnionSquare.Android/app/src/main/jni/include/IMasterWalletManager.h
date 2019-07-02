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
			  * @param wordCount value can only be one of {12, 15, 18, 21, 24}.
			  * @return a random mnemonic.
			  */
			virtual std::string GenerateMnemonic(const std::string &language, int wordCount = 12) const = 0;

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
			  * @param masterWalletID is the unique identification of a master wallet object.
			  * @param publicKeys is an array of signers' public key.
			  * @param m specify minimum count of signature to accomplish related transaction.
			  * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletID,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0) = 0;

			/**
			  * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			  * @param masterWalletId is the unique identification of a master wallet object.
			  * @param xprv root extend private key of wallet.
			  * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			  * @param publicKeys is an array of signers' public key.
			  * @param m specify minimum count of signature to accomplish related transaction.
			  * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			  * @return If success will return a pointer of master wallet interface.
			  */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &xprv,
					const std::string &payPassword,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0) = 0;

			/**
			 * Create a multi-sign master wallet by private key and related co-signers, or return existing master wallet if current master wallet manager has the master wallet id.
			 * @param masterWalletId is the unique identification of a master wallet object.
			 * @param mnemonic use to generate seed which deriving the master private key and chain code.
			 * @param phrasePassword combine with random seed to generate root key and chain code. Phrase password can be empty or between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to encrypt important things(such as private key) in memory. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param publicKeys is an array of signers' public key.
			 * @param m specify minimum count of signature to accomplish related transactions.
			 * @param timestamp the value of time in seconds since 1970-01-01 00:00:00. It means the time when the wallet contains the first transaction.
			 * @return If success will return a pointer of master wallet interface.
			 */
			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0) = 0;

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
			virtual IMasterWallet *GetMasterWallet(
					const std::string &masterWalletId) const = 0;

			/**
			 * Destroy a master wallet.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 */
			virtual void DestroyWallet(const std::string &masterWalletId) = 0;

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
			 * @param walletJson generate by ExportReadonlyWallet().
			 */
			virtual IMasterWallet *ImportReadonlyWallet(
				const std::string &masterWalletID,
				const nlohmann::json &walletJson) = 0;

			/**
			 * Export key store content of the master wallet in json format.
			 * @param masterWallet A pointer of master wallet interface create or imported by wallet factory object.
			 * @param backupPassword use to decrypt key store file. Backup password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @param withPrivKey indicate keystore contain private key or not.
			 * @return If success will return key store content in json format.
			 */
			virtual nlohmann::json ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &payPassword) const = 0;

			/**
			 * Export mnemonic of the master wallet.
			 * @param masterWallet A pointer of master wallet interface created or imported by wallet factory object.
			 * @param payPassword use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success will return the mnemonic of master wallet.
			 */
			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &payPassword) const = 0;

			/**
			 * Export wallet info except private keys.
			 * @param masterWallet A pointer of master wallet interface created or imported by wallet factory object.
			 * @return If success will return public keys of readonly(watch) wallet.
			 *
			 * example of return json:
			 * {"CoinInfoList":[{"ChainID":"ELA","EarliestPeerTime":1561716528,"FeePerKB":10000,"VisibleAssets":["a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"]}],"OwnerPubKey":"03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d","SingleAddress":false,"m":1,"mnemonicHasPassphrase":false,"n":1,"network":"","publicKeyRing":[{"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}],"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}
			 */
			virtual nlohmann::json ExportReadonlyWallet(
				IMasterWallet *masterWallet) const = 0;

			/**
			 * Export root extend private key of wallet.
			 * @param masterWallet A pointer of master wallet interface created or imported by wallet factory object.
			 * @param payPasswd use to decrypt and generate mnemonic temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return extend private key.
			 */
			virtual std::string ExportxPrivateKey(
				IMasterWallet *masterWallet,
				const std::string &payPasswd) const = 0;

			/**
			 * Export master public key.
			 * @param masterWallet A pointer of master wallet interface created or imported by wallet factory object.
			 * @return master public key
			 */
			virtual std::string ExportMasterPublicKey(
				IMasterWallet *masterWallet) const = 0;

			virtual std::string GetVersion() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IMASTERWALLETMANAGER_H__
