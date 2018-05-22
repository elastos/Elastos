// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRBIP39Mnemonic.h"

#include "Utils.h"
#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "Log.h"
#include "Mnemonic.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

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
									  bool singleAddress) {

			if (!Initialized()) {
				Log::warn("Current master wallet is not initialized.");
				return nullptr;
			}

			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			//todo [zxb] generate master public key of sub wallet[by coinTypeIndex]
			Key masterPrivKey = deriveKey(payPassword);
			MasterPubKeyPtr masterPubKey;

			fs::path subWalletDbPath = _dbRoot;
			subWalletDbPath /= _name + chainID + DB_FILE_EXTENSION;

			SubWallet *subWallet = new SubWallet(masterPubKey, subWalletDbPath, 0, singleAddress,
												 ChainParams::mainNet());
			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									   bool singleAddress, int limitGap) {
			ISubWallet *subWallet = _createdWallets.find(chainID) == _createdWallets.end()
									? CreateSubWallet(chainID, coinTypeIndex, payPassword, singleAddress)
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
			return (const char *) (void *) _masterPubKey->getPubKey();
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
			//todo get phrase password from key store
			std::string phrasePass;

			initFromEntropy(entropy, phrasePass, payPassword);
			return true;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {

			if(!MasterPubKey::validateRecoveryPhrase(_mnemonic.words(), mnemonic)) {
				Log::error("Invalid mnemonic.");
				return false;
			}

			initFromPhrase(mnemonic, phrasePassword, payPassword);
			return false;
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			if(!_keyStore.save(keystorePath, backupPassword)) {
				Log::error("Export key error.");
				return false;
			}

			return true;
		}

		bool MasterWallet::exportMnemonic(const std::string &phrasePassword, std::string &mnemonic) {

			//todo compare phrase password with phrase hash first

			//todo [zxb] entropy from _encryptedEntropy
			UInt128 entropy;
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
			UInt512 key = UINT512_ZERO;
			BRBIP39DeriveKey(key.u8, phrase.c_str(), phrasePassword.c_str());

			//todo [zxb] init _encryptedEntropy by phrase

			//todo [zxb] init master public key and private key
			CMBlock privKey;
			_encryptedKey = Utils::encrypt(privKey, payPassword);

			return false;
		}

		Key MasterWallet::deriveKey(const std::string &payPassword) {
			CMBlock keyData = Utils::decrypt(_encryptedKey, payPassword);

			//todo [zxb] key data to key
			return Key();
		}

	}
}