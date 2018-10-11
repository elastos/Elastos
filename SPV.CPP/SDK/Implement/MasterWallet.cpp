// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <boost/filesystem.hpp>
#include <Core/BRKey.h>
#include <Core/BRBIP32Sequence.h>
#include <Core/BRChainParams.h>

#include "BRBase58.h"
#include "BRBIP39Mnemonic.h"
#include "BRCrypto.h"

#include "Utils.h"
#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "IdChainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SidechainSubWallet.h"
#include "Log.h"
#include "Config.h"
#include "ParamChecker.h"
#include "BigIntFormat.h"
#include "WalletTool.h"
#include "Payload/PayloadRegisterIdentification.h"

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
								   const std::string &language,
								   bool p2pEnable,
								   const std::string &rootPath,
								   MasterWalletInitFrom from) :
			_id(id),
			_rootPath(rootPath),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_localStore(rootPath) {

			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkArgumentNotEmpty(language, "Language");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromMnemonic(mnemonic, language, phrasePassword, payPassword);
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPassword,
								   const std::string &phrasePassword,
								   const std::string &rootPath,
								   bool p2pEnable,
								   MasterWalletInitFrom from) :
			_id(id),
			_rootPath(rootPath),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_localStore(rootPath) {

			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");
			ParamChecker::checkArgumentNotEmpty(rootPath, "Root path");

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromKeyStore(keystoreContent, backupPassword, payPassword, phrasePassword);
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
			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkArgumentNotEmpty(rootPath, "Root path");
			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount, "Signers");

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

			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkArgumentNotEmpty(rootPath, "Root path");
			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");

			initFromMultiSigners(privKey, payPassword, coSigners, requiredSignCount);
		}

		MasterWallet::MasterWallet(const std::string &id, const std::string &mnemonic,
								   const std::string &phrasePassword, const std::string &payPassword,
								   const std::string &language, const nlohmann::json &coSigners,
								   uint32_t requiredSignCount, bool p2pEnable, const std::string &rootPath,
								   MasterWalletInitFrom from) :
			_id(id),
			_rootPath(rootPath),
			_p2pEnable(p2pEnable),
			_initFrom(from),
			_localStore(rootPath) {
			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");
			ParamChecker::checkArgumentNotEmpty(language, "Language");
			ParamChecker::checkArgumentNotEmpty(rootPath, "Root path");
			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");

			initFromMultiSigners(mnemonic, phrasePassword, language, payPassword, coSigners, requiredSignCount);
		}

		MasterWallet::~MasterWallet() {

		}

		std::string MasterWallet::GenerateMnemonic(const std::string &language, const std::string &rootPath) {
			CMemBlock<uint8_t> seed128 = WalletTool::GenerateSeed128();
			Mnemonic mnemonic(language, rootPath);
			CMemBlock<char> phrase = WalletTool::GeneratePhraseFromSeed(seed128, mnemonic.words());
			return (const char *) phrase;
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
		MasterWallet::CreateSubWallet(const std::string &chainID, const std::string &payPassword, bool singleAddress,
									  uint64_t feePerKb) {

			ParamChecker::checkArgumentNotEmpty(chainID, "Chain ID");
			ParamChecker::checkCondition(chainID.size() > 128, Error::InvalidArgument, "Chain ID sould less than 128");

			if (!_localStore.Account()->IsReadOnly())
				ParamChecker::checkPassword(payPassword, "Pay");

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

			info.setSingleAddress(singleAddress);
			info.setUsedMaxAddressIndex(0);
			info.setChainId(chainID);
			info.setFeePerKb(feePerKb);

			SubWallet *subWallet = SubWalletFactoryMethod(info, ChainParams(coinConfig), payPassword,
														  PluginTypes(coinConfig), this);
			_createdWallets[chainID] = subWallet;
			startPeerManager(subWallet);
			Save();
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, const std::string &payPassword, bool singleAddress,
									   uint32_t limitGap, uint64_t feePerKb) {

			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			ParamChecker::checkCondition(limitGap > SEQUENCE_GAP_LIMIT_EXTERNAL, Error::LimitGap,
										 "Limit gap should less than or equal " +
										 std::to_string(SEQUENCE_GAP_LIMIT_EXTERNAL));

			ISubWallet *subWallet = CreateSubWallet(chainID, payPassword, singleAddress, feePerKb);
			SubWallet *walletInner = dynamic_cast<SubWallet *>(subWallet);
			assert(walletInner != nullptr);
			walletInner->recover(limitGap);

			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		void MasterWallet::restoreSubWallets(const std::vector<CoinInfo> &coinInfoList) {

			for (int i = 0; i < coinInfoList.size(); ++i) {
				if (_createdWallets.find(coinInfoList[i].getChainId()) != _createdWallets.end()) continue;

				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].getChainId());
				_createdWallets[coinInfoList[i].getChainId()] =
					SubWalletFactoryMethod(coinInfoList[i], ChainParams(coinConfig), "", PluginTypes(coinConfig),
										   this);
			}
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {

			ParamChecker::checkCondition(wallet == nullptr, Error::Wallet,
										 "Destroy wallet can't be null");

			ParamChecker::checkCondition(_createdWallets.empty(), Error::Wallet,
										 "There is no sub wallet in this wallet");

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

		std::string MasterWallet::GetPublicKey() {
			return _localStore.Account()->GetPublicKey();
		}

		void MasterWallet::importFromKeyStore(const nlohmann::json &keystoreContent, const std::string &backupPassword,
											  const std::string &payPassword, const std::string &phrasePassword) {
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			KeyStore keyStore(_rootPath);
			ParamChecker::checkCondition(!keyStore.open(keystoreContent, backupPassword), Error::WrongPasswd,
										 "Wrong backup password");

			if (keyStore.isOld()) {
				Log::getLogger()->info("Import from old keystore");
				_initFrom = ImportFromOldKeyStore;
			}

			initFromKeyStore(keyStore, payPassword, phrasePassword);
		}

		void MasterWallet::importFromMnemonic(const std::string &mnemonic,
											  const std::string &languange,
											  const std::string &phrasePassword,
											  const std::string &payPassword) {
			ParamChecker::checkArgumentNotEmpty(mnemonic, "Mnemonic");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			_localStore.Reset(mnemonic, languange, phrasePassword, payPassword);
		}

		nlohmann::json MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &payPassword) {
			KeyStore keyStore(_rootPath);
			restoreKeyStore(keyStore, payPassword);

			nlohmann::json result;
			ParamChecker::checkCondition(!keyStore.save(result, backupPassword), Error::KeyStore, "Export key error.");

			return result;
		}

		bool MasterWallet::exportMnemonic(const std::string &payPassword, std::string &mnemonic) {

			CMBlock cb_Mnemonic = Utils::decrypt(_localStore.Account()->GetEncryptedMnemonic(), payPassword);
			if (false == cb_Mnemonic) {
				return false;
			}
			CMemBlock<char> cb_char_Mnemonic(cb_Mnemonic.GetSize() + 1);
			cb_char_Mnemonic.Zero();
			memcpy(cb_char_Mnemonic, cb_Mnemonic, cb_Mnemonic.GetSize());
			mnemonic = (const char *) cb_char_Mnemonic;
			return true;
		}

		void MasterWallet::initFromLocalStore(const MasterWalletStore &localStore) {
			tryInitCoinConfig();
			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, localStore.GetIdAgentInfo()));
			initSubWallets(localStore.GetSubWalletInfoList(), "");
		}

		void MasterWallet::initSubWallets(const std::vector<CoinInfo> &coinInfoList, const std::string &payPassword) {
			for (int i = 0; i < coinInfoList.size(); ++i) {
				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].getChainId());
				ISubWallet *subWallet = SubWalletFactoryMethod(coinInfoList[i], ChainParams(coinConfig), payPassword,
															   PluginTypes(coinConfig), this);
				SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
				ParamChecker::checkCondition(subWalletImpl == nullptr, Error::CreateSubWalletError,
											 "Recover sub wallet error");
				startPeerManager(subWalletImpl);
				_createdWallets[subWallet->GetChainId()] = subWallet;
			}
			Save();
		}


		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {

			ParamChecker::checkArgumentNotEmpty(message, "Sign message");
			ParamChecker::checkPassword(payPassword, "Pay");

			Key key = _localStore.Account()->DeriveKey(payPassword);
			return key.compactSign(message);
		}

		nlohmann::json
		MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {

			bool r = Key::verifyByPublicKey(publicKey, message, signature);
			nlohmann::json jsonData;
			jsonData["Result"] = r;
			return jsonData;
		}

		bool MasterWallet::IsIdValid(const std::string &id) {
			return Address::isValidIdAddress(id);
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfo &info, const ChainParams &chainParams,
														const std::string &payPassword, const PluginTypes &pluginTypes,
														MasterWallet *parent) {

			CoinInfo fixedInfo = info;

			BRChainParams *rawParams = chainParams.getRaw();
			time_t timeStamp = rawParams->checkpoints[0].timestamp;
			size_t checkPointsCount = rawParams->checkpointsCount;

			if (_initFrom == CreateNormal) {
				timeStamp = rawParams->checkpoints[checkPointsCount - 1].timestamp;
				fixedInfo.setEaliestPeerTime(timeStamp);
			} else if (_initFrom == CreateMultiSign || _initFrom == ImportFromMnemonic ||
					   _initFrom == ImportFromOldKeyStore) {
				timeStamp = rawParams->checkpoints[0].timestamp;
				fixedInfo.setEaliestPeerTime(timeStamp);
			} else if (_initFrom == ImportFromKeyStore || _initFrom == ImportFromLocalStore) {
				fixedInfo.setEaliestPeerTime(info.getEarliestPeerTime());
			} else {
				timeStamp = rawParams->checkpoints[0].timestamp;
				fixedInfo.setEaliestPeerTime(timeStamp);
			}

			Log::getLogger()->info("Master wallet init from {}, ealiest peer time = {}", _initFrom,
								   fixedInfo.getEarliestPeerTime());

			switch (fixedInfo.getWalletType()) {
				case Mainchain:
					return new MainchainSubWallet(fixedInfo, chainParams, payPassword, pluginTypes, parent);
				case Sidechain:
					return new SidechainSubWallet(fixedInfo, chainParams, payPassword, pluginTypes, parent);
				case Idchain:
					return new IdChainSubWallet(fixedInfo, chainParams, payPassword, pluginTypes, parent);
				case Normal:
				default:
					return new SubWallet(fixedInfo, chainParams, payPassword, pluginTypes, parent);
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
			payload.fromJson(payLoadJson);

			ByteStream ostream;
			payload.Serialize(ostream);
			CMBlock payloadData = ostream.getBuffer();

			nlohmann::json j;
			j["Parameter"] = _idAgentImpl->Sign(id, Utils::convertToString(payloadData), password);
			j["Code"] = _idAgentImpl->GenerateRedeemScript(id, password);
			return j;
		}

		std::string MasterWallet::Sign(const std::string &id, const std::string &message, const std::string &password) {
			ParamChecker::checkArgumentNotEmpty(id, "Master wallet id");
			ParamChecker::checkArgumentNotEmpty(message, "Master wallet sign message");
			ParamChecker::checkPassword(password, "Master wallet sign");

			return _idAgentImpl->Sign(id, message, password);
		}

		std::vector<std::string> MasterWallet::GetAllIds() const {
			if (_idAgentImpl == nullptr)
				return std::vector<std::string>();

			return _idAgentImpl->GetAllIds();
		}

		std::string MasterWallet::GetPublicKey(const std::string &id) {
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

		bool MasterWallet::IsAddressValid(const std::string &address) {
			return Address::isValidAddress(address);
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() {
			tryInitCoinConfig();
			return _coinConfigReader.GetAllChainId();
		}

		void MasterWallet::tryInitCoinConfig() {
			if (!_coinConfigReader.IsInitialized()) {
				boost::filesystem::path configPath = _rootPath;
				configPath /= COIN_COINFIG_FILE;
				ParamChecker::checkPathExists(configPath);
				_coinConfigReader.Load(configPath);
			}
		}

		void MasterWallet::restoreLocalStore() {
			if (_idAgentImpl != nullptr)
				_localStore.SetIdAgentInfo(_idAgentImpl->GetIdAgentInfo());

			std::vector<CoinInfo> coinInfos;
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet == nullptr) continue;
				Log::getLogger()->info("Going to save configuration of subwallet '{}'", subWallet->GetChainId());
				coinInfos.push_back(subWallet->_info);
			}
			_localStore.SetSubWalletInfoList(coinInfos);
		}

		void MasterWallet::initFromKeyStore(const KeyStore &keyStore, const std::string &payPassword,
											const std::string &phrasePassword) {
			tryInitCoinConfig();

			IAccount *account = keyStore.createAccountFromJson(phrasePassword, payPassword);
			_localStore.Reset(account);
			initSubWallets(keyStore.json().getCoinInfoList(), payPassword);
		}

		void MasterWallet::restoreKeyStore(KeyStore &keyStore, const std::string &payPassword) {
			keyStore.initJsonFromAccount(_localStore.Account(), payPassword);

			keyStore.json().clearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(),
						  [&keyStore](const WalletMap::value_type &item) {
							  SubWallet *subWallet = dynamic_cast<SubWallet *>(item.second);
							  keyStore.json().addCoinInfo(subWallet->_info);
						  });
		}

		void MasterWallet::initFrom(MasterWalletInitFrom from) {
			_initFrom = from;
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			_localStore.Account()->ChangePassword(oldPassword, newPassword);
		}

		void MasterWallet::initFromMultiSigners(const std::string &privKey, const std::string &payPassword,
												const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			if (privKey.empty())
				_localStore.Reset(coSigners, requiredSignCount);
			else
				_localStore.Reset(privKey, coSigners, payPassword, requiredSignCount);
		}

		nlohmann::json MasterWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = _localStore.Account()->GetBasicInfo();
			return j;
		}

		void MasterWallet::initFromMultiSigners(const std::string &mnemonic, const std::string &phrasePassword,
												const std::string &language, const std::string &payPassword,
												const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			_localStore.Reset(mnemonic, language, phrasePassword, coSigners, payPassword, requiredSignCount);
		}

		bool MasterWallet::IsEqual(const MasterWallet &wallet) const {
			return _localStore.Account()->IsEqual(*wallet._localStore.Account());
		}

	}
}
