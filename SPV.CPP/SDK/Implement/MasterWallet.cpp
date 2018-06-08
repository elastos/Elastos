// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include "BRBase58.h"
#include "BRBIP39Mnemonic.h"

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

#define MNEMONIC_FILE_PREFIX "mnemonic_"
#define MNEMONIC_FILE_EXTENSION ".txt"


namespace fs = boost::filesystem;

namespace Elastos {
	namespace SDK {

		MasterWallet::MasterWallet(const std::string &language) :
				_initialized(false),
				_dbRoot("Data") {

			ParamChecker::checkNotEmpty(language);

			resetMnemonic(language);
			_keyStore.json().setMnemonicLanguage(language);
		}

		MasterWallet::MasterWallet(const std::string &phrasePassword,
								   const std::string &payPassword,
								   const std::string &language,
								   const std::string &rootPath) :
				_initialized(true),
				_dbRoot(rootPath) {

			ParamChecker::checkPasswordWithNullLegal(phrasePassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkNotEmpty(language);
			ParamChecker::checkNotEmpty(rootPath);

			resetMnemonic(language);
			_keyStore.json().setMnemonicLanguage(language);

			UInt128 entropy = Utils::generateRandomSeed();
			initFromEntropy(entropy, phrasePassword, payPassword);
		}

		MasterWallet::~MasterWallet() {

		}

		ISubWallet *
		MasterWallet::CreateSubWallet(SubWalletType type, const std::string &chainID, uint32_t coinTypeIndex,
									  const std::string &payPassword, bool singleAddress, uint64_t feePerKb) {

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
			info.setWalletType(type);
			info.setEaliestPeerTime(0);
			info.setIndex(coinTypeIndex);
			info.setSingleAddress(singleAddress);
			info.setUsedMaxAddressIndex(0);
			info.setChainId(chainID);
			info.setFeePerKb(feePerKb);
			SubWallet *subWallet = SubWalletFactoryMethod(info, ChainParams::mainNet(), payPassword, this);
			_createdWallets[chainID] = subWallet;
			startPeerManager(subWallet);
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(SubWalletType type, const std::string &chainID, uint32_t coinTypeIndex,
									   const std::string &payPassword, bool singleAddress, uint32_t limitGap,
									   uint64_t feePerKb) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			if (limitGap > SEQUENCE_GAP_LIMIT_EXTERNAL) {
				throw std::invalid_argument("Limit gap should less than or equal 10.");
			}

			ISubWallet *subWallet = CreateSubWallet(type, chainID, coinTypeIndex, payPassword, singleAddress, feePerKb);
			SubWallet *walletInner = dynamic_cast<SubWallet *>(subWallet);
			assert(walletInner != nullptr);
			walletInner->recover(limitGap);

			_createdWallets[chainID] = subWallet;
			return subWallet;
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

			return _publicKey;
		}

		bool MasterWallet::importFromKeyStore(const std::string &keystorePath, const std::string &backupPassword,
											  const std::string &payPassword, const std::string &phrasePassword,
											  const std::string &rootPath) {
			ParamChecker::checkNotEmpty(keystorePath);
			ParamChecker::checkPassword(backupPassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);
			ParamChecker::checkNotEmpty(rootPath);

			_dbRoot = rootPath;

			if (!_keyStore.open(keystorePath, backupPassword)) {
				Log::error("Import key error.");
				return false;
			}

			CMBlock phrasePass_Raw0 = Utils::convertToMemBlock<unsigned char>(
					_keyStore.json().getEncryptedPhrasePassword());
			std::string phrasePass = "";
			if (true == phrasePass_Raw0) {
				CMBlock phrasePass_Raw1(phrasePass_Raw0.GetSize() + 1);
				phrasePass_Raw1.Zero();
				memcpy(phrasePass_Raw1, phrasePass_Raw0, phrasePass_Raw0.GetSize());
				CMBlock phrasePassRaw = Utils::decrypt(phrasePass_Raw1, payPassword);
				phrasePass = Utils::convertToString(phrasePassRaw);
			}

			CMBlock entropy0 = Utils::convertToMemBlock<unsigned char>(
					_keyStore.json().getEncryptedEntropySource());
			//resetMnemonic must before initFromEntropy because initFromEntropy use _mnemonic !!!!
			if (phrasePass == "") {
				// todo fixme to restore from keystore of webwallet
			}
			else {
				CMBlock entropy1(entropy0.GetSize() + 1);
				entropy1.Zero();
				memcpy(entropy1, entropy0, entropy0.GetSize());
				CMBlock entropy2 = Utils::decrypt(entropy1, payPassword);
				if (false == entropy2) {
					return false;
				}
				UInt128 entropy;
				memcpy((void *) &entropy.u8[0], entropy2, sizeof(UInt128));

				resetMnemonic(_keyStore.json().getMnemonicLanguage());
				initFromEntropy(entropy, phrasePass, payPassword);
			}

			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword, const std::string &rootPath) {
			ParamChecker::checkNotEmpty(mnemonic);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);
			ParamChecker::checkNotEmpty(rootPath);

			_dbRoot = rootPath;

			if (!MasterPubKey::validateRecoveryPhrase(_mnemonic.words(), mnemonic)) {
				Log::error("Invalid mnemonic.");
				return false;
			}

			return initFromPhrase(mnemonic, phrasePassword, payPassword);
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			if (_keyStore.json().getEncryptedPhrasePassword().empty())
				_keyStore.json().setEncryptedPhrasePassword((const char *) (unsigned char *) _encryptedPhrasePass);
			if (_keyStore.json().getEncryptedEntropySource().empty())
				_keyStore.json().setEncryptedEntropySource((const char *) (unsigned char *) _encryptedEntropy);

			_keyStore.json().clearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(), [this](const WalletMap::value_type &item) {
				SubWallet *subWallet = dynamic_cast<SubWallet *>(item.second);
				_keyStore.json().addCoinInfo(subWallet->_info);
			});

