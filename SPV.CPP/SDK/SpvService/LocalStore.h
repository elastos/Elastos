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

			LocalStore(const std::string &path, const std::string &xprv, bool singleAddress, const std::string &payPasswd);

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

			void SaveTo(const std::string &path);

			bool SingleAddress() const;

			void SetSingleAddress(bool status);

			const std::string &GetxPrivKey() const;

			void SetxPrivKey(const std::string &xprvkey);

			const std::string &GetRequestPrivKey() const;

			void SetRequestPrivKey(const std::string &prvkey);

			const std::string &GetMnemonic() const;

			void SetMnemonic(const std::string &mnemonic);

			const std::string &GetPassPhrase() const;

			void SetPassPhrase(const std::string &passphrase);

			const std::string &GetxPubKey() const;

			void SetxPubKey(const std::string &xpubkey);

			const std::string &GetRequestPubKey() const;

			void SetRequestPubKey(const std::string &pubkey);

			const std::string &GetOwnerPubKey() const;

			void SetOwnerPubKey(const std::string &ownerPubKey);

			const std::vector<PublicKeyRing> &GetPublicKeyRing() const;

			void AddPublicKeyRing(const PublicKeyRing &ring);

			void SetPublicKeyRing(const std::vector<PublicKeyRing> &pubKeyRing);

			int GetM() const;

			void SetM(int m);

			int GetN() const;

			void SetN(int n);

			bool HasPassPhrase() const;

			void SetHasPassPhrase(bool has);

			bool Readonly() const;

			void SetReadonly(bool status);

			const std::vector<CoinInfoPtr> &GetSubWalletInfoList() const;

			void AddSubWalletInfoList(const CoinInfoPtr &info);

			void RemoveSubWalletInfo(const CoinInfoPtr &info);

			void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &infoList);

			void ClearSubWalletInfoList();

		private:
			TO_JSON(LocalStore);

			FROM_JSON(LocalStore);

		private:
			// encrypted
			std::string _xPrivKey;
			std::string _requestPrivKey;
			std::string _mnemonic;
			// only old version keystore and localstore of spvsdk contain this. will remove later
//			std::string _passphrase __attribute__((deprecated));
			std::string _passphrase;

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
