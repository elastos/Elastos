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
#ifndef __ELASTOS_SDK_MASTERWALLET_H__
#define __ELASTOS_SDK_MASTERWALLET_H__

#include <WalletCore/Mnemonic.h>
#include <WalletCore/KeyStore.h>
#include <SpvService/LocalStore.h>
#include <Plugin/Registry.h>
#include <Plugin/Transaction/Transaction.h>
#include <Account/Account.h>

#include <IMasterWallet.h>

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

		class MasterWallet : public IMasterWallet {
		public:
			virtual ~MasterWallet();

			virtual void RemoveLocalStore();

			bool IsEqual(const MasterWallet &wallet) const;

			void FlushData();

		public: //override from IMasterWallet

			static std::string GenerateMnemonic(const std::string &language, const std::string &rootPath,
			                                    Mnemonic::WordCount wordCount = Mnemonic::WordCount::WORDS_12);

			virtual std::string GetID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual std::vector<ISubWallet *> GetAllSubWallets() const;

			virtual ISubWallet *GetSubWallet(const std::string &chainID) const;

			virtual ISubWallet *CreateSubWallet(const std::string &chainID);

			nlohmann::json ExportKeystore(const std::string &backupPassword,
										  const std::string &payPassword) const;

			std::string ExportMnemonic(const std::string &payPassword) const;

			nlohmann::json ExportReadonlyWallet() const;

			std::string ExportPrivateKey(const std::string &payPasswd) const;

			std::string ExportMasterPublicKey() const;

			virtual bool VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const;

			virtual bool VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const;

			virtual bool VerifyPayPassword(const std::string &payPasswd) const;

			virtual void DestroyWallet(const std::string &chainID);

			virtual nlohmann::json GetPubKeyInfo() const;

			virtual bool IsAddressValid(const std::string &address) const;

			virtual std::vector<std::string> GetSupportedChains() const;

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			void InitSubWallets();

			std::string GetWalletID() const;

			ChainConfigPtr GetChainConfig(const std::string &chainID) const;

			std::string GetDataPath() const;

			AccountPtr GetAccount() const;
		protected:

			friend class MasterWalletManager;

			friend class SubWallet;

			typedef std::map<std::string, ISubWallet *> WalletMap;

			MasterWallet(
					const std::string &id,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPasswd,
					bool singleAddress,
					bool p2pEnable,
					const ConfigPtr &config,
					const std::string &dataPath,
					time_t earliestPeerTime,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &keystoreContent,
					const std::string &backupPassword,
					const std::string &payPassword,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const nlohmann::json &readonlyWalletJson,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::vector<PublicKeyRing> &pubKeyRings,
					uint32_t m,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					bool singleAddress,
					bool compatible,
					time_t earliestPeerTime,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &xprv,
					const std::string &payPassword,
					const std::vector<PublicKeyRing> &cosigners,
					uint32_t m,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					bool singleAddress,
					bool compatible,
					time_t earliestPeerTime,
					MasterWalletInitFrom from);

			MasterWallet(
					const std::string &id,
					const std::string &mnemonic,
					const std::string &passphrase,
					const std::string &payPasswd,
					const std::vector<PublicKeyRing> &cosigners,
					uint32_t m,
					const ConfigPtr &config,
					const std::string &dataPath,
					bool p2pEnable,
					bool singleAddress,
					bool compatible,
					time_t earliestPeerTime,
					MasterWalletInitFrom from);

			ISubWallet *SubWalletFactoryMethod(const CoinInfoPtr &info,
											  const ChainConfigPtr &config,
											  MasterWallet *parent,
											  const std::string &netType);

			virtual void startPeerManager(SubWallet *wallet);

			virtual void stopPeerManager(SubWallet *wallet);

			virtual void CloseAllSubWallets();

		protected:
			mutable WalletMap _createdWallets;

			MasterWalletInitFrom _initFrom;

			AccountPtr _account;

			std::string _id;

			time_t _earliestPeerTime;

			ConfigPtr _config;
			bool _p2pEnable;

		};

	}
}

#endif //__ELASTOS_SDK_MASTERWALLET_H__
