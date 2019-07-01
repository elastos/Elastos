// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERWALLET_H__
#define __ELASTOS_SDK_MASTERWALLET_H__

#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/KeyStore/KeyStore.h>
#include <SDK/SpvService/LocalStore.h>
#include <SDK/IDAgent/IDAgentImpl.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Account/Account.h>

#include <Interface/IMasterWallet.h>
#include <Interface/IIDAgent.h>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo;
		class ChainParams;
		class SubWallet;
		class KeyStore;
		class Config;
		class ChainConfig;

		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;
		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;
		typedef boost::shared_ptr<Config> ConfigPtr;
		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;

		typedef enum {
			CreateNormal,          // Select newest check point
			CreateMultiSign,       // Select oldest check point
			ImportFromMnemonic,    // Select oldest check point
			ImportFromLocalStore,  // Select check point from local store
			ImportFromKeyStore,    // Select check point from key store
		} MasterWalletInitFrom;

		class MasterWallet : public IMasterWallet, public IIDAgent {
		public:
			virtual ~MasterWallet();

			virtual void Save();

			virtual void ClearLocal();

			bool IsEqual(const MasterWallet &wallet) const;

		public: //override from IMasterWallet

			static std::string GenerateMnemonic(const std::string &language, const std::string &rootPath,
			                                    Mnemonic::WordCount wordCount = Mnemonic::WordCount::WORDS_12);

			virtual std::string GetID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual std::vector<ISubWallet *> GetAllSubWallets() const;

			virtual ISubWallet *CreateSubWallet(
					const std::string &chainID,
					uint64_t feePerKB);

			virtual void DestroyWallet(ISubWallet *wallet);

			virtual std::string GetPublicKey() const;

			virtual std::string Sign(
					const std::string &message,
					const std::string &payPassword);

			virtual bool CheckSign(
					const std::string &publicKey,
					const std::string &message,
					const std::string &signature);

			virtual bool IsAddressValid(const std::string &address) const;

			virtual std::vector<std::string> GetSupportedChains() const;

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			void InitSubWallets();

			std::string GetWalletID() const;

			virtual IIDAgent *GetIIDAgent();

		public: //override from IIDAgent
			virtual std::string DeriveIDAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index);

			virtual bool IsIDValid(const std::string &id);

			virtual nlohmann::json GenerateProgram(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			virtual std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			virtual std::vector<std::string> GetAllIDs() const;

			virtual std::string GetPublicKey(const std::string &id) const;

		protected:

			friend class MasterWalletManager;

			friend class IDAgentImpl;

			friend class SubWallet;

			typedef std::map<std::string, ISubWallet *> WalletMap;

			MasterWallet(
					const std::string &id,
					const std::string &rootPath,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					bool singleAddress,
					bool p2pEnable,
					const std::string &rootPath,
					const std::string &dataPath,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword,
					const std::string &rootPath,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &readonlyWalletJson,
					const std::string &rootPath,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					const std::string &rootPath,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &privKey,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					const std::string &rootPath,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword,
					const nlohmann::json &coSigners,
					uint32_t requiredSignCount,
					bool p2pEnable,
					const std::string &rootPath,
					const std::string &dataPath,
					MasterWalletInitFrom from);

			nlohmann::json ExportReadonlyKeyStore();

			nlohmann::json exportKeyStore(const std::string &backupPassword,
										  const std::string &payPassword);

			std::string exportMnemonic(const std::string &payPassword);

			SubWallet *SubWalletFactoryMethod(const CoinInfoPtr &info,
											  const ChainConfigPtr &config,
											  MasterWallet *parent);


			virtual void startPeerManager(SubWallet *wallet);

			virtual void stopPeerManager(SubWallet *wallet);

		protected:
			WalletMap _createdWallets;

			LocalStorePtr _localStore;

			MasterWalletInitFrom _initFrom;

			AccountPtr _account;

			std::string _id;
			std::string _rootPath;
			std::string _dataPath;

			ConfigPtr _config;
			boost::shared_ptr<IDAgentImpl> _idAgentImpl;
			bool _p2pEnable;

		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLET_H__
