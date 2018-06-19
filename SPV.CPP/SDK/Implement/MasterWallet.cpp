// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <boost/filesystem.hpp>
#include <Core/BRKey.h>

#include "BRBase58.h"
#include "BRBIP39Mnemonic.h"

#include "Utils.h"
#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "IdChainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SidechainSubWallet.h"
#include "Enviroment.h"
#include "Log.h"
#include "Config.h"
#include "ParamChecker.h"
#include "BigIntFormat.h"
#include "WalletTool.h"
#include "BTCBase58.h"

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define COIN_COINFIG_FILE "CoinConfig.json"

namespace fs = boost::filesystem;

namespace Elastos {
	namespace SDK {

		MasterWallet::MasterWallet(const boost::filesystem::path &localStore) :
				_initialized(false) {

			_localStore.Load(localStore);
			_id = localStore.parent_path().filename().string();

			initFromLocalStore(_localStore);
		}

		MasterWallet::MasterWallet(const std::string &id,
								   const std::string &language) :
				_id(id),
				_initialized(false) {

			ParamChecker::checkNotEmpty(id);
			ParamChecker::checkNotEmpty(language);

			resetMnemonic(language);

			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, _localStore.GetIdAgentInfo()));
		}

		MasterWallet::~MasterWallet() {
			Save();
		}

		std::string MasterWallet::GenerateMnemonic() const {
			CMemBlock<uint8_t> seed128 = WalletTool::GenerateSeed128();
			CMemBlock<char> phrase = WalletTool::GeneratePhraseFromSeed(seed128, _mnemonic->words());
			return (const char *) phrase;
		}

		void MasterWallet::Save() {

			if (!Initialized())
				return;

			restoreLocalStore();

			boost::filesystem::path path = Enviroment::GetRootPath();
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
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			std::vector<ISubWallet *> result;
			for (WalletMap::const_iterator it = _createdWallets.cbegin(); it != _createdWallets.cend(); ++it) {
				result.push_back(it->second);
			}

			return result;
		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, const std::string &payPassword, bool singleAddress,
									  uint64_t feePerKb) {

			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			ParamChecker::checkNotEmpty(chainID);

			if (chainID.size() > 128)
				throw std::invalid_argument("Chain id should less than 128.");

			ParamChecker::checkPassword(payPassword);

			//todo limit coinTypeIndex and feePerKb if needed in future

			if (_createdWallets.find(chainID) != _createdWallets.end()) {
				deriveKey(payPassword);
				return _createdWallets[chainID];
			}

			CoinInfo info;
			info.setEaliestPeerTime(0);

			tryInitCoinConfig();
			CoinConfig coinConfig = _coinConfigReader.FindConfig(chainID);
			info.setWalletType(coinConfig.Type);
			info.setIndex(coinConfig.Index);

			info.setSingleAddress(singleAddress);
			info.setUsedMaxAddressIndex(0);
			info.setChainId(chainID);
			info.setFeePerKb(feePerKb);
			SubWallet *subWallet = SubWalletFactoryMethod(info, ChainParams::mainNet(), payPassword, this);
			_createdWallets[chainID] = subWallet;
			startPeerManager(subWallet);
			Save();
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, const std::string &payPassword, bool singleAddress,
									   uint32_t limitGap, uint64_t feePerKb) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

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

				_createdWallets[coinInfoList[i].getChainId()] =
						SubWalletFactoryMethod(coinInfoList[i], ChainParams::mainNet(), "", this);
			}
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {

			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

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
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			return _localStore.GetPublicKey();
		}

		bool MasterWallet::importFromKeyStore(const std::string &keystorePath, const std::string &backupPassword,
											  const std::string &payPassword, const std::string &phrasePassword) {
			ParamChecker::checkNotEmpty(keystorePath);
			ParamChecker::checkPassword(backupPassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);

			KeyStore keyStore;
			if (!keyStore.open(keystorePath, backupPassword))
				throw std::logic_error("Import key error.");

			initFromKeyStore(keyStore, payPassword, phrasePassword);
			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {
			ParamChecker::checkNotEmpty(mnemonic);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);

			return initFromPhrase(mnemonic, phrasePassword, payPassword);
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &payPassword,
										  const std::string &keystorePath) {
			KeyStore keyStore;
			restoreKeyStore(keyStore, payPassword);

			if (!keyStore.save(keystorePath, backupPassword)) {
				Log::error("Export key error.");
				return false;
			}

			return true;
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

		bool MasterWallet::Initialized() const {
			return _initialized;
		}

		bool MasterWallet::initFromEntropy(const UInt128 &entropy, const std::string &phrasePassword,
										   const std::string &payPassword) {

			std::string phrase = MasterPubKey::generatePaperKey(entropy, _mnemonic->words());
			return initFromPhrase(phrase, phrasePassword, payPassword);
		}

		bool MasterWallet::initFromPhrase(const std::string &phrase, const std::string &phrasePassword,
										  const std::string &payPassword) {
			CMemBlock<char> phraseData;
			phraseData.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			std::string originLanguage = _mnemonic->getLanguage();
			if (!_localStore.GetLanguage().empty() && _localStore.GetLanguage() != originLanguage) {
				resetMnemonic(_localStore.GetLanguage());
			}
#ifdef MNEMONIC_SOURCE_H
			if (!WalletTool::PhraseIsValid(phraseData, language)) {
				if (!WalletTool::PhraseIsValid(phraseData, "chinese")) {
					Log::error("Phrase is unvalid.");
					return false;
				} else {
					_localStore.json().setMnemonicLanguage("chinese");
				}
			}
#else
			if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
				resetMnemonic("chinese");
				if (!WalletTool::PhraseIsValid(phraseData, _mnemonic->words())) {
					throw std::logic_error("Import key error.");
				}
			}
#endif
			CMBlock cbPhrase0 = Utils::convertToMemBlock<unsigned char>(phrase);
			_localStore.SetEncryptedMnemonic(Utils::encrypt(cbPhrase0, payPassword));
			CMBlock phrasePass = Utils::convertToMemBlock<unsigned char>(phrasePassword);
			_localStore.SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, payPassword));

			//init master public key and private key
			CMemBlock<char> cbPhrase;
			cbPhrase.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			std::string prikeyBase58 = WalletTool::getDeriveKey_base58(cbPhrase, phrasePassword);
			CMemBlock<unsigned char> prikey = BTCBase58::DecodeBase58(prikeyBase58);

			CMBlock cbTmp;
			cbTmp.SetMemFixed((const uint8_t *) (void *) prikey, prikey.GetSize());
			CMBlock privKey = Key::getAuthPrivKeyForAPI(cbTmp);

			_localStore.SetEncryptedKey(Utils::encrypt(privKey, payPassword));
			initPublicKey(payPassword);

			_initialized = true;
			return true;
		}

		void MasterWallet::initFromLocalStore(const MasterWalletStore &localStore) {
			resetMnemonic(localStore.GetLanguage());
			_idAgentImpl = boost::shared_ptr<IdAgentImpl>(new IdAgentImpl(this, localStore.GetIdAgentInfo()));
			initSubWallets(localStore.GetSubWalletInfoList());
			_initialized = true;
		}

		void MasterWallet::initSubWallets(const std::vector<CoinInfo> &coinInfoList) {
			for (int i = 0; i < coinInfoList.size(); ++i) {
				SubWallet *subWallet = new SubWallet(coinInfoList[i], ChainParams::mainNet(), "", this);
				startPeerManager(subWallet);
				_createdWallets[subWallet->GetChainId()] = subWallet;
			}
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(_localStore.GetEncrpytedKey(), payPassword);
			ParamChecker::checkDataNotEmpty(keyData);

			Key key;
			char stmp[keyData.GetSize()];
			memcpy(stmp, keyData, keyData.GetSize());
			std::string secret(stmp, keyData.GetSize());
			key.setPrivKey(secret);
			return key;
		}

		void MasterWallet::initPublicKey(const std::string &payPassword) {
			Key key = deriveKey(payPassword);
			//todo throw exception here
			if (key.getPrivKey() == "") {
				return;
			}
			CMBlock data = key.getPubkey();
			_localStore.SetPublicKey(Utils::encodeHex(data));
		}

		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			ParamChecker::checkNotEmpty(message);
			ParamChecker::checkPassword(payPassword);

			Key key = deriveKey(payPassword);

			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());

			CMBlock signedData = key.sign(md);

			char data[signedData.GetSize()];
			memcpy(data, signedData, signedData.GetSize());
			std::string singedMsg(data, signedData.GetSize());
			return singedMsg;
		}

		nlohmann::json
		MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			CMBlock signatureData(signature.size());
			memcpy(signatureData, signature.c_str(), signature.size());

			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());

			bool r = Key::verifyByPublicKey(publicKey, md, signatureData);
			nlohmann::json jsonData;
			jsonData["Result"] = r;
			return jsonData;
		}

		bool MasterWallet::IsIdValid(const std::string &id) {
			return Address::isValidIdAddress(id);
		}

		UInt512 MasterWallet::deriveSeed(const std::string &payPassword) {
			UInt512 result;
			CMBlock entropyData = Utils::decrypt(_localStore.GetEncryptedMnemonic(), payPassword);
			ParamChecker::checkDataNotEmpty(entropyData, false);

			CMemBlock<char> mnemonic(entropyData.GetSize() + 1);
			mnemonic.Zero();
			memcpy(mnemonic, entropyData, entropyData.GetSize());

			std::string phrasePassword = _localStore.GetEncrptedPhrasePassword().GetSize() == 0
										 ? ""
										 : Utils::convertToString(
							Utils::decrypt(_localStore.GetEncrptedPhrasePassword(), payPassword));

			std::string prikey_base58 = WalletTool::getDeriveKey_base58(mnemonic, phrasePassword);
			CMemBlock<uint8_t> prikey = BTCBase58::DecodeBase58(prikey_base58);
			assert(prikey.GetSize() == sizeof(result));
			memcpy(&result, prikey, prikey.GetSize());

			return result;
		}

		SubWallet *MasterWallet::SubWalletFactoryMethod(const CoinInfo &info, const ChainParams &chainParams,
														const std::string &payPassword, MasterWallet *parent) {
			switch (info.getWalletType()) {
				case Mainchain:
					return new MainchainSubWallet(info, chainParams, payPassword, parent);
				case Sidechain:
					return new SidechainSubWallet(info, chainParams, payPassword, parent);
				case Idchain:
					return new IdChainSubWallet(info, chainParams, payPassword, parent);
				case Normal:
				default:
					return new SubWallet(info, chainParams, payPassword, parent);
			}
		}

		void MasterWallet::resetMnemonic(const std::string &language) {
			_localStore.SetLanguage(language);
			fs::path mnemonicPath = Enviroment::GetRootPath();
			_mnemonic = boost::shared_ptr<Mnemonic>(new Mnemonic(language, mnemonicPath));
		}

		std::string
		MasterWallet::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index, const std::string &payPassword) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			return _idAgentImpl->DeriveIdAndKeyForPurpose(purpose, index, payPassword);
		}

		nlohmann::json
		MasterWallet::GenerateProgram(const std::string &id, const std::string &message, const std::string &password) {
			nlohmann::json j;
			j["parameter"] = _idAgentImpl->Sign(id, message, password);
			j["code"] = _idAgentImpl->GenerateRedeemScript(id, password);
			return j;
		}

		std::string MasterWallet::Sign(const std::string &id, const std::string &message, const std::string &password) {
			ParamChecker::checkNotEmpty(id);
			ParamChecker::checkNotEmpty(message);
			ParamChecker::checkPassword(password);

			return _idAgentImpl->Sign(id, message, password);
		}

		std::vector<std::string> MasterWallet::GetAllIds() const {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			return _idAgentImpl->GetAllIds();
		}

		std::string MasterWallet::GetPublicKey(const std::string &id) {
			return _idAgentImpl->GetPublicKey(id);
		}

		void MasterWallet::startPeerManager(SubWallet *wallet) {
			wallet->_walletManager->start();
		}

		void MasterWallet::stopPeerManager(SubWallet *wallet) {
			wallet->_walletManager->stop();
		}

		bool MasterWallet::IsAddressValid(const std::string &address) {
			return Address::isValidAddress(address);
		}

		std::vector<std::string> MasterWallet::GetSupportedChains() {
			tryInitCoinConfig();
			return _coinConfigReader.GetAllChainId();
		}

		void MasterWallet::ChangePassword(const std::string &oldPassword, const std::string &newPassword) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			ParamChecker::checkPassword(oldPassword);
			ParamChecker::checkPassword(newPassword);

			CMBlock key = Utils::decrypt(_localStore.GetEncrpytedKey(), oldPassword);
			ParamChecker::checkDataNotEmpty(key, false);
			CMBlock phrasePass = Utils::decrypt(_localStore.GetEncrptedPhrasePassword(), oldPassword);
			ParamChecker::checkDataNotEmpty(phrasePass, false);
			CMBlock mnemonic = Utils::decrypt(_localStore.GetEncryptedMnemonic(), oldPassword);
			ParamChecker::checkDataNotEmpty(mnemonic, false);

			_localStore.SetEncryptedKey(Utils::encrypt(key, newPassword));
			_localStore.SetEncryptedPhrasePassword(Utils::encrypt(phrasePass, newPassword));
			_localStore.SetEncryptedMnemonic(Utils::encrypt(mnemonic, newPassword));
		}

		void MasterWallet::ResetAddressCache(const std::string &payPassword) {
			std::for_each(_createdWallets.begin(), _createdWallets.end(),
						  [&payPassword](const WalletMap::value_type &item) {
							  SubWallet *wallet = dynamic_cast<SubWallet *>(item.second);
							  wallet->ResetAddressCache(payPassword);
						  });
		}

		void MasterWallet::tryInitCoinConfig() {
			if (!_coinConfigReader.IsInitialized()) {
				boost::filesystem::path configPath = Enviroment::GetRootPath();
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
			CMBlock phrasePassRaw0 = Utils::convertToMemBlock<unsigned char>(
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

			initSubWallets(keyStore.json().getCoinInfoList());
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

	}
}