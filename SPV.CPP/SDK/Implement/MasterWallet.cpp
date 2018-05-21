// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"
#include "Log.h"

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
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {
			//todo recover entropy by mnemonic and phrasePassword
			//todo create private key by payPassword and entropy
			return false;
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			return false;
		}

		bool MasterWallet::exportMnemonic(const std::string &phrasePassword, std::string &mnemonic) {
			//todo compare phrase password with phrase hash first
			//todo generate mnemonic from entropy
			return false;
		}

		bool MasterWallet::Initialized() const {
			return _initialized;
		}
	}
}