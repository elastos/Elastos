// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdChainSubWallet.h"
#include "SidechainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SubWallet.h"
#include "MasterWallet.h"

#include <SDK/Account/SubAccountGenerator.h>
#include <SDK/Crypto/MasterPubKey.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterIdentification.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/BIPs/BIP32Sequence.h>
#include <Config.h>

#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRCrypto.h>

#include <vector>
#include <boost/filesystem.hpp>
#include <SDK/Common/Base58.h>

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define COIN_COINFIG_FILE "CoinConfig.json"
#define BIP32_SEED_KEY "Bitcoin seed"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const boost::filesystem::path &localStore,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_localStore(rootPath) {

			_localStore.Load(localStore);
			_id = localStore.parent_path().filename().string();

			initFromLocalStore(_localStore);
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &mnemonic,
								   const std::string &phrasePassword,
								   const std::string &payPassword,
								   bool singleAddress,
								   bool p2pEnable,
								   const std::string &rootPath,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_localStore(rootPath) {

			_localStore.IsSingleAddress() = singleAddress;
			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromMnemonic(mnemonic, phrasePassword, payPassword);
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &phrasePassword,
								   const std::string &payPassword,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
			_id(id),
			_rootPath(rootPath),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_localStore(rootPath) {

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromKeyStore(keystoreContent, backupPassword, payPassword, phrasePassword);
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
				_initFrom(from),
				_localStore(rootPath) {

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromKeyStore(keystoreContent, backupPassword, payPassword);
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
				_localStore(rootPath),
				_idAgentImpl(nullptr) {
			initFromMultiSigners("", "", coSigners, requiredSignCount);
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &privKey, const std::string &payPassword,
								   const nlohmann::json &coSigners, uint32_t requiredSignCount,
								   const std::string &rootPath, bool p2pEnable, MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_localStore(rootPath),
				_idAgentImpl(nullptr) {
			initFromMultiSigners(privKey, payPassword, coSigners, requiredSignCount);
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &mnemonic,
								   const std::string &phrasePassword, const std::string &payPassword,
								   const nlohmann::json &coSigners,
								   uint32_t requiredSignCount, bool p2pEnable, const std::string &rootPath,
								   MasterWalletInitFrom from) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_initFrom(from),
				_localStore(rootPath) {
			initFromMultiSigners(mnemonic, phrasePassword, payPassword, coSigners, requiredSignCount);
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath) {
			UInt128 entropy;
			Mnemonic mnemonic(language, boost::filesystem::path(rootPath));

			for (size_t i = 0; i < sizeof(entropy); ++i) {
				entropy.u8[i] = Utils::getRandomByte();
			}
			const std::vector<std::string> &words = mnemonic.words();
			const char *wordList[words.size()];
			for (size_t i = 0; i < words.size(); i++) {
				wordList[i] = words[i].c_str();
			}
			size_t phraselen = BRBIP39Encode(nullptr, 0, wordList, entropy.u8, sizeof(entropy));

			char phrase[phraselen];
			BRBIP39Encode(phrase, phraselen, wordList, entropy.u8, sizeof(entropy));

			return phrase;
		}

		void MasterWallet::ClearLocal() {
			boost::filesystem::path path = _rootPath;
			path /= GetId();
			if (boost::filesystem::exists(path))
				boost::filesystem::remove_all(path);

		}

		void MasterWallet::Save() {
			restoreLocalStore();

			boost::filesystem::path path = _rootPath;
			path /= GetId();
			if (!boost::filesystem::exists(path))
				boost::filesystem::create_directory(path);
			path /= MASTER_WALLET_STORE_FILE;
			_localStore.Save(path);
		}

		MasterPubKeyPtr MasterWallet::GetMasterPubKey(const std::string &chainID) const {
			return _localStore.GetMasterPubKey(chainID);
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

			ParamChecker::checkParamNotEmpty(chainID, "Chain ID");
			ParamChecker::checkParam(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");

			//todo limit coinTypeIndex and feePerKb if needed in future

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				return _createdWallets[chainID];
			}

			CoinInfo info;
			tryInitCoinConfig();
			CoinConfig coinConfig = _coinConfigReader.FindConfig(chainID);
			info.setWalletType(coinConfig.Type);
			info.setIndex(coinConfig.Index);
			info.setMinFee(coinConfig.MinFee);
			info.setGenesisAddress(coinConfig.GenesisAddress);
			info.setEnableP2P(coinConfig.EnableP2P);
			info.setReconnectSeconds(coinConfig.ReconnectSeconds);

			info.setSingleAddress(_localStore.IsSingleAddress());
			info.setUsedMaxAddressIndex(0);
			info.setChainId(chainID);
			info.setFeePerKb(feePerKb);

			SubWallet *subWallet = SubWalletFactoryMethod(info, coinConfig, ChainParams(coinConfig), this);
			_createdWallets[chainID] = subWallet;
			startPeerManager(subWallet);
			Save();
			return subWallet;
		}

		void MasterWallet::restoreSubWallets(const std::vector<CoinInfo> &coinInfoList) {

			for (int i = 0; i < coinInfoList.size(); ++i) {
				if (_createdWallets.find(coinInfoList[i].getChainId()) != _createdWallets.end()) continue;

				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].getChainId());
				_createdWallets[coinInfoList[i].getChainId()] =
						SubWalletFactoryMethod(coinInfoList[i], coinConfig, ChainParams(coinConfig), this);
			}
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			ParamChecker::checkParam(wallet == nullptr, Error::Wallet, "Destroy wallet can't be null");
			ParamChecker::checkParam(_createdWallets.empty(), Error::Wallet, "There is no sub wallet in this wallet.");

			if (std::find_if(_createdWallets.begin(), _createdWallets.end(),
							 [wallet](const WalletMap::value_type &item) {
								 return item.second == wallet;
							 }) == _createdWallets.end())
				ParamChecker::checkCondition(true, Error::Wallet,
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
			return Utils::encodeHex(_localStore.Account()->GetMultiSignPublicKey());
		}

		// to support old web keystore
		void MasterWallet::importFromKeyStore(const nlohmann::json &keystoreContent, const std::string &backupPassword,
											  const std::string &payPassword, const std::string &phrasePassword) {

			KeyStore keyStore(_rootPath);

			ParamChecker::checkCondition(!keyStore.Import(keystoreContent, backupPassword, phrasePassword),
										 Error::WrongPasswd, "Wrong backup password");

			ParamChecker::checkCondition(!keyStore.isOld(), Error::KeyStore,
										 "This interface use for support old keystore");
			ParamChecker::checkCondition(!keyStore.HasPhrasePassword(), Error::KeyStore,
										 "This keystore should has phrase password");

			Log::getLogger()->info("Import from old keystore");
			_initFrom = ImportFromOldKeyStore;

			initFromKeyStore(keyStore, payPassword);
		}

		void MasterWallet::importFromKeyStore(const nlohmann::json &keystoreContent, const std::string &backupPassword,
											  const std::string &payPassword) {

			KeyStore keyStore(_rootPath);
			ParamChecker::checkCondition(!keyStore.Import(keystoreContent, backupPassword), Error::WrongPasswd,
										 "Wrong backup password");

			if (keyStore.isOld()) {
				Log::info("Import from old keystore");
				if (keyStore.HasPhrasePassword()) {
					ParamChecker::checkCondition(true, Error::KeyStoreNeedPhrasePassword, "Old keystore need Phrase password");
				}
				_initFrom = ImportFromOldKeyStore;
			}

			initFromKeyStore(keyStore, payPassword);
		}

		void MasterWallet::importFromMnemonic(const std::string &mnemonic,
											  const std::string &phrasePassword,
											  const std::string &payPassword) {
			_localStore.Reset(mnemonic, phrasePassword, payPassword);
			initSubWalletsPubKeyMap(payPassword);
		}

		nlohmann::json MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &payPassword) {
			KeyStore keyStore(_rootPath);
			restoreKeyStore(keyStore, payPassword);

			nlohmann::json result;
			ParamChecker::checkCondition(!keyStore.Export(result, backupPassword), Error::KeyStore, "Export key error.");

			return result;
		}

		bool MasterWallet::exportMnemonic(const std::string &payPassword, std::string &mnemonic) {
			std::string encryptedMnemonic = _localStore.Account()->GetEncryptedMnemonic();
			ParamChecker::CheckDecrypt(!Utils::Decrypt(mnemonic, encryptedMnemonic, payPassword));
			return true;
		}

		void MasterWallet::initFromLocalStore(const MasterWalletStore &localStore) {
			tryInitCoinConfig();
			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, localStore.GetIdAgentInfo()));
			initSubWallets(localStore.GetSubWalletInfoList());
		}

		void MasterWallet::initSubWallets(const std::vector<CoinInfo> &coinInfoList) {
			for (int i = 0; i < coinInfoList.size(); ++i) {
				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].getChainId());
				ISubWallet *subWallet = SubWalletFactoryMethod(coinInfoList[i], coinConfig,
															   ChainParams(coinConfig), this);
				SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
				ParamChecker::checkCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
											 "Recover sub wallet error");
				startPeerManager(subWalletImpl);
				_createdWallets[subWallet->GetChainId()] = subWallet;
			}
			Save();
		}


		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {

			ParamChecker::checkParamNotEmpty(message, "Sign message");
			ParamChecker::checkPassword(payPassword, "Pay");

			Key key = _localStore.Account()->DeriveMultiSignKey(payPassword);
			return Utils::encodeHex(key.Sign(message));
		}

		bool MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {

			Key key;
			key.SetPubKey(Utils::decodeHex(publicKey));
			return key.Verify(message, Utils::decodeHex(signature));
		}

		bool MasterWallet::IsIdValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfo &info, const CoinConfig &config,
														const ChainParams &chainParams,
														MasterWallet *parent) {

			CoinInfo fixedInfo = info;

			if (_initFrom == CreateNormal) {
				fixedInfo.setEaliestPeerTime(chainParams.GetLastCheckpoint().GetTimestamp());
			} else if (_initFrom == CreateMultiSign || _initFrom == ImportFromMnemonic ||
					   _initFrom == ImportFromOldKeyStore) {
				fixedInfo.setEaliestPeerTime(chainParams.GetFirstCheckpoint().GetTimestamp());
			} else if (_initFrom == ImportFromKeyStore || _initFrom == ImportFromLocalStore) {
				fixedInfo.setEaliestPeerTime(info.getEarliestPeerTime());
			} else {
				fixedInfo.setEaliestPeerTime(chainParams.GetFirstCheckpoint().GetTimestamp());
			}

			std::vector<UInt256> visibleAssets;
			visibleAssets.push_back(Asset::GetELAAssetID());
			fixedInfo.SetVisibleAssets(visibleAssets);

			Log::info("Master wallet init from {}, ealiest peer time = {}", _initFrom,
								   fixedInfo.getEarliestPeerTime());

			fixedInfo.setGenesisAddress(config.GenesisAddress);

			switch (fixedInfo.getWalletType()) {
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
			return _idAgentImpl->DeriveIdAndKeyForPurpose(purpose, index);
		}

		nlohmann::json
		MasterWallet::GenerateProgram(const std::string &id, const std::string &message, const std::string &password) {
			PayloadRegisterIdentification payload;
			nlohmann::json payLoadJson = nlohmann::json::parse(message);
			payload.fromJson(payLoadJson, 0);

			ByteStream ostream;
			payload.Serialize(ostream, 0);

			nlohmann::json j;
			CMBlock signedData = _idAgentImpl->Sign(id, ostream.getBuffer(), password);

			ostream.setPosition(0);
			ostream.writeVarBytes(signedData);
			j["Parameter"] = Utils::encodeHex(ostream.getBuffer());
			j["Code"] = _idAgentImpl->GenerateRedeemScript(id, password);
			return j;
		}

		std::string MasterWallet::Sign(const std::string &id, const std::string &message, const std::string &password) {
			ParamChecker::checkParamNotEmpty(id, "Master wallet id");
			ParamChecker::checkParamNotEmpty(message, "Master wallet sign message");
			ParamChecker::checkPassword(password, "Master wallet sign");

			return _idAgentImpl->Sign(id, message, password);
		}

		std::vector<std::string> MasterWallet::GetAllIds() const {
			if (_idAgentImpl == nullptr)
				return std::vector<std::string>();

			return _idAgentImpl->GetAllIds();
		}

		std::string MasterWallet::GetPublicKey(const std::string &id) const {
			return _idAgentImpl->GetPublicKey(id);
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
			CMBlock programHash = Base58::CheckDecode(address);
			if (programHash.GetSize() == 21) {
				if (programHash[0] == PrefixStandard ||
					programHash[0] == PrefixCrossChain ||
					programHash[0] == PrefixMultiSign ||
					programHash[0] == PrefixIDChain ||
					programHash[0] == PrefixDeposit ||
					programHash[0] == PrefixDestroy)
					return true;
			}

			return false;
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() const {
			tryInitCoinConfig();
			return _coinConfigReader.GetAllChainId();
		}

		void MasterWallet::tryInitCoinConfig() const {
			if (!_coinConfigReader.IsInitialized()) {
				boost::filesystem::path configPath = _rootPath;
				configPath /= COIN_COINFIG_FILE;
				ParamChecker::checkPathExists(configPath);
				_coinConfigReader.Load(configPath);
			}
		}

		void MasterWallet::initSubWalletsPubKeyMap(const std::string &payPassword) {
			tryInitCoinConfig();

			MasterPubKeyMap subWalletsPubKeyMap;
			VotePubKeyMap votePubKeyMap;
			typedef std::map<std::string, uint32_t> IdIndexMap;
			IdIndexMap idIndexMap = _coinConfigReader.GetChainIdsAndIndices();
			std::for_each(idIndexMap.begin(), idIndexMap.end(),
						  [this, &subWalletsPubKeyMap, &votePubKeyMap, &payPassword](const IdIndexMap::value_type &item) {
							  subWalletsPubKeyMap[item.first] = SubAccountGenerator::GenerateMasterPubKey(
									  _localStore.Account(),
									  item.second, payPassword);
							  if (item.first == "ELA") {
								  votePubKeyMap[item.first] = SubAccountGenerator::GenerateVotePubKey(
									  _localStore.Account(), item.second, payPassword);
							  } else {
								  votePubKeyMap[item.first] = CMBlock();
							  }
						  });
			_localStore.SetMasterPubKeyMap(subWalletsPubKeyMap);
			_localStore.SetVotePublicKeyMap(votePubKeyMap);
		}

		void MasterWallet::restoreLocalStore() {
			if (_idAgentImpl != nullptr)
				_localStore.SetIdAgentInfo(_idAgentImpl->GetIdAgentInfo());

			std::vector<CoinInfo> coinInfos;
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet == nullptr) continue;
				coinInfos.push_back(subWallet->getCoinInfo());
			}
			_localStore.SetSubWalletInfoList(coinInfos);
		}

		void MasterWallet::initFromKeyStore(const KeyStore &keyStore, const std::string &payPassword) {
			tryInitCoinConfig();

			IAccount *account = keyStore.createAccountFromJson(payPassword);
			if (!account->IsReadOnly()) {
				ParamChecker::checkPassword(payPassword, "Pay");
			}

			_localStore.Reset(account);
			_localStore.IsSingleAddress() = keyStore.json().getIsSingleAddress();
			initSubWalletsPubKeyMap(payPassword);
			initSubWallets(keyStore.json().getCoinInfoList());
		}

		void MasterWallet::restoreKeyStore(KeyStore &keyStore, const std::string &payPassword) {
			keyStore.initJsonFromAccount(_localStore.Account(), payPassword);

			keyStore.json().clearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(),
						  [&keyStore](const WalletMap::value_type &item) {
							  SubWallet *subWallet = dynamic_cast<SubWallet *>(item.second);
							  keyStore.json().addCoinInfo(subWallet->_info);
						  });
			keyStore.json().setIsSingleAddress(_localStore.IsSingleAddress());
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			_localStore.Account()->ChangePassword(oldPassword, newPassword);
		}

		IIdAgent *MasterWallet::GetIIdAgent() {
			return this;
		}

		void MasterWallet::initFromMultiSigners(const std::string &privKey, const std::string &payPassword,
												const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			if (privKey.empty())
				_localStore.Reset(coSigners, requiredSignCount);
			else
				_localStore.Reset(privKey, coSigners, payPassword, requiredSignCount);
			_localStore.IsSingleAddress() = true;
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = _localStore.Account()->GetBasicInfo();
			j["Account"]["SingleAddress"] = _localStore.IsSingleAddress();
			return j;
		}

		void MasterWallet::initFromMultiSigners(const std::string &mnemonic, const std::string &phrasePassword,
												const std::string &payPassword,
												const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			_localStore.Reset(mnemonic, phrasePassword, coSigners, payPassword, requiredSignCount);
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _localStore.Account()->IsEqual(*wallet._localStore.Account());
		}

	}
}
