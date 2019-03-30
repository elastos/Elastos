// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdChainSubWallet.h"
#include "SidechainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SubWallet.h"
#include "MasterWallet.h"

#include <SDK/Plugin/Transaction/Payload/PayloadRegisterIdentification.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <Config.h>

#include <vector>
#include <boost/filesystem.hpp>

#define COIN_COINFIG_FILE "CoinConfig.json"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from) {
			_localStore = LocalStorePtr(new LocalStore(_rootPath + "/" + _id));
			_account = AccountPtr(new Account(_localStore, _rootPath));

			if (_account->GetSignType() > Account::MultiSign)
				_idAgentImpl = nullptr;
			else
				_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this));
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &passphrase,
								   const std::string &payPassword,
								   bool singleAddress,
								   bool p2pEnable,
								   const std::string &rootPath,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from) {

			Mnemonic m(_rootPath);
			ErrorChecker::CheckLogic(!m.Validate(mnemonic), Error::Mnemonic, "Invalid mnemonic");

			_localStore = LocalStorePtr(new LocalStore(_rootPath + "/" + _id, mnemonic, passphrase,
													   singleAddress, payPassword));
			_account = AccountPtr(new Account(_localStore, _rootPath));

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this));
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPassword,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from) {

			KeyStore keystore;
			keystore.Import(keystoreContent, backupPassword);

			_localStore = LocalStorePtr(new LocalStore(_rootPath + "/" + _id, keystore.WalletJson(), payPassword));
			_account = AccountPtr(new Account(_localStore, _rootPath));

			if (_account->GetSignType() > Account::MultiSign) {
				_idAgentImpl = nullptr;
			} else {
				_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this));
			}
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &coSigners, uint32_t requiredSignCount,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_idAgentImpl(nullptr) {
			ErrorChecker::CheckPubKeyJsonArray(coSigners, 1, "coSigner");
			ErrorChecker::CheckParam(coSigners.size() < requiredSignCount, Error::InvalidArgument, "Invalid M");

			_localStore = LocalStorePtr(new LocalStore(_rootPath + "/" + _id, coSigners, requiredSignCount));
			_account = AccountPtr(new Account(_localStore, _rootPath));
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &privKey, const std::string &payPassword,
								   const nlohmann::json &coSigners, uint32_t requiredSignCount,
								   const std::string &rootPath, bool p2pEnable, MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_idAgentImpl(nullptr) {
			ErrorChecker::ThrowLogicException(Error::KeyStore, "Deprecated interface: will refactor later");
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &mnemonic,
								   const std::string &passphrase, const std::string &payPassword,
								   const nlohmann::json &coSigners,
								   uint32_t requiredSignCount, bool p2pEnable, const std::string &rootPath,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_idAgentImpl(nullptr) {

			ErrorChecker::CheckPubKeyJsonArray(coSigners, 1, "coSigner");
			ErrorChecker::CheckParam(coSigners.size() + 1 < requiredSignCount, Error::InvalidArgument, "Invalid M");

			_localStore = LocalStorePtr(new LocalStore(_rootPath + "/" + _id, mnemonic, passphrase, true, payPassword));
			for (nlohmann::json::const_iterator it = coSigners.cbegin(); it != coSigners.cend(); ++it)
				_localStore->AddPublicKeyRing(PublicKeyRing((*it).get<std::string>()));

			_localStore->SetM(requiredSignCount);
			_localStore->SetN(_localStore->GetPublicKeyRing().size());
			_account = AccountPtr(new Account(_localStore, _rootPath));
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath) {
			return Mnemonic(boost::filesystem::path(rootPath)).Create(language);
		}

		void MasterWallet::ClearLocal() {
			boost::filesystem::path path = _rootPath;
			path /= GetId();
			if (boost::filesystem::exists(path))
				boost::filesystem::remove_all(path);

		}

		void MasterWallet::Save() {

			_localStore->ClearSubWalletInfoList();
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet == nullptr) continue;

				_localStore->AddSubWalletInfoList(subWallet->getCoinInfo());
			}

			_localStore->Save();
		}

		std::string MasterWallet::GetId() const {
			return _id;
		}

		std::vector<ISubWallet *> MasterWallet::GetAllSubWallets() const {

			std::vector<ISubWallet *> result;
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				result.push_back(it->second);
			}

			return result;
		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, uint64_t feePerKb) {

			ErrorChecker::CheckParamNotEmpty(chainID, "Chain ID");
			ErrorChecker::CheckParam(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				return _createdWallets[chainID];
			}

			CoinInfo info;
			tryInitCoinConfig();
			CoinConfig coinConfig = _coinConfigReader.FindConfig(chainID);
			info.SetWalletType(coinConfig.Type);
			info.SetIndex(coinConfig.Index);
			info.SetMinFee(coinConfig.MinFee);
			info.SetGenesisAddress(coinConfig.GenesisAddress);
			info.SetEnableP2P(coinConfig.EnableP2P);
			info.SetReconnectSeconds(coinConfig.ReconnectSeconds);

			info.SetChainId(chainID);
			info.SetFeePerKb(feePerKb);

			SubWallet *subWallet = SubWalletFactoryMethod(info, coinConfig, ChainParams(coinConfig), this);
			_createdWallets[chainID] = subWallet;
			startPeerManager(subWallet);
			Save();
			return subWallet;
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			ErrorChecker::CheckParam(wallet == nullptr, Error::Wallet, "Destroy wallet can't be null");
			ErrorChecker::CheckParam(_createdWallets.empty(), Error::Wallet, "There is no sub wallet in this wallet.");

			if (std::find_if(_createdWallets.begin(), _createdWallets.end(),
							 [wallet](const WalletMap::value_type &item) {
								 return item.second == wallet;
							 }) == _createdWallets.end())
				ErrorChecker::CheckCondition(true, Error::Wallet,
											 "Specified sub wallet is not belong to current master wallet.");

			SubWallet *walletInner = dynamic_cast<SubWallet *>(wallet);
			assert(walletInner != nullptr);
			stopPeerManager(walletInner);

			_createdWallets.erase(std::find_if(_createdWallets.begin(), _createdWallets.end(),
											   [wallet](const WalletMap::value_type &item) {
												   return item.second == wallet;
											   }));
			delete walletInner;
		}

		std::string MasterWallet::GetPublicKey() const {
			return _localStore->GetRequestPubKey();
		}

		nlohmann::json MasterWallet::exportKeyStore(const std::string &backupPassword,
													const std::string &payPassword,
													bool withPrivKey) {
			KeyStore keyStore;

			_localStore->GetWalletJson(keyStore.WalletJson(), payPassword);

			keyStore.WalletJson().ClearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(),
						  [&keyStore](const WalletMap::value_type &item) {
							  SubWallet *subWallet = dynamic_cast<SubWallet *>(item.second);
							  keyStore.WalletJson().AddCoinInfo(subWallet->_info);
						  });

			Save();

			return keyStore.Export(backupPassword, withPrivKey);
		}

		std::string MasterWallet::exportMnemonic(const std::string &payPassword) {
			std::string encryptedMnemonic = _localStore->GetMnemonic();
			bytes_t bytes = AES::DecryptCCM(encryptedMnemonic, payPassword);
			return std::string((char *)bytes.data(), bytes.size());
		}

		void MasterWallet::InitSubWallets() {
			const std::vector<CoinInfo> &coinInfoList = _localStore->GetSubWalletInfoList();

			tryInitCoinConfig();

			for (int i = 0; i < coinInfoList.size(); ++i) {
				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].GetChainId());
				ISubWallet *subWallet = SubWalletFactoryMethod(coinInfoList[i], coinConfig,
															   ChainParams(coinConfig), this);
				SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
				ErrorChecker::CheckCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
											 "Recover sub wallet error");
				startPeerManager(subWalletImpl);
				_createdWallets[subWallet->GetChainId()] = subWallet;
			}
			Save();
		}


		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {

			ErrorChecker::CheckParamNotEmpty(message, "Sign message");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			Key key = _account->RequestPrivKey(payPassword);
			return key.Sign(message).getHex();
		}

		bool MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {

			Key key;
			key.SetPubKey(bytes_t(publicKey));
			return key.Verify(message, bytes_t(signature));
		}

		bool MasterWallet::IsIdValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfo &info, const CoinConfig &config,
														const ChainParams &chainParams,
														MasterWallet *parent) {

			CoinInfo fixedInfo = info;

			if (_initFrom == CreateNormal) {
				fixedInfo.SetEaliestPeerTime(chainParams.GetLastCheckpoint().GetTimestamp());
			} else if (_initFrom == CreateMultiSign || _initFrom == ImportFromMnemonic) {
				fixedInfo.SetEaliestPeerTime(chainParams.GetFirstCheckpoint().GetTimestamp());
			} else if (_initFrom == ImportFromKeyStore || _initFrom == ImportFromLocalStore) {
				fixedInfo.SetEaliestPeerTime(info.GetEarliestPeerTime());
			} else {
				fixedInfo.SetEaliestPeerTime(chainParams.GetFirstCheckpoint().GetTimestamp());
			}

			std::vector<uint256> visibleAssets;
			visibleAssets.push_back(Asset::GetELAAssetID());
			fixedInfo.SetVisibleAssets(visibleAssets);

			Log::info("Master wallet init from {}, ealiest peer time = {}", _initFrom,
					  fixedInfo.GetEarliestPeerTime());

			fixedInfo.SetGenesisAddress(config.GenesisAddress);

			switch (fixedInfo.GetWalletType()) {
				case Mainchain:
					return new MainchainSubWallet(fixedInfo, chainParams, config.PluginType, parent);
				case Sidechain:
					return new SidechainSubWallet(fixedInfo, chainParams, config.PluginType, parent);
				case Idchain:
					return new IdChainSubWallet(fixedInfo, chainParams, config.PluginType, parent);
				case Normal:
				default:
					return new SubWallet(fixedInfo, chainParams, config.PluginType, parent);
			}
		}

		std::string
		MasterWallet::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index) {
			return _idAgentImpl->DeriveIdAndKeyForPurpose(purpose, index).String();
		}

		nlohmann::json
		MasterWallet::GenerateProgram(const std::string &id, const std::string &message, const std::string &password) {
			PayloadRegisterIdentification payload;
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
			return j;
		}

		std::string MasterWallet::Sign(const std::string &id, const std::string &message, const std::string &password) {
			ErrorChecker::CheckParamNotEmpty(id, "Master wallet id");
			ErrorChecker::CheckParamNotEmpty(message, "Master wallet sign message");
			ErrorChecker::CheckPassword(password, "Master wallet sign");

			return _idAgentImpl->Sign(id, message, password);
		}

		std::vector<std::string> MasterWallet::GetAllIds() const {
			if (_idAgentImpl == nullptr)
				return std::vector<std::string>();

			return _idAgentImpl->GetAllIds();
		}

		std::string MasterWallet::GetPublicKey(const std::string &id) const {
			return _idAgentImpl->GetPublicKey(id).getHex();
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
			return Address(address).Valid();
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() const {
			tryInitCoinConfig();
			return _coinConfigReader.GetAllChainId();
		}

		void MasterWallet::tryInitCoinConfig() const {
			if (!_coinConfigReader.IsInitialized()) {
				boost::filesystem::path configPath = _rootPath;
				configPath /= COIN_COINFIG_FILE;
				ErrorChecker::CheckPathExists(configPath);
				_coinConfigReader.Load(configPath);
			}
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			_account->ChangePassword(oldPassword, newPassword);
		}

		IIdAgent *MasterWallet::GetIIdAgent() {
			return this;
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = _account->GetBasicInfo();
			return j;
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _account->Equal(*wallet._account);
		}

	}
}
