// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <boost/filesystem.hpp>
#include <Core/BRKey.h>
#include <Core/BRBIP32Sequence.h>

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
#include "BTCBase58.h"
#include "ErrorCode.h"
#include "Payload/PayloadRegisterIdentification.h"

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define COIN_COINFIG_FILE "CoinConfig.json"
#define BIP32_SEED_KEY "Bitcoin seed"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {

		MasterWallet::MasterWallet(const boost::filesystem::path &localStore,
								   const std::string &rootPath,
								   bool p2pEnable) :
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_isImportFromMnemonic(false),
				_isOldKeyStore(false) {

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
								   const std::string &rootPath) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_isImportFromMnemonic(false),
				_isOldKeyStore(false) {

			ParamChecker::checkNotEmpty(id);
			ParamChecker::checkNotEmpty(language);
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			resetMnemonic(language);

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromMnemonic(mnemonic, phrasePassword, payPassword);
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const nlohmann::json &keystoreContent,
								   const std::string &backupPassword,
								   const std::string &payPassword,
								   const std::string &phrasePassword,
								   const std::string &rootPath,
								   bool p2pEnable) :
				_id(id),
				_rootPath(rootPath),
				_p2pEnable(p2pEnable),
				_isImportFromMnemonic(false),
				_isOldKeyStore(false) {

			ParamChecker::checkNotEmpty(id);
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");
			ParamChecker::checkNotEmpty(rootPath);

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
			importFromKeyStore(keystoreContent, backupPassword, payPassword, phrasePassword);
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

			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet != nullptr) {
					subWallet->fireDestroyWallet();
				}
			}
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

			ParamChecker::checkNotEmpty(chainID);

			if (chainID.size() > 128)
				throw std::invalid_argument("Chain id should less than 128.");

			ParamChecker::checkPassword(payPassword, "Pay");

			//todo limit coinTypeIndex and feePerKb if needed in future

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				deriveKey(payPassword);
				return _createdWallets[chainID];
			}

			CoinInfo info;
			if (_isImportFromMnemonic || _isOldKeyStore) {
				info.setEaliestPeerTime(1513936800);
			} else {
				info.setEaliestPeerTime(0);
			}

			Log::getLogger()->info("import from mnemonic is {}, ealiest peer time = {}", _isImportFromMnemonic, info.getEarliestPeerTime());

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

			if (limitGap > SEQUENCE_GAP_LIMIT_EXTERNAL) {
				throw std::invalid_argument("Limit gap should less than or equal 10.");
			}

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

			if (wallet == nullptr)
				throw std::invalid_argument("Sub wallet can't be null.");

			if (_createdWallets.empty())
				throw std::logic_error("There is no sub wallet in this wallet.");

			if (std::find_if(_createdWallets.begin(), _createdWallets.end(),
							 [wallet](const WalletMap::value_type &item) {
								 return item.second == wallet;
							 }) == _createdWallets.end())
				throw std::logic_error("Specified sub wallet is not belong to current master wallet.");

			_createdWallets.erase(std::find_if(_createdWallets.begin(), _createdWallets.end(),
											   [wallet](const WalletMap::value_type &item) {
												   return item.second == wallet;
											   }));
			SubWallet *walletInner = dynamic_cast<SubWallet *>(wallet);
			assert(walletInner != nullptr);
			stopPeerManager(walletInner);
			delete walletInner;
		}

		std::string MasterWallet::GetPublicKey() {
			return _localStore.GetPublicKey();
		}

		bool MasterWallet::importFromKeyStore(const nlohmann::json &keystoreContent, const std::string &backupPassword,
											  const std::string &payPassword, const std::string &phrasePassword) {
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			KeyStore keyStore;
			if (!keyStore.open(keystoreContent, backupPassword))
				throw std::logic_error("Import key error.");

			if (keyStore.isOld()) {
				Log::getLogger()->info("import from old keystore");
				_isOldKeyStore = true;
			}

			initFromKeyStore(keyStore, payPassword, phrasePassword);

			Save();
			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {
			ParamChecker::checkNotEmpty(mnemonic);
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");

			bool result = initFromPhrase(mnemonic, phrasePassword, payPassword);
//			CreateSubWallet("ELA", payPassword, false); //we create ela sub wallet by default

			Save();
			return result;
		}

		nlohmann::json MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &payPassword) {
			KeyStore keyStore;
			restoreKeyStore(keyStore, payPassword);

			nlohmann::json result;
			if (!keyStore.save(result, backupPassword)) {
				throw std::logic_error("Export key error.");
			}

			return result;
		}

		bool MasterWallet::exportMnemonic(const std::string &payPassword, std::string &mnemonic) {

			CMBlock cb_Mnemonic = Utils::decrypt(_localStore.GetEncryptedMnemonic(), payPassword);
			if (false == cb_Mnemonic) {
				return false;
			}
			CMemBlock<char> cb_char_Mnemonic(cb_Mnemonic.GetSize() + 1);
			cb_char_Mnemonic.Zero();
			memcpy(cb_char_Mnemonic, cb_Mnemonic, cb_Mnemonic.GetSize());
			mnemonic = (const char *) cb_char_Mnemonic;
			return true;
		}

		bool MasterWallet::initFromPhrase(const std::string &phrase, const std::string &phrasePassword,
										  const std::string &payPassword) {
			CMemBlock<char> phraseData;
			phraseData.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			std::string originLanguage = _mnemonic->getLanguage();
			if (!_localStore.GetLanguage().empty() && _localStore.GetLanguage() != originLanguage) {
				resetMnemonic(_localStore.GetLanguage());
			}

			if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
				resetMnemonic("chinese");
				if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
					throw std::logic_error("Import key error.");
				}
			}

			CMBlock cbPhrase0 = Utils::convertToMemBlock(phrase);
			_localStore.SetEncryptedMnemonic(Utils::encrypt(cbPhrase0, payPassword));
			CMBlock phrasePass = Utils::convertToMemBlock(phrasePassword);
			_localStore.SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, payPassword));

			//init master public key and private key
			UInt512 seed = deriveSeed(payPassword);

			BRKey masterKey;
			BRBIP32APIAuthKey(&masterKey, &seed, sizeof(seed));

			CMBlock cbTmp(sizeof(UInt256));
			memcpy(cbTmp, &masterKey.secret, sizeof(UInt256));
			_localStore.SetEncryptedKey(Utils::encrypt(cbTmp, payPassword));

			Key key(masterKey);
			key.setPublicKey();
			_localStore.SetPublicKey(Utils::encodeHex(key.getPubkey()));

			//init id chain derived master public key
			BRKey idMasterKey;
			UInt256 idChainCode;
			BRBIP32PrivKeyPath(&idMasterKey, &idChainCode, &seed, sizeof(seed), 1, 0 | BIP32_HARD);
			Key wrapperKey(idMasterKey.secret, idMasterKey.compressed);
			_localStore.SetIDMasterPubKey(MasterPubKey(wrapperKey.getPubkey(), idChainCode));

			var_clean(&seed);
			var_clean(&masterKey.secret);

			return true;
		}

		void MasterWallet::initFromLocalStore(const MasterWalletStore &localStore) {
			tryInitCoinConfig();
			resetMnemonic(localStore.GetLanguage());
			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, localStore.GetIdAgentInfo()));
			initSubWallets(localStore.GetSubWalletInfoList(), "");
		}

		void MasterWallet::initSubWallets(const std::vector<CoinInfo> &coinInfoList, const std::string &payPassword) {
			for (int i = 0; i < coinInfoList.size(); ++i) {
				CoinConfig coinConfig = _coinConfigReader.FindConfig(coinInfoList[i].getChainId());
				ISubWallet *subWallet = SubWalletFactoryMethod(coinInfoList[i], ChainParams(coinConfig), payPassword,
															   PluginTypes(coinConfig), this);
				SubWallet *subWalletImpl = dynamic_cast<SubWallet *>(subWallet);
				if (subWalletImpl == nullptr)
					throw std::logic_error("Recover sub wallet error: unknown sub wallet implement type.");
				startPeerManager(subWalletImpl);
				_createdWallets[subWallet->GetChainId()] = subWallet;
			}
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(_localStore.GetEncrpytedKey(), payPassword);
			ParamChecker::checkDataNotEmpty(keyData);

			Key key;
			UInt256 secret;
			memcpy(secret.u8, keyData, keyData.GetSize());
			key.setSecret(secret, true);

			return key;
		}

		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {

			ParamChecker::checkNotEmpty(message);
			ParamChecker::checkPassword(payPassword);

			Key key = deriveKey(payPassword);
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

		UInt512 MasterWallet::deriveSeed(const std::string &payPassword) {
			UInt512 result;
			std::string mnemonic = Utils::convertToString(
					Utils::decrypt(_localStore.GetEncryptedMnemonic(), payPassword));
			if (mnemonic.empty())
				ErrorCode::StandardLogicError(ErrorCode::PasswordError, "Invalid password.");

			std::string phrasePassword = _localStore.GetEncrptedPhrasePassword().GetSize() == 0
										 ? ""
										 : Utils::convertToString(
							Utils::decrypt(_localStore.GetEncrptedPhrasePassword(), payPassword));

			BRBIP39DeriveKey(&result, mnemonic.c_str(), phrasePassword.c_str());
			return result;
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfo &info, const ChainParams &chainParams,
														const std::string &payPassword, const PluginTypes &pluginTypes,
														MasterWallet *parent) {
			switch (info.getWalletType()) {
				case Mainchain:
					return new MainchainSubWallet(info, chainParams, payPassword, pluginTypes, parent);
				case Sidechain:
					return new SidechainSubWallet(info, chainParams, payPassword, pluginTypes, parent);
				case Idchain:
					return new IdChainSubWallet(info, chainParams, payPassword, pluginTypes, parent);
				case Normal:
				default:
					return new SubWallet(info, chainParams, payPassword, pluginTypes, parent);
			}
		}

		void MasterWallet::resetMnemonic(const std::string &language) {
			_localStore.SetLanguage(language);
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(language, _rootPath));
		}

		std::string
		MasterWallet::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index) {
			std::string r = _idAgentImpl->DeriveIdAndKeyForPurpose(purpose, index);
			Save();
			return r;
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
			ParamChecker::checkNotEmpty(id);
			ParamChecker::checkNotEmpty(message);
			ParamChecker::checkPassword(password);

			return _idAgentImpl->Sign(id, message, password);
		}

		std::vector<std::string> MasterWallet::GetAllIds() const {
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

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			ParamChecker::checkPassword(oldPassword, "Old");
			ParamChecker::checkPassword(newPassword, "New");

			CMBlock key = Utils::decrypt(_localStore.GetEncrpytedKey(), oldPassword);
			ParamChecker::checkDataNotEmpty(key, false);
			CMBlock phrasePass = Utils::decrypt(_localStore.GetEncrptedPhrasePassword(), oldPassword);
			CMBlock mnemonic = Utils::decrypt(_localStore.GetEncryptedMnemonic(), oldPassword);
			ParamChecker::checkDataNotEmpty(mnemonic, false);

			_localStore.SetEncryptedKey(Utils::encrypt(key, newPassword));
			_localStore.SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, newPassword));
			_localStore.SetEncryptedMnemonic(Utils::encrypt(mnemonic, newPassword));
		}

		void MasterWallet::tryInitCoinConfig() {
			if (!_coinConfigReader.IsInitialized()) {
				boost::filesystem::path configPath = _rootPath;
				configPath /= COIN_COINFIG_FILE;
				if (!boost::filesystem::exists(configPath))
					throw std::logic_error("Coin config file not found.");
				_coinConfigReader.Load(configPath);
			}
		}

		void MasterWallet::restoreLocalStore() {
			_localStore.SetIdAgentInfo(_idAgentImpl->GetIdAgentInfo());

			std::vector<CoinInfo> coinInfos;
			for (WalletMap::iterator it = _createdWallets.begin(); it != _createdWallets.end(); ++it) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(it->second);
				if (subWallet == nullptr) continue;
				coinInfos.push_back(subWallet->_info);
			}
			_localStore.SetSubWalletInfoList(coinInfos);
		}

		void MasterWallet::initFromKeyStore(const KeyStore &keyStore, const std::string &payPassword,
											const std::string &phrasePassword) {
			tryInitCoinConfig();

			CMBlock phrasePassRaw0 = Utils::convertToMemBlock(
					keyStore.json().getEncryptedPhrasePassword());
			std::string phrasePass = "";
			if (true == phrasePassRaw0) {
				CMBlock phrasePassRaw1(phrasePassRaw0.GetSize() + 1);
				phrasePassRaw1.Zero();
				memcpy(phrasePassRaw1, phrasePassRaw0, phrasePassRaw0.GetSize());
				CMBlock phrasePassRaw = Utils::decrypt(phrasePassRaw1, payPassword);
				phrasePass = Utils::convertToString(phrasePassRaw);
			}
			phrasePass = phrasePassword != "" ? phrasePassword : phrasePass;

			std::string mnemonic = keyStore.json().getMnemonic();
			CMBlock cbMnemonic;
			cbMnemonic.SetMemFixed((const uint8_t *) mnemonic.c_str(), mnemonic.size());
			resetMnemonic(keyStore.json().getMnemonicLanguage());
			_localStore.SetEncryptedMnemonic(Utils::encrypt(cbMnemonic, payPassword));

			if (!initFromPhrase(mnemonic, phrasePass, payPassword))
				throw std::logic_error("Initialize from phrase error.");

			initSubWallets(keyStore.json().getCoinInfoList(), payPassword);
		}

		void MasterWallet::restoreKeyStore(KeyStore &keyStore, const std::string &payPassword) {
			keyStore.json().setEncryptedPhrasePassword(Utils::encodeHex(_localStore.GetEncrptedPhrasePassword()));

			CMBlock cbMnemonic = Utils::decrypt(_localStore.GetEncryptedMnemonic(), payPassword);
			if (false == cbMnemonic) {
				throw std::logic_error("Wrong password.");
			}

			CMemBlock<char> charMnemonic(cbMnemonic.GetSize() + 1);
			charMnemonic.Zero();
			memcpy(charMnemonic, cbMnemonic, cbMnemonic.GetSize());
			if (keyStore.json().getMnemonic().empty())
				keyStore.json().setMnemonic((const char *) charMnemonic);

			keyStore.json().clearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(),
						  [&keyStore](const WalletMap::value_type &item) {
							  SubWallet *subWallet = dynamic_cast<SubWallet *>(item.second);
							  keyStore.json().addCoinInfo(subWallet->_info);
						  });
		}

		void MasterWallet::setImportFromMnemonic() {
			_isImportFromMnemonic = true;
		}

	}
}
