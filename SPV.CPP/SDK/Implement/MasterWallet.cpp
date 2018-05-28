// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRBase58.h>
#include "BRBIP39Mnemonic.h"
#include "Utils.h"
#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "IdChainSubWallet.h"
#include "MainchainSubWallet.h"
#include "SidechainSubWallet.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		MasterWallet::MasterWallet() :
			_initialized(false),
			_dbRoot("Data"),
			_mnemonic("english") {
		}

		MasterWallet::MasterWallet(const std::string &phrasePassword,
								   const std::string &payPassword) :
			_initialized(true),
			_dbRoot("Data"),
			_mnemonic("english") {

			UInt128 entropy = Utils::generateRandomSeed();
			initFromEntropy(entropy, phrasePassword, payPassword);
		}

		MasterWallet::~MasterWallet() {

		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									  bool singleAddress, uint64_t feePerKb) {

			if (!Initialized()) {
				Log::warn("Current master wallet is not initialized.");
				return nullptr;
			}

			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			CoinInfo info;
			info.setEaliestPeerTime(0);
			info.setIndex(coinTypeIndex);
			info.setSingleAddress(singleAddress);
			info.setUsedMaxAddressIndex(0);
			info.setChainId(chainID);
			info.setFeePerKb(feePerKb);
			SubWallet *subWallet = SubWalletFactoryMethod(info, ChainParams::mainNet(), payPassword, this);
			_createdWallets[chainID] = subWallet;
			subWallet->_walletManager->start();
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									   bool singleAddress, int limitGap, uint64_t feePerKb) {
			ISubWallet *subWallet = _createdWallets.find(chainID) == _createdWallets.end()
									? CreateSubWallet(chainID, coinTypeIndex, payPassword, singleAddress, feePerKb)
									: _createdWallets[chainID];
			SubWallet *walletInner = dynamic_cast<SubWallet *>(subWallet);
			assert(walletInner != nullptr);
			walletInner->recover(limitGap);

			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			_createdWallets.erase(std::find_if(_createdWallets.begin(), _createdWallets.end(),
											   [wallet](const WalletMap::value_type &item) {
												   return item.second == wallet;
											   }));
			SubWallet *walletInner = dynamic_cast<SubWallet *>(wallet);
			assert(walletInner != nullptr);
			walletInner->_walletManager->stop();
			delete walletInner;
		}

		std::string MasterWallet::GetPublicKey() {
			return _publicKey;
		}

		bool MasterWallet::importFromKeyStore(const std::string &keystorePath, const std::string &backupPassword,
											  const std::string &payPassword) {
			if (!_keyStore.open(keystorePath, backupPassword)) {
				Log::error("Import key error.");
				return false;
			}

			//todo get entropy from keystore
			CMemBlock<unsigned char> phrasePass_Raw0 = Utils::convertToMemBlock<unsigned char>(
				_keyStore.json().getEncryptedPhrasePassword());
			CMemBlock<unsigned char> phrasePass_Raw1(phrasePass_Raw0.GetSize() + 1);
			phrasePass_Raw1.Zero();
			memcpy(phrasePass_Raw1, phrasePass_Raw0, phrasePass_Raw0.GetSize());
			CMemBlock<unsigned char> phrasePassRaw = Utils::decrypt(phrasePass_Raw1, payPassword);
			std::string phrasePass = Utils::convertToString(phrasePassRaw);

			CMemBlock<unsigned char> entropy0 = Utils::convertToMemBlock<unsigned char>(
				_keyStore.json().getEncryptedEntropySource());
			CMemBlock<unsigned char> entropy1(entropy0.GetSize() + 1);
			entropy1.Zero();
			memcpy(entropy1, entropy0, entropy0.GetSize());
			CMemBlock<unsigned char> entropy2 = Utils::decrypt(entropy1, payPassword);
			UInt128 entropy;
			memcpy((void *)&entropy.u8[0], entropy2, sizeof(UInt128));

			initFromEntropy(entropy, phrasePass, payPassword);
			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {

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

			CMemBlock<unsigned char> entropyBytes = Utils::decrypt(_encryptedEntropy, payPassword);
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
//			UInt512 key = UINT512_ZERO;
//			BRBIP39DeriveKey(key.u8, phrase.c_str(), phrasePassword.c_str());

			CMemBlock<unsigned char> encryptedPhrasePass = Utils::convertToMemBlock<unsigned char>(phrasePassword);
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

			CMemBlock<unsigned char> entropyBytes;
			entropyBytes.SetMemFixed((const unsigned char *) entropy.u8, sizeof(entropy));
			_encryptedEntropy = Utils::encrypt(entropyBytes, payPassword);

			//init master public key and private key
			CMBlock phraseBlock;
			phraseBlock.SetMemFixed((const uint8_t *) phrase.c_str(), phrase.size() + 1);
			CMBlock seed = Key::getSeedFromPhrase(phraseBlock, phrasePassword);

			CMBlock privKey = Key::getAuthPrivKeyForAPI(seed);

			CMemBlock<unsigned char> secretKey;
			secretKey.SetMemFixed((const unsigned char *) (const uint8_t *) privKey, privKey.GetSize());

			_encryptedKey = Utils::encrypt(secretKey, payPassword);
			initPublicKey(payPassword);

			_initialized = true;
			return true;
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMemBlock<unsigned char> keyData = Utils::decrypt(_encryptedKey, payPassword);
			Key key;
			if (true == keyData) {
				char *stmp = new char[keyData.GetSize()];
				memcpy(stmp, keyData, keyData.GetSize());
				std::string secret(stmp, keyData.GetSize());
				key.setPrivKey(secret);
			}
			return key;
		}

		void MasterWallet::initPublicKey(const std::string &payPassword) {
			Key key = deriveKey(payPassword);
			//todo throw exception here
			if (key.getPrivKey() == "") {
				return;
			}
			size_t len = BRKeyPubKey(key.getRaw(), nullptr, 0);
			uint8_t *pubKey = new uint8_t[len];
			BRKeyPubKey(key.getRaw(), pubKey, len);
			_publicKey = std::string((char *) pubKey, len);
		}

		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {
			Key key = deriveKey(payPassword);
			CMBlock signedData = key.sign(Utils::UInt256FromString(message));

			char *data = new char[signedData.GetSize()];
			memcpy(data, signedData, signedData.GetSize());
			std::string singedMsg(data, signedData.GetSize());
			return singedMsg;
		}

		nlohmann::json
		MasterWallet::CheckSign(const std::string &publicKey, const std::string &message,
								const std::string &signature) {
			CMBlock signatureData(signature.size());
			memcpy(signatureData, signature.c_str(), signature.size());

			bool r = Key::verifyByPublicKey(publicKey, Utils::UInt256FromString(message), signatureData);
			//todo json return is correct ?
			nlohmann::json jsonData;
			jsonData["result"] = r;
			return jsonData;
		}

		UInt512 MasterWallet::deriveSeed(const std::string &payPassword) {
			UInt512 result;
			CMemBlock<unsigned char> entropyData = Utils::decrypt(_encryptedEntropy, payPassword);
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
				case CoinInfo::Deposit:
					return new MainchainSubWallet(info, chainParams, payPassword, parent);
				case CoinInfo::Withdraw:
					return new SidechainSubWallet(info, chainParams, payPassword, parent);
				case CoinInfo::IdChain:
					return new IdChainSubWallet(info, chainParams, payPassword, parent);
				case CoinInfo::Normal:
				default:
					return new SubWallet(info, chainParams, payPassword, parent);
			}
		}

	}
}