			if (!_keyStore.save(keystorePath, backupPassword)) {
				Log::error("Export key error.");
				return false;
			}

			return true;
		}

		bool MasterWallet::exportMnemonic(const std::string &payPassword, std::string &mnemonic) {

			CMBlock entropyBytes = Utils::decrypt(_encryptedEntropy, payPassword);
			UInt128 entropy = UINT128_ZERO;
			memcpy(entropy.u8, entropyBytes, entropyBytes.GetSize());
			mnemonic = MasterPubKey::generatePaperKey(entropy, _mnemonic.words());
			return true;
		}

		bool MasterWallet::Initialized() const {
			return _initialized;
		}

		bool MasterWallet::initFromEntropy(const UInt128 &entropy, const std::string &phrasePassword,
										   const std::string &payPassword) {

			std::string phrase = MasterPubKey::generatePaperKey(entropy, _mnemonic.words());
			return initFromPhrase(phrase, phrasePassword, payPassword);
		}

		bool MasterWallet::initFromPhrase(const std::string &phrase, const std::string &phrasePassword,
										  const std::string &payPassword) {
			assert(phrase.size() > 0);
			CMBlock encryptedPhrasePass = Utils::convertToMemBlock<unsigned char>(phrasePassword);
			_encryptedPhrasePass = Utils::encrypt(encryptedPhrasePass, payPassword);

			//init _encryptedEntropy by phrase
			UInt128 entropy = UINT128_ZERO;

			std::vector<std::string> words = _mnemonic.words();
			size_t len = words.size();
			const char *wordList[len];
			for (size_t i = 0; i < len; ++i) {
				wordList[i] = words[i].c_str();
			}

			BRBIP39Decode(entropy.u8, sizeof(entropy), wordList, phrase.c_str());

			CMBlock entropyBytes;
			entropyBytes.SetMemFixed((const unsigned char *) entropy.u8, sizeof(entropy));
			_encryptedEntropy = Utils::encrypt(entropyBytes, payPassword);

			//init master public key and private key
			CMBlock phraseBlock;
			phraseBlock.SetMemFixed((const uint8_t *) phrase.c_str(), phrase.size() + 1);
			CMBlock seed = Key::getSeedFromPhrase(phraseBlock, phrasePassword);

			CMBlock privKey = Key::getAuthPrivKeyForAPI(seed);

			_encryptedKey = Utils::encrypt(privKey, payPassword);
			initPublicKey(payPassword);

			_initialized = true;
			return true;
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(_encryptedKey, payPassword);
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
			size_t len = BRKeyPubKey(key.getRaw(), nullptr, 0);
			uint8_t pubKey[len];
			BRKeyPubKey(key.getRaw(), pubKey, len);
			CMBlock data;
			data.SetMemFixed(pubKey, len);
			_publicKey = Key::encodeHex(data);
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
			CMBlock entropyData = Utils::decrypt(_encryptedEntropy, payPassword);
			ParamChecker::checkDataNotEmpty(entropyData, false);

			UInt128 entropy;
			UInt128Get(&entropy, (void *) entropyData);

			std::string phrasePassword = _encryptedPhrasePass.GetSize() == 0
										 ? ""
										 : Utils::convertToString(Utils::decrypt(_encryptedPhrasePass, payPassword));

			std::string phrase = MasterPubKey::generatePaperKey(entropy, _mnemonic.words());
			BRBIP39DeriveKey(result.u8, phrase.c_str(), phrasePassword.c_str());
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
			fs::path mnemonicPath = _dbRoot;
			//mnemonicPath /= MNEMONIC_FILE_PREFIX + language + MNEMONIC_FILE_EXTENSION;
			_mnemonic = Mnemonic(language, mnemonicPath);
		}

		bool MasterWallet::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index, const std::string &payPassword,
													std::string &id, std::string &key) {
			if (!Initialized())
				throw std::logic_error("Current master wallet is not initialized.");

			ParamChecker::checkPassword(payPassword);

			if (purpose == 44)
				throw std::invalid_argument("Can not use reserved purpose.");

			UInt512 seed = deriveSeed(payPassword);
			BRKey *privkey = new BRKey;
			UInt256 chainCode;
			Key::deriveKeyAndChain(privkey, chainCode, &seed, sizeof(seed), 2, purpose, index);

			Key wrappedKey(privkey);
			id = wrappedKey.keyToAddress(ELA_IDCHAIN);
			key = wrappedKey.toString();
			return true;
		}

		void MasterWallet::startPeerManager(SubWallet *wallet) {
			wallet->_walletManager->start();
		}

		void MasterWallet::stopPeerManager(SubWallet *wallet) {
			wallet->_walletManager->stop();
		}

	}
}