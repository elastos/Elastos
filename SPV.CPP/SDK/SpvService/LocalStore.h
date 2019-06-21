// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_LOCALSTORE_H__
#define __ELASTOS_SDK_LOCALSTORE_H__

#include <SDK/IDAgent/IDAgentImpl.h>
#include <SDK/WalletCore/KeyStore/KeyStore.h>

#include <boost/filesystem.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo;

		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

		class LocalStore {
		public:
			// for test case
			LocalStore(const nlohmann::json &store);

			LocalStore(const std::string &path);

			LocalStore(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
					   bool singleAddress, const std::string &payPasswd);

			LocalStore(const std::string &path, const ElaNewWalletJson &json, const std::string &payPasswd);

			LocalStore(const std::string &path, const std::vector<std::string> &pubkeys, int m);

			~LocalStore();

			void GetReadOnlyWalletJson(ElaNewWalletJson &json);

			void GetWalletJson(ElaNewWalletJson &json, const std::string &payPasswd);

			void RegenerateKey(const std::string &payPasswd);

			void ChangePasswd(const std::string &oldPasswd, const std::string &newPasswd);

			void Save();

			bool SingleAddress() const { return _singleAddress; }

			void SetSingleAddress(bool status) { _singleAddress = status; }

			const std::string &GetxPrivKey() const { return _xPrivKey; }

			void SetxPrivKey(const std::string &xprvkey) { _xPrivKey = xprvkey; }

			const std::string &GetRequestPrivKey() const { return _requestPrivKey; }

			void SetRequestPrivKey(const std::string &prvkey) { _requestPrivKey = prvkey; }

			const std::string &GetMnemonic() const { return _mnemonic; }

			void SetMnemonic(const std::string &mnemonic) { _mnemonic = mnemonic; }

			const std::string &GetPassPhrase() const { return _passphrase; }

			void SetPassPhrase(const std::string &passphrase) { _passphrase = passphrase; }

			const std::string &GetxPubKey() const { return _xPubKey; }

			void SetxPubKey(const std::string &xpubkey) { _xPubKey = xpubkey; }

			const std::string &GetRequestPubKey() const { return _requestPubKey; }

			void SetRequestPubKey(const std::string &pubkey) { _requestPubKey = pubkey; }

			const std::string &GetOwnerPubKey() const { return _ownerPubKey; }

			void SetOwnerPubKey(const std::string &ownerPubKey) { _ownerPubKey = ownerPubKey; }

			const std::vector<PublicKeyRing> &GetPublicKeyRing() const { return _publicKeyRing; }

			void AddPublicKeyRing(const PublicKeyRing &ring) { _publicKeyRing.push_back(ring); }

			void SetPublicKeyRing(const std::vector<PublicKeyRing> &pubKeyRing) { _publicKeyRing = pubKeyRing; }

			int GetM() const { return _m; }

			void SetM(int m) { _m = m; }

			int GetN() const { return _n; }

			void SetN(int n) { _n = n; }

			bool HasPassPhrase() const { return _mnemonicHasPassphrase; }

			void SetHasPassPhrase(bool has) { _mnemonicHasPassphrase = has; }

			bool Readonly() const { return _readonly; }

			void SetReadonly(bool status) { _readonly = status; }

			const std::vector<CoinInfoPtr> &GetSubWalletInfoList() const { return _subWalletsInfoList; }

			void AddSubWalletInfoList(const CoinInfoPtr &info) { _subWalletsInfoList.push_back(info); }

			void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &infoList) { _subWalletsInfoList = infoList; }

			void ClearSubWalletInfoList() { _subWalletsInfoList.clear(); }

		private:
			TO_JSON(LocalStore);

			FROM_JSON(LocalStore);

		private:
			// encrypted
			std::string _xPrivKey;
			std::string _requestPrivKey;
			std::string _mnemonic;
			// only old version keystore and localstore of spvsdk contain this. will remove later
			std::string _passphrase __attribute__((deprecated));

			// plain text
			std::string _xPubKey;
			std::string _requestPubKey;
			std::string _ownerPubKey;
			std::string _derivationStrategy;

			//std::string _addressType;
			std::vector<PublicKeyRing> _publicKeyRing;

			// request signature
			int _m;
			// total request public keys
			int _n;

			int _account;

			bool _mnemonicHasPassphrase;
			bool _singleAddress;
			bool _readonly;

			std::vector<CoinInfoPtr> _subWalletsInfoList;
		private:
			std::string _path; // rootPath + masterWalletID
		};

		typedef boost::shared_ptr<LocalStore> LocalStorePtr;

	}
}

#endif //__ELASTOS_SDK_LOCALSTORE_H__
