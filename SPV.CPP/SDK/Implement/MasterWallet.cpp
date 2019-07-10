// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "SidechainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SubWallet.h"
#include "MasterWallet.h"
#include "TokenchainSubWallet.h"

#include <SDK/Plugin/Transaction/Payload/RegisterIdentification.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/SpvService/Config.h>
#include <CMakeConfig.h>

#include <vector>
#include <boost/filesystem.hpp>

#define COIN_COINFIG_FILE "CoinConfig.json"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(0) {

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id));
			_account = AccountPtr(new Account(_localStore));

			if (_account->GetSignType() == Account::MultiSign)
				_idAgentImpl = nullptr;
			else
				_idAgentImpl = boost::shared_ptr<IDAgentImpl>(new IDAgentImpl(this));
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &passphrase,
								   const std::string &payPassword,
								   bool singleAddress,
								   bool p2pEnable,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(earliestPeerTime),
				_initFrom(from) {

			Mnemonic m(_rootPath);
			ErrorChecker::CheckLogic(!m.Validate(mnemonic), Error::Mnemonic, "Invalid mnemonic");

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, mnemonic, passphrase,
													   singleAddress, payPassword));
			_account = AccountPtr(new Account(_localStore));

			_idAgentImpl = boost::shared_ptr<IDAgentImpl>(new IDAgentImpl(this));

			_localStore->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPassword,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_earliestPeerTime(0),
				_initFrom(from) {

			KeyStore keystore;
			keystore.Import(keystoreContent, backupPassword);

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, keystore.WalletJson(), payPassword));
			_account = AccountPtr(new Account(_localStore));

			if (_account->GetSignType() == Account::MultiSign) {
				_idAgentImpl = nullptr;
			} else {
				_idAgentImpl = boost::shared_ptr<IDAgentImpl>(new IDAgentImpl(this));
			}

			_localStore->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &readonlyWalletJson,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
			_id(id),
			_rootPath(rootPath),
			_dataPath(dataPath),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_earliestPeerTime(0) {

			KeyStore keyStore;
			keyStore.ImportReadonly(readonlyWalletJson);

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, keyStore.WalletJson(), ""));
			_account = AccountPtr(new Account(_localStore));

			if (_account->GetSignType() == Account::MultiSign) {
				_idAgentImpl = nullptr;
			} else {
				_idAgentImpl = boost::shared_ptr<IDAgentImpl>(new IDAgentImpl(this));
			}

			_localStore->Save();
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &publicKeys, uint32_t m,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   bool p2pEnable,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_idAgentImpl(nullptr) {
			ErrorChecker::CheckPubKeyJsonArray(publicKeys, 1, "coSigner");
			ErrorChecker::CheckParam(publicKeys.size() < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, publicKeys, m));
			_account = AccountPtr(new Account(_localStore));

			_localStore->Save();
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &xprv, const std::string &payPassword,
								   const nlohmann::json &publicKeys, uint32_t m,
								   const std::string &rootPath, const std::string &dataPath,
								   bool p2pEnable, time_t earliestPeerTime, MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_idAgentImpl(nullptr) {

			ErrorChecker::CheckPubKeyJsonArray(publicKeys, 1, "coSigner");
			ErrorChecker::CheckParam(publicKeys.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, xprv, true, payPassword));
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				_localStore->AddPublicKeyRing(PublicKeyRing((*it).get<std::string>()));
			}

			_localStore->SetM(m);
			_localStore->SetN(_localStore->GetPublicKeyRing().size());
			_account = AccountPtr(new Account(_localStore));

			_localStore->Save();
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &mnemonic,
								   const std::string &passphrase, const std::string &payPassword,
								   const nlohmann::json &publicKeys, uint32_t m,
								   bool p2pEnable,
								   const std::string &rootPath,
								   const std::string &dataPath,
								   time_t earliestPeerTime,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_earliestPeerTime(earliestPeerTime),
				_idAgentImpl(nullptr) {

			ErrorChecker::CheckPubKeyJsonArray(publicKeys, 1, "coSigner");
			ErrorChecker::CheckParam(publicKeys.size() + 1 < m, Error::InvalidArgument, "Invalid M");

			_config = ConfigPtr(new Config(_rootPath));
			_localStore = LocalStorePtr(new LocalStore(_dataPath + "/" + _id, mnemonic, passphrase, true, payPassword));
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it)
				_localStore->AddPublicKeyRing(PublicKeyRing((*it).get<std::string>()));

			_localStore->SetM(m);
			_localStore->SetN(_localStore->GetPublicKeyRing().size());
			_account = AccountPtr(new Account(_localStore));

			_localStore->Save();
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath,
		                                           Mnemonic::WordCount wordCount) {
			return Mnemonic(boost::filesystem::path(rootPath)).Create(language, wordCount);
		}

		void MasterWallet::RemoveLocalStore() {
			boost::filesystem::path path = _rootPath;
			path /= GetID();
			if (boost::filesystem::exists(path))
				boost::filesystem::remove_all(path);
		}

		std::string MasterWallet::GetID() const {
			ArgInfo("{} {}", _id, GetFunName());
			return _id;
		}

		std::string MasterWallet::GetWalletID() const {
			return _id;
		}

		std::vector<ISubWallet *> MasterWallet::GetAllSubWallets() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<ISubWallet *> result;
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				result.push_back(it->second);
			}

			std::string chainIDs = "";
			for (size_t i = 0; i < result.size(); ++i) {
				SubWallet *wallet = dynamic_cast<SubWallet *>(result[i]);
				chainIDs += wallet->GetInfoChainID() + ",";
			}

			ArgInfo("r => size: {} list: {}", result.size(), chainIDs);

			return result;
		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, uint64_t feePerKB) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("chainID: {}", chainID);
			ArgInfo("feePerKB: {}", feePerKB);

			ErrorChecker::CheckParamNotEmpty(chainID, "Chain ID");
			ErrorChecker::CheckParam(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");


			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				ISubWallet *subWallet = _createdWallets[chainID];
				ArgInfo("r => already created {}, 0x{:x}", dynamic_cast<SubWallet *>(subWallet)->GetInfoChainID(),
						(long)subWallet);
				return subWallet;
			}

			ChainConfigPtr chainConfig = _config->GetChainConfig(chainID);
			ErrorChecker::CheckLogic(chainConfig == nullptr, Error::InvalidArgument, "Unsupport chain ID: " + chainID);

			CoinInfoPtr info(new CoinInfo());

			info->SetChainID(chainID);
			if (feePerKB > chainConfig->FeePerKB())
				info->SetFeePerKB(feePerKB);
			else
				info->SetFeePerKB(chainConfig->FeePerKB());
			info->SetVisibleAsset(Asset::GetELAAssetID());

			_localStore->AddSubWalletInfoList(info);
			SubWallet *subWallet = SubWalletFactoryMethod(info, chainConfig, this);
			_createdWallets[chainID] = subWallet;
			_localStore->Save();
			startPeerManager(subWallet);

			ArgInfo("r => {}, 0x{:x}", subWallet->GetInfoChainID(), (long)subWallet);

			return subWallet;
		}

		void MasterWallet::CloseAllSubWallets() {
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				stopPeerManager(subWallet);

				it = _createdWallets.erase(it);

				delete subWallet;
			}
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			ErrorChecker::CheckParam(wallet == nullptr, Error::Wallet, "Destroy wallet can't be null");
			ErrorChecker::CheckParam(_createdWallets.empty(), Error::Wallet, "There is no sub wallet in this wallet.");

			SubWallet *subWallet = dynamic_cast<SubWallet *>(wallet);
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("subWallet: {}, 0x{:x}", subWallet->GetInfoChainID(), (long)wallet);

			if (_createdWallets.find(subWallet->GetInfoChainID()) == _createdWallets.end())
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Sub wallet did not created");

			if (_createdWallets[subWallet->GetInfoChainID()] != wallet)
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Sub wallet does not belong to this master wallet");

			_localStore->RemoveSubWalletInfo(subWallet->_info);
			_localStore->Save();

			stopPeerManager(subWallet);
			WalletMap::iterator it = _createdWallets.find(subWallet->GetInfoChainID());
			_createdWallets.erase(it);

			delete subWallet;

			ArgInfo("{} {} done", _id, GetFunName());
		}

		std::string MasterWallet::GetPublicKey() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::string publicKey = _localStore->GetRequestPubKey();
			ArgInfo("r => {}", publicKey);
			return publicKey;
		}

		nlohmann::json MasterWallet::ExportReadonlyKeyStore() {
			KeyStore keyStore;

			_localStore->GetReadOnlyWalletJson(keyStore.WalletJson());

			return keyStore.ExportReadonly();
		}

		nlohmann::json MasterWallet::exportKeyStore(const std::string &backupPassword,
													const std::string &payPassword) {
			KeyStore keyStore;

			_localStore->GetWalletJson(keyStore.WalletJson(), payPassword);

			return keyStore.Export(backupPassword, true);
		}

		std::string MasterWallet::exportMnemonic(const std::string &payPassword) {
			std::string encryptedMnemonic = _localStore->GetMnemonic();
			bytes_t bytes = AES::DecryptCCM(encryptedMnemonic, payPassword);
			return std::string((char *)bytes.data(), bytes.size());
		}

		std::string MasterWallet::ExportxPrivateKey(const std::string &payPasswd) const {
			ErrorChecker::CheckLogic(_localStore->Readonly(), Error::UnsupportOperation,
									 "Unsupport operation: read-only wallet do not contain xprv");

			if (_localStore->GetxPrivKey().empty())
				_localStore->RegenerateKey(payPasswd);

			ErrorChecker::CheckLogic(_localStore->GetxPrivKey().empty(), Error::InvalidLocalStore, "xprv is empty");

			bytes_t bytes = AES::DecryptCCM(_localStore->GetxPrivKey(), payPasswd);

			return Base58::CheckEncode(bytes);
		}

		std::string MasterWallet::ExportMasterPublicKey() const {
			if (_localStore->GetxPubKey().empty()) {
				ErrorChecker::ThrowLogicException(Error::UnsupportOperation,
												  "Unsupport operation: xpub is empty");
			}

			return _localStore->GetxPubKey();
		}

		void MasterWallet::InitSubWallets() {
			const std::vector<CoinInfoPtr> &info = _localStore->GetSubWalletInfoList();

			if (info.size() == 0) {
				const ChainConfigPtr &mainchainConfig = _config->GetChainConfig("ELA");
				if (mainchainConfig) {
					CoinInfoPtr defaultInfo(new CoinInfo());
					defaultInfo->SetChainID(mainchainConfig->ID());
					defaultInfo->SetFeePerKB(mainchainConfig->FeePerKB());
					defaultInfo->SetVisibleAsset(Asset::GetELAAssetID());

					ISubWallet *subWallet = SubWalletFactoryMethod(defaultInfo, mainchainConfig, this);
					SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
					ErrorChecker::CheckCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					startPeerManager(subWalletImpl);
					_createdWallets[subWalletImpl->GetInfoChainID()] = subWallet;

					_localStore->AddSubWalletInfoList(defaultInfo);
					_localStore->Save();
				}
			} else {
				for (int i = 0; i < info.size(); ++i) {
					const ChainConfigPtr &chainConfig = _config->GetChainConfig(info[i]->GetChainID());
					if (chainConfig == nullptr) {
						Log::error("Can not find config of chain ID: " + info[i]->GetChainID());
						continue;
					}

					ISubWallet *subWallet = SubWalletFactoryMethod(info[i], chainConfig, this);
					SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
					ErrorChecker::CheckCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
												 "Recover sub wallet error");
					startPeerManager(subWalletImpl);
					_createdWallets[subWalletImpl->GetInfoChainID()] = subWallet;
				}
			}
		}


		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("msg: {}", message);
			ArgInfo("payPasswd: {}", "*");

			ErrorChecker::CheckParamNotEmpty(message, "Sign message");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			Key key = _account->RequestPrivKey(payPassword);
			std::string hex = key.Sign(message).getHex();

			ArgInfo("r => {}", hex);
			return hex;
		}

		bool MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("pubkey: {}", publicKey);
			ArgInfo("msg: {}", message);
			ArgInfo("sign: {}", signature);

			Key key;
			key.SetPubKey(bytes_t(publicKey));
			bool result = key.Verify(message, bytes_t(signature));

			ArgInfo("r => {}", result);
			return result;
		}

		bool MasterWallet::IsIDValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfoPtr &info, const ChainConfigPtr &config,
														MasterWallet *parent) {

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
			Log::info("Ealiest peer time: {}", info->GetEarliestPeerTime());

			if (info->GetChainID() == "ELA") {
				return new MainchainSubWallet(info, config, parent);
			} else if (info->GetChainID() == "IDChain") {
				return new IDChainSubWallet(info, config, parent);
			} else if (info->GetChainID() == "TokenChain") {
				return new TokenchainSubWallet(info, config, parent);
			} else {
				ErrorChecker::ThrowLogicException(Error::InvalidChainID, "Invalid chain ID: " + info->GetChainID());
			}

			return nullptr;
		}

		std::string
		MasterWallet::DeriveIDAndKeyForPurpose(uint32_t purpose, uint32_t index) {
			return _idAgentImpl->DeriveIDAndKeyForPurpose(purpose, index).String();
		}

		nlohmann::json
		MasterWallet::GenerateProgram(const std::string &id, const std::string &message, const std::string &password) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("id: {}", id);
			ArgInfo("msg: {}", message);
			ArgInfo("passwd: {}", "*");

			RegisterIdentification payload;
			nlohmann::json payLoadJson = nlohmann::json::parse(message);
			payload.FromJson(payLoadJson, 0);

			ByteStream ostream;
			payload.Serialize(ostream, 0);

			nlohmann::json j;
			bytes_t signedData = _idAgentImpl->Sign(id, ostream.GetBytes(), password);

			ostream.Reset();
			ostream.WriteVarBytes(signedData);
			j["Parameter"] = ostream.GetBytes().getHex();
			j["Code"] = _idAgentImpl->GenerateRedeemScript(id, password);

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string MasterWallet::Sign(const std::string &id, const std::string &message, const std::string &password) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("id: {}", id);
			ArgInfo("msg: {}", message);
			ArgInfo("payPasswd: {}", "*");

			ErrorChecker::CheckParamNotEmpty(id, "Master wallet id");
			ErrorChecker::CheckParamNotEmpty(message, "Master wallet sign message");
			ErrorChecker::CheckPassword(password, "Master wallet sign");

			std::string data = _idAgentImpl->Sign(id, message, password);

			ArgInfo("r => {}", data);

			return data;
		}

		std::vector<std::string> MasterWallet::GetAllIDs() const {
			if (_idAgentImpl == nullptr)
				return std::vector<std::string>();

			return _idAgentImpl->GetAllIDs();
		}

		std::string MasterWallet::GetPublicKey(const std::string &id) const {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("id: {}", id);

			std::string pubkey = _idAgentImpl->GetPublicKey(id).getHex();

			ArgInfo("r => {}", pubkey);
			return pubkey;
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

			bool result = Address(address).Valid();

			ArgInfo("r => {}", result);
			return result;
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() const {
			ArgInfo("{} {}", _id, GetFunName());

			std::vector<std::string> chainIDs;

			const std::vector<ChainConfigPtr> &chainConfigs = _config->GetChainConfigs();
			for (size_t i = 0; i < chainConfigs.size(); ++i)
				chainIDs.push_back(chainConfigs[i]->ID());

			std::string chainID = "";
			for (size_t i = 0; i < chainIDs.size(); ++i) {
				chainID += chainIDs[i] + ", ";
			}

			ArgInfo("r => size: {} list: {}", chainIDs.size(), chainID);
			return chainIDs;
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ArgInfo("{} {}", _id, GetFunName());
			ArgInfo("old: {}", "*");
			ArgInfo("new: {}", "*");

			_account->ChangePassword(oldPassword, newPassword);
		}

		IIDAgent *MasterWallet::GetIIDAgent() {
			ArgInfo("{} {}", _id, GetFunName());

			ArgInfo("r => 0x{:x}", (long)this);
			return this;
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _id, GetFunName());

			nlohmann::json info = _account->GetBasicInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _account->Equal(*wallet._account);
		}

	}
}
