// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterPubKey.h"
#include "MasterWallet.h"
#include "SubWallet.h"

namespace Elastos {
	namespace SDK {

		MasterWallet::MasterWallet() :
				_initialized(false),
				_name("") {

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
			if (_createdWallets.find(chainID) != _createdWallets.end())
				return _createdWallets[chainID];

			//todo generate master public key of sub wallet
			MasterPubKeyPtr masterPubKey;
			SubWallet *subWallet = new SubWallet(_key, masterPubKey);
			_createdWallets[chainID] = subWallet;
			return subWallet;
		}

		ISubWallet *
		MasterWallet::RecoverSubWallet(const std::string &chainID, int coinTypeIndex, const std::string &payPassword,
									   bool singleAddress, int limitGap) {
			ISubWallet *subWallet = _createdWallets.find(chainID) == _createdWallets.end()
									? CreateSubWallet(chainID, coinTypeIndex, payPassword, singleAddress)
									: _createdWallets[chainID];
			//todo recover logic
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
			//todo return master public key
			return "";
		}

		const std::string &MasterWallet::GetName() const {
			return _name;
		}

		bool MasterWallet::importFromKeyStore(const std::string &keystorePath, const std::string &backupPassword,
											  const std::string &payPassword) {
			return false;
		}

		bool MasterWallet::importFromMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
											  const std::string &payPassword) {
			return false;
		}

		bool MasterWallet::exportKeyStore(const std::string &backupPassword, const std::string &keystorePath) {
			return false;
		}

		bool MasterWallet::exportMnemonic(const std::string &phrasePassword, std::string &mnemonic) {
			return false;
		}

		bool MasterWallet::Initialized() const {
			return _initialized;
		}
	}
}