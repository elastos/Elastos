// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Account.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>


namespace Elastos {
	namespace ElaWallet {

		Account::Account(const LocalStorePtr &store, const std::string &rootpath) :
			_localstore(store),
			_rootpath(rootpath) {
			if (_localstore->GetN() > 1) {
				const std::vector<PublicKeyRing> &pubkeyRing = _localstore->GetPublicKeyRing();
				std::vector<bytes_t> pubkeys;
				for (size_t i = 0; i < pubkeyRing.size(); ++i) {
					bytes_t pubkey(pubkeyRing[i].GetRequestPubKey());
					pubkeys.push_back(pubkey);
				}
				_address = Address(PrefixMultiSign, pubkeys, _localstore->GetM());
			}

		}

		bytes_t Account::RequestPubKey() const {
			return bytes_t(_localstore->GetRequestPubKey());
		}

		HDKeychain Account::RootKey(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

			if (_localstore->GetxPrivKey().empty()) {
				_localstore->RegenerateKey(payPasswd);
			}

			bytes_t extkey = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);

			HDKeychain key(extkey);

			extkey.clean();

			return key;
		}

		Key Account::RequestPrivKey(const std::string &payPassword) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

			if (_localstore->GetRequestPrivKey().empty()) {
				_localstore->RegenerateKey(payPassword);
			}

			bytes_t bytes = AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPassword);

			Key key;
			key.SetPrvKey(bytes);

			bytes.clean();

			return key;
		}

		HDKeychain Account::MasterPubKey() const {
			if (_localstore->GetxPubKey().empty()) {
				ErrorChecker::ThrowLogicException(Error::Key, "this account without master public key");
			}

			bytes_t bytes;
			bool valid = Base58::CheckDecode(_localstore->GetxPubKey(), bytes);
			ErrorChecker::CheckLogic(!valid, Error::Key, "Decode xpub fail");

			return HDKeychain(bytes);
		}

		bytes_ptr Account::OwnerPubKey() const {
			bytes_ptr pubkey(new bytes_t());
			pubkey->setHex(_localstore->GetOwnerPubKey());
			return pubkey;
		}

		const Address &Account::GetAddress() const {
			return _address;
		}

		void Account::ChangePassword(const std::string &oldPasswd, const std::string &newPasswd) {
			if (!_localstore->Readonly()) {
				ErrorChecker::CheckPassword(newPasswd, "New");

				if (_localstore->GetxPrivKey().empty()) {
					_localstore->RegenerateKey(oldPasswd);
				}

				_localstore->ChangePasswd(oldPasswd, newPasswd);
			}
		}

		nlohmann::json Account::GetBasicInfo() const {
			nlohmann::json j;
			if (GetSignType() == MultiSign)
				j["Type"] = "MultiSign";
			else
				j["Type"] = "Standard";

			j["Readonly"] = _localstore->Readonly();
			j["SingleAddress"] = _localstore->SingleAddress();
			j["M"] = _localstore->GetM();
			j["N"] = _localstore->GetN();
			return j;
		}

		Account::SignType Account::GetSignType() const {
			if (_localstore->GetN() > 1)
				return MultiSign;

			return Standard;
		}

		bool Account::ReadOnly() const {
			return _localstore->Readonly();
		}

		bool Account::SingleAddress() const {
			return _localstore->SingleAddress();
		}

		bool Account::Equal(const Account &account) const {
			if (GetSignType() != account.GetSignType() || ReadOnly() != account.ReadOnly())
				return false;

			if (GetSignType() == MultiSign && !ReadOnly()) {
				return RequestPubKey() == RequestPubKey() && GetAddress() == account.GetAddress();
			}

			return _localstore->GetxPubKey() == account._localstore->GetxPubKey();
		}

	}
}
