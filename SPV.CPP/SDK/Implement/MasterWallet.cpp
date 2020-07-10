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
#include "IDChainSubWallet.h"
#include "EthSidechainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SubWallet.h"
#include "MasterWallet.h"
#include "TokenchainSubWallet.h"

#include <Plugin/Transaction/Asset.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/CoinInfo.h>
#include <SpvService/Config.h>

#include <vector>
#include <boost/filesystem.hpp>
#include <ethereum/base/BREthereumLogic.h>
#include <ethereum/base/BREthereumAddress.h>

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const std::string &id,
								   const ConfigPtr &config,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(0),
				_config(config) {

			_account = AccountPtr(new Account(dataPath + "/" + _id));
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &passphrase,
								   const std::string &payPasswd,
								   bool singleAddress,
								   bool p2pEnable,
								   const ConfigPtr &config,
								   const std::string &dataPath,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(earliestPeerTime),
				_initFrom(from),
				_config(config) {

			_account = AccountPtr(new Account(dataPath + "/" + _id, mnemonic, passphrase, payPasswd, singleAddress));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPasswd,
								   const ConfigPtr &config,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(0),
				_initFrom(from),
				_config(config) {

			KeyStore keystore;
			keystore.Import(keystoreContent, backupPassword);

			_account = AccountPtr(new Account(dataPath + "/" + _id, keystore, payPasswd));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &readonlyWalletJson,
								   const ConfigPtr &config,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
			_id(id),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_earliestPeerTime(0),
			_config(config) {

			_account = AccountPtr(new Account(dataPath + "/" + _id, readonlyWalletJson));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::vector<PublicKeyRing> &pubKeyRings,
								   uint32_t m,
								   const ConfigPtr &config,
								   const std::string &dataPath,
								   bool p2pEnable,
								   bool singleAddress,
								   bool compatible,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_config(config) {
			ErrorChecker::CheckParam(pubKeyRings.size() < m, Error::InvalidArgument, "Invalid M");

			_account = AccountPtr(new Account(dataPath + "/" + _id, pubKeyRings, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
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
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_config(config) {

			ErrorChecker::CheckParam(cosigners.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_account = AccountPtr(new Account(dataPath + "/" + _id, xprv, payPassword, cosigners, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
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
								   MasterWalletInitFrom from) :
				_id(id),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_config(config) {

			ErrorChecker::CheckParam(cosigners.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_account = AccountPtr(new Account(dataPath + "/" + _id, mnemonic, passphrase, payPasswd, cosigners, m, singleAddress, compatible));
			_account->Save();
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath,
		                                           Mnemonic::WordCount wordCount) {
			return Mnemonic(boost::filesystem::path(rootPath)).Create(language, wordCount);
		}

		void MasterWallet::RemoveLocalStore() {
			_account->Remove();
		}

		std::string MasterWallet::GetID() const {
			return _id;
		}

		std::string MasterWallet::GetWalletID() const {
			return _id;
		}

		std::vector<ISubWallet *> MasterWallet::GetAllSubWallets() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<ISubWallet *> subwallets;
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				subwallets.push_back(it->second);
			}

			std::string result;
			for (size_t i = 0; i < subwallets.size(); ++i)
				result += subwallets[i]->GetChainID() + ",";

			ArgInfo("r => {}", result);
			return subwallets;
		}

		ISubWallet *MasterWallet::GetSubWallet(const std::string &chainID) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("chainID: {}", chainID);

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				return _createdWallets[chainID];
			}

			return nullptr;
		}

		ISubWallet *MasterWallet::CreateSubWallet(const std::string &chainID) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("chainID: {}", chainID);

			ErrorChecker::CheckParamNotEmpty(chainID, "Chain ID");
			ErrorChecker::CheckParam(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				ISubWallet *subWallet = _createdWallets[chainID];
				ArgInfo("r => already created");
				return subWallet;
			}

			ChainConfigPtr chainConfig = _config->GetChainConfig(chainID);
			ErrorChecker::CheckLogic(chainConfig == nullptr, Error::InvalidArgument, "Unsupport chain ID: " + chainID);

			CoinInfoPtr info(new CoinInfo());

			info->SetChainID(chainID);
			info->SetVisibleAsset(Asset::GetELAAssetID());

			ISubWallet *subWallet = SubWalletFactoryMethod(info, chainConfig, this, _config->GetNetType());
			_createdWallets[chainID] = subWallet;
			_account->AddSubWalletInfoList(info);
			_account->Save();

			return subWallet;
		}

		bool MasterWallet::VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("mnemonic: *");
			ArgInfo("passphrase: *");
			bool r = _account->VerifyPrivateKey(mnemonic, passphrase);
			ArgInfo("r => {}", r);
			return r;
		}

		bool MasterWallet::VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("passphrase: *");
			ArgInfo("payPasswd: *");
			bool r = _account->VerifyPassPhrase(passphrase, payPasswd);
			ArgInfo("r => {}", r);
			return r;
		}

		bool MasterWallet::VerifyPayPassword(const std::string &payPasswd) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("payPasswd: *");
			bool r = _account->VerifyPayPassword(payPasswd);
			ArgInfo("r => {}", r);
			return r;
		}

		void MasterWallet::CloseAllSubWallets() {
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				std::string id = _id + ":" + subWallet->GetChainID();
				Log::info("{} closing...", id);
				stopPeerManager(subWallet);

				it = _createdWallets.erase(it);

				delete subWallet;
				subWallet = nullptr;
				Log::info("{} closed", id);
			}
		}

		void MasterWallet::DestroyWallet(const std::string &chainID) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("chainID: {}", chainID);

			if (_createdWallets.find(chainID) == _createdWallets.end())
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "chainID not found");

			SubWallet *subWallet = dynamic_cast<SubWallet *>(_createdWallets[chainID]);
			_account->RemoveSubWalletInfo(subWallet->_info);
			_account->Save();

			stopPeerManager(subWallet);
			_createdWallets.erase(chainID);

			delete subWallet;
			subWallet = nullptr;

			ArgInfo("r => {} {} done", _id, GetFunName());
		}

		nlohmann::json MasterWallet::GetPubKeyInfo() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json j = _account->GetPubKeyInfo();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json MasterWallet::ExportReadonlyWallet() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json j = _account->ExportReadonlyWallet();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json MasterWallet::ExportKeystore(const std::string &backupPassword, const std::string &payPassword) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("backupPassword: *");
			ArgInfo("payPassword: *");
			ErrorChecker::CheckPassword(backupPassword, "Backup");

			std::vector<CoinInfoPtr> coinInfo = _account->SubWalletInfoList();
			for (CoinInfoPtr &info : coinInfo) {
				WalletMap::iterator it = _createdWallets.find(info->GetChainID());
				if (it == _createdWallets.end())
					continue;

				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet == nullptr)
					continue;

				time_t timestamp = subWallet->GetFirstTxnTimestamp();
				if (timestamp > 0)
					info->SetEaliestPeerTime(timestamp);
			}
			_account->SetSubWalletInfoList(coinInfo);
			_account->Save();

			KeyStore keyStore = _account->ExportKeystore(payPassword);
			nlohmann::json j = keyStore.Export(backupPassword, true);

			ArgInfo("r => *");
			return j;
		}

		std::string MasterWallet::ExportMnemonic(const std::string &payPassword) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("payPassword: *");

			std::string mnemonic = _account->ExportMnemonic(payPassword);

			ArgInfo("r => *");
			return mnemonic;
		}

		std::string MasterWallet::ExportPrivateKey(const std::string &payPasswd) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("payPsswd: *");

			ErrorChecker::CheckLogic(_account->Readonly(), Error::UnsupportOperation,
									 "Unsupport operation: read-only wallet do not contain xprv");

			std::string xprv = _account->GetxPrvKeyString(payPasswd);

			ArgInfo("r => *");
			return xprv;
		}

		std::string MasterWallet::ExportMasterPublicKey() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::string mpk = _account->MasterPubKeyString();

			ArgInfo("r => {}", mpk);
			return mpk;
		}

		void MasterWallet::InitSubWallets() {
			const std::vector<CoinInfoPtr> &info = _account->SubWalletInfoList();

			if (info.size() == 0) {
				ChainConfigPtr mainchainConfig = _config->GetChainConfig(CHAINID_MAINCHAIN);
				if (mainchainConfig) {
					CoinInfoPtr defaultInfo(new CoinInfo());
					defaultInfo->SetChainID(CHAINID_MAINCHAIN);
					defaultInfo->SetVisibleAsset(Asset::GetELAAssetID());

					ISubWallet *subWallet = SubWalletFactoryMethod(defaultInfo, mainchainConfig, this, _config->GetNetType());
					ErrorChecker::CheckCondition(subWallet == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					_createdWallets[subWallet->GetChainID()] = subWallet;

					_account->AddSubWalletInfoList(defaultInfo);
					_account->Save();
				}
			} else {
				for (int i = 0; i < info.size(); ++i) {
					ChainConfigPtr chainConfig = _config->GetChainConfig(info[i]->GetChainID());
					if (chainConfig == nullptr) {
						Log::error("Can not find config of chain ID: " + info[i]->GetChainID());
						continue;
					}

					ISubWallet *subWallet = SubWalletFactoryMethod(info[i], chainConfig, this, _config->GetNetType());
					ErrorChecker::CheckCondition(subWallet == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					_createdWallets[subWallet->GetChainID()] = subWallet;
				}
			}
		}

		ISubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfoPtr &info, const ChainConfigPtr &config,
														MasterWallet *parent, const std::string &netType) {

			if (_initFrom == CreateNormal) {
				Log::info("Create new master wallet");
				info->SetEaliestPeerTime(config->ChainParameters()->LastCheckpoint().Timestamp());
			} else if (_initFrom == CreateMultiSign) {
				if (_earliestPeerTime != 0) {
					info->SetEaliestPeerTime(_earliestPeerTime);
				} else {
					info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
				}
				Log::info("Create new multi-sign master wallet");
			} else if (_initFrom == ImportFromMnemonic) {
				if (_earliestPeerTime != 0) {
					info->SetEaliestPeerTime(_earliestPeerTime);
				} else {
					info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
				}
				Log::info("Import master wallet with mnemonic");
			} else if (_initFrom == ImportFromKeyStore) {
				Log::info("Master wallet import with keystore");
			} else if (_initFrom == ImportFromLocalStore) {
				Log::info("Master wallet init from local store");
			} else {
				Log::error("Should not be here");
				info->SetEaliestPeerTime(config->ChainParameters()->FirstCheckpoint().Timestamp());
			}
			Log::info("{}:{} Ealiest peer time: {}", _id, info->GetChainID(), info->GetEarliestPeerTime());

			if (info->GetChainID() == "ELA") {
				return new MainchainSubWallet(info, config, parent, netType);
			} else if (info->GetChainID() == "IDChain") {
				return new IDChainSubWallet(info, config, parent, netType);
			} else if (info->GetChainID() == "TokenChain") {
				return new TokenchainSubWallet(info, config, parent, netType);
			} else if (info->GetChainID() == "ETHSC") {
				return new EthSidechainSubWallet(info, config, parent, netType);
			} else {
				ErrorChecker::ThrowLogicException(Error::InvalidChainID, "Invalid chain ID: " + info->GetChainID());
			}

			return nullptr;
		}

		std::string MasterWallet::GetDataPath() const {
			return _account->GetDataPath();
		}

		AccountPtr MasterWallet::GetAccount() const {
			return _account;
		}

		void MasterWallet::startPeerManager(SubWallet *wallet) {
			if (_p2pEnable)
				wallet->StartP2P();
		}

		void MasterWallet::stopPeerManager(SubWallet *wallet) {
			if (_p2pEnable)
				wallet->StopP2P();
		}

		bool MasterWallet::IsAddressValid(const std::string &address) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("addr: {}", address);

			bool valid = Address(address).Valid();
			if (!valid) {
				valid = addressValidateString(address.c_str()) == ETHEREUM_BOOLEAN_TRUE;
			}

			ArgInfo("r => {}", valid);
			return valid;
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<std::string> chainIDs = _config->GetAllChainIDs();

			std::string result;
			for (size_t i = 0; i < chainIDs.size(); ++i) {
				result += chainIDs[i] + ", ";
			}

			ArgInfo("r => {}", result);
			return chainIDs;
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("old: *");
			ArgInfo("new: *");

			_account->ChangePassword(oldPassword, newPassword);
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json info = _account->GetBasicInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _account->Equal(wallet._account);
		}

		void MasterWallet::FlushData() {
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet*>(it->second);
				subWallet->FlushData();
			}
		}

		ChainConfigPtr MasterWallet::GetChainConfig(const std::string &chainID) const {
			return _config->GetChainConfig(chainID);
		}

	}
}
