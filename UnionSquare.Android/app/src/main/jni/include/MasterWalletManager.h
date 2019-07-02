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
			 * @param rootPath Specify directory for all config files, including mnemonic config files. Root should not be empty, otherwise will throw invalid argument exception.
			 * @param dataPath The path contains data of wallet created. If empty, data of wallet will store in rootPath.
			 */
			explicit MasterWalletManager(const std::string &rootPath, const std::string &dataPath = "");

			virtual ~MasterWalletManager();

			void SaveConfigs();

			virtual std::string GenerateMnemonic(const std::string &language, int wordCount = 12) const;

			virtual std::string GetMultiSignPubKey(const std::string &phrase, const std::string &phrasePassword) const;

			virtual std::string GetMultiSignPubKey(const std::string &privKey) const;

			virtual IMasterWallet *CreateMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &xprv,
					const std::string &payPassword,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &publicKeys,
					uint32_t m,
					time_t timestamp = 0);

			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const;

			virtual std::vector<std::string> GetAllMasterWalletIds() const;

			virtual IMasterWallet *GetMasterWallet(
					const std::string &masterWalletId) const;

			virtual void DestroyWallet(
					const std::string &masterWalletId);

			virtual IMasterWallet *ImportWalletWithKeystore(
					const std::string &masterWalletId,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword);

			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &masterWalletId,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress,
					time_t timestamp = 0);

			virtual IMasterWallet *ImportReadonlyWallet(
				const std::string &masterWalletID,
				const nlohmann::json &walletJson);

			virtual nlohmann::json ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &payPassword) const;

			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &payPassword) const;

			virtual nlohmann::json ExportReadonlyWallet(
				IMasterWallet *masterWallet) const;

			virtual std::string ExportxPrivateKey(
				IMasterWallet *masterWallet,
				const std::string &payPasswd) const;

			virtual std::string ExportMasterPublicKey(
				IMasterWallet *masterWallet) const;

			virtual std::string GetVersion() const;

		protected:
			typedef std::map<std::string, IMasterWallet *> MasterWalletMap;

			MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath, const std::string &dataPath);

			void initMasterWallets();

			void removeWallet(const std::string &masterWalletId, bool saveMaster = true);

			void checkRedundant(IMasterWallet *wallet) const;

		protected:
			std::string _rootPath;
			std::string _dataPath;
			bool _p2pEnable;
			mutable MasterWalletMap _masterWalletMap;
		};
	}
}

#endif //__ELASTOS_SDK_MASTERWALLETMANAGER_H__
