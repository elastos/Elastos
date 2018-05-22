// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
				_dbRoot("db") {

		}

		MasterWallet::MasterWallet(const std::string &name, const std::string &backupPassword,
								   const std::string &payPassword) :
				_initialized(true),
				_name(name) {

		}

		MasterWallet::~MasterWallet() {

		}

		ISubWallet *
		MasterWallet::CreateSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									  bool singleAddress) {

			if(!Initialized()) {
				Log::warn("Current master wallet is not initialized.");
				return nullptr;
			}

			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			//todo generate master public key of sub wallet[by coinTypeIndex]
			MasterPubKeyPtr masterPubKey;

			fs::path subWalletDbPath = _dbRoot;
			subWalletDbPath /= _name + chainID + DB_FILE_EXTENSION;

			SubWallet *subWallet = new SubWallet(masterPubKey, subWalletDbPath, 0, singleAddress, ChainParams::mainNet());
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
			return (const char *)(void *)_masterPubKey->getPubKey();
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

			_masterPubKey = MasterPubKeyPtr(new MasterPubKey(_keyStore.getMnemonic()));
			//todo create private key by payPassword and entropy

			std::string privateKey = _keyStore.getMasterPrivateKey();
			_key->setPrivKey(privateKey);
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {
			//todo recover entropy by mnemonic and phrasePassword
			//todo create private key by payPassword and entropy

			CMBlock phrase(mnemonic.size());
			memcpy(phrase, mnemonic.c_str(), mnemonic.size());

			CMBlock seed = Key::getSeedFromPhrase(phrase, phrasePassword);

			CMBlock privateKey = Key::getAuthPrivKeyForAPI(seed);

			_key->setPrivKey((char *)(void  *)privateKey);

			return false;
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			return false;
		}

		bool MasterWallet::exportMnemonic(const std::string &phrasePassword, std::string &mnemonic) {
			//todo compare phrase password with phrase hash first
			//todo generate mnemonic from entropy

			UInt128 entropy = UINT128_ZERO;

			CMBlock entropySeed(sizeof(entropy));
			memcpy(entropySeed, entropy.u8, sizeof(entropy));

			Mnemonic mne;
			CMBlock paperKey = MasterPubKey::generatePaperKey(entropySeed, mne.words());
			mnemonic = paperKey;

			return false;
		}

		bool MasterWallet::Initialized() const {
			return _initialized;
		}
	}
}