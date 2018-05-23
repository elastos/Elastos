// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRBase58.h>
#include "BRBIP39Mnemonic.h"
#include "Utils.h"
#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		MasterWallet::MasterWallet() :
				_initialized(false),
				_name(""),
				_dbRoot("db"),
				_mnemonic("english") {
		}

		MasterWallet::MasterWallet(const std::string &name, const std::string &phrasePassword,
								   const std::string &payPassword) :
				_initialized(true),
				_name(name),
				_mnemonic("english") {

			UInt128 entropy = Utils::generateRandomSeed();
			initFromEntropy(entropy, phrasePassword, payPassword);
		}

		MasterWallet::~MasterWallet() {

		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									  bool singleAddress, double balanceUnit, uint64_t feePerKb) {

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
			info.setBalanceUnit(balanceUnit);
			info.setFeePerKb(feePerKb);
			SubWallet *subWallet = new SubWallet(info, ChainParams::mainNet(), this);
			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									   bool singleAddress, int limitGap, double balanceUnit, uint64_t feePerKb) {
			ISubWallet *subWallet = _createdWallets.find(chainID) == _createdWallets.end()
									? CreateSubWallet(chainID, coinTypeIndex, payPassword, singleAddress, balanceUnit,
													  feePerKb)
									: _createdWallets[chainID];
			SubWallet *walletInner = static_cast<SubWallet *>(subWallet);
			walletInner->recover(limitGap);

			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		void MasterWallet::DestroyWallet(ISubWallet *wallet) {
			_createdWallets.erase(std::find_if(_createdWallets.begin(), _createdWallets.end(),
											   [wallet](const WalletMap::value_type &item) {
												   return item.second == wallet;
											   }));
		}

		std::string MasterWallet::GetPublicKey() {
			return _publicKey;
		}

		const std::string &MasterWallet::GetName() const {
			return _name;
		}

		bool MasterWallet::importFromKeyStore(const std::string &keystorePath, const std::string &backupPassword,
											  const std::string &payPassword) {
			if (!_keyStore.open(keystorePath, backupPassword)) {
				Log::error("Import key error.");
				return false;
			}

			//todo get entropy from keystore
			UInt128 entropy;
			CMemBlock<unsigned char> phrasePassRaw = Utils::decrypt(Utils::convertToMemBlock<unsigned char>(
					_keyStore.json().getEncryptedPhrasePassword()), payPassword);
			std::string phrasePass = Utils::convertToString(phrasePassRaw);

			initFromEntropy(entropy, phrasePass, payPassword);
			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {

			if (!MasterPubKey::validateRecoveryPhrase(_mnemonic.words(), mnemonic)) {
				Log::error("Invalid mnemonic.");
				return false;
			}

			initFromPhrase(mnemonic, phrasePassword, payPassword);
			return false;
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			if (_keyStore.json().getEncryptedPhrasePassword().empty())
				_keyStore.json().setEncryptedPhrasePassword((const char *) (unsigned char *) _encryptedPhrasePass);

			_keyStore.json().clearCoinInfo();
			std::for_each(_createdWallets.begin(), _createdWallets.end(), [this](const WalletMap::value_type &item) {
				_keyStore.json().addCoinInfo(((SubWallet *) item.second)->_info);
			});

			if (!_keyStore.save(keystorePath, backupPassword)) {
				Log::error("Export key error.");
				return false;
			}

			return true;
		}

		bool MasterWallet::exportMnemonic(const std::string &payPassword, std::string &mnemonic) {

			//todo compare phrase password with phrase hash first

			//todo [zxb] entropy from _encryptedEntropy
			CMemBlock<unsigned char> entropyBytes = Utils::decrypt(_encryptedEntropy, payPassword);
			UInt128 entropy = UINT128_ZERO;
			memcpy(entropy.u8, entropyBytes, sizeof(entropyBytes));
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
			UInt512 key = UINT512_ZERO;
			BRBIP39DeriveKey(key.u8, phrase.c_str(), phrasePassword.c_str());

			CMemBlock<unsigned char> encryptedPhrasePass = Utils::convertToMemBlock<unsigned char>(phrasePassword);
			_encryptedPhrasePass = Utils::encrypt(encryptedPhrasePass, payPassword);

			//todo [zxb] init _encryptedEntropy by phrase
			UInt128 entropy = UINT128_ZERO;

			std::vector<std::string> words = _mnemonic.words();
			size_t len = words.size();
			const char *wordList[len];
			for (size_t i = 0; i < len; ++i) {
				wordList[i] = words[i].c_str();
			}

			BRBIP39Decode(entropy.u8, sizeof(entropy), wordList, phrase.c_str());

			CMemBlock<unsigned char> entropyBytes(sizeof(entropy));
			memcpy(entropyBytes, entropy.u8, sizeof(entropy));
			_encryptedEntropy = Utils::encrypt(entropyBytes, payPassword);

			//todo [zxb] init master public key and private key
			initPublicKey(payPassword);

			CMBlock phraseBlock(phrase.size() + 1);
			phraseBlock[phrase.size()] = '\0';
			memcpy(phraseBlock, phrase.c_str(), phrase.size());
			CMBlock seed = Key::getSeedFromPhrase(phraseBlock, phrasePassword);

			CMBlock privKey = Key::getAuthPrivKeyForAPI(seed);

			CMemBlock<unsigned char> secretKey(privKey.GetSize());
			memcpy(secretKey, privKey, privKey.GetSize());

			_encryptedKey = Utils::encrypt(secretKey, payPassword);

			MasterPubKeyPtr masterPubKey = MasterPubKeyPtr(new MasterPubKey(phrase, phrasePassword));
			CMBlock pubKey = masterPubKey->getPubKey();
			pubKey[pubKey.GetSize() + 1] = '\0';
			_publicKey = (char *)(void *)pubKey;
			return false;
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMemBlock<unsigned char> keyData = Utils::decrypt(_encryptedKey, payPassword);

			//todo [zxb] key data to key
			Key key;
			std::string secret = (char *)(void *)keyData;
			key.setPrivKey(secret);
			return key;
		}

		void MasterWallet::initPublicKey(const std::string &payPassword) {
			Key key = deriveKey(payPassword);

			size_t len = BRKeyPubKey(key.getRaw(), nullptr, 0);
			uint8_t pubKey[len];
			BRKeyPubKey(key.getRaw(), &pubKey, len);
			size_t strLen = BRBase58Encode(nullptr, 0, pubKey, len);
			char *result = new char[strLen];
			BRBase58Encode(result, strLen, pubKey, len);

			_publicKey = std::string(result);
		}

		std::string MasterWallet::Sign(const std::string &message, const std::string &payPassword) {
			Key key = deriveKey(payPassword);
			CMBlock messageData;
			messageData.SetMemFixed((const uint8_t *)message.data(), message.size());
		 	CMBlock signedData = key.compactSign(messageData);
			signedData[signedData.GetSize() + 1] = '\0';
			std::string singedMsg = (char *)(void *)signedData;

			return singedMsg;
		}

		nlohmann::json
		MasterWallet::CheckSign(const std::string &address, const std::string &message, const std::string &signature) {
			//todo complete me
			return nlohmann::json();
		}

	}
}