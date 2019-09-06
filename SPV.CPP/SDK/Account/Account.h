// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ACCOUNT_H__
#define __ELASTOS_SDK_ACCOUNT_H__

#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/Common/Mstream.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/SpvService/LocalStore.h>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

#define MAX_MULTISIGN_COSIGNERS 6

		class Account {
		public:
			enum SignType {
				Standard,
				MultiSign,
			};

		public:
			// for test
			Account(const LocalStorePtr &store);

			// init from localstore
			Account(const std::string &path);

			// multi-sign readonly
			Account(const std::string &path, const std::vector<PublicKeyRing> &cosigners, int m, bool singleAddress,
					bool compatible);

			// multi-sign xprv
			Account(const std::string &path, const std::string &xprv, const std::string &payPasswd,
					const std::vector<PublicKeyRing> &cosigners, int m, bool singleAddress, bool compatible);

			// multi-sign mnemonic + passphrase
			Account(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
					const std::string &payPasswd, const std::vector<PublicKeyRing> &cosigners, int m,
					bool singleAddress, bool compatible);

			// HD standard with mnemonic + passphrase
			Account(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
					const std::string &payPasswd, bool singleAddress);

			// Read-Only wallet JSON
			Account(const std::string &path, const nlohmann::json &walletJSON);

			// keystore
			Account(const std::string &path, const KeyStore &ks, const std::string &payPasswd);

			bytes_t RequestPubKey() const;

			Key RequestPrivKey(const std::string &payPassword) const;

			HDKeychainPtr RootKey(const std::string &payPassword) const;

			HDKeychainPtr MasterPubKey() const;

			std::string GetxPrvKeyString(const std::string &payPasswd) const;

			const std::string &MasterPubKeyString() const;

			const std::string &MasterPubKeyHDPMString() const;

			const std::vector<PublicKeyRing> &MasterPubKeyRing() const;

			bytes_t OwnerPubKey() const;

			void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			nlohmann::json GetBasicInfo() const;

			SignType GetSignType() const;

			bool Readonly() const;

			bool SingleAddress() const;

			bool Equal(const Account &account) const;

			int GetM() const;

			int GetN() const;

			const std::string &DerivationStrategy() const;

			nlohmann::json GetPubKeyInfo() const;

			HDKeychainPtr MultiSignSigner() const;

			HDKeychainArray MultiSignCosigner() const;

			int CosignerIndex() const;

			const std::vector<CoinInfoPtr> &SubWalletInfoList() const;

			void AddSubWalletInfoList(const CoinInfoPtr &info);

			void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &info);

			void RemoveSubWalletInfo(const CoinInfoPtr &info);

			KeyStore ExportKeyStore(const std::string &payPasswd);

			nlohmann::json ExportReadonlyWallet() const;

			bool ImportReadonlyWallet(const nlohmann::json &walletJSON);

			std::string GetDecryptedMnemonic(const std::string &payPasswd) const;

			bool VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const;

			bool VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const;

			bool VerifyPayPassword(const std::string &payPasswd) const;

			void Save();

			void RegenerateKey(const std::string &payPasswd) const;
		private:
			void Init() const;

		private:
			LocalStorePtr _localstore;
			mutable HDKeychainPtr _xpub;
			mutable int _cosignerIndex;
			mutable HDKeychainPtr _curMultiSigner; // multi sign current wallet signer
			mutable HDKeychainArray _allMultiSigners; // including _multiSigner and sorted
			mutable bytes_t _ownerPubKey, _requestPubKey;
		};

		typedef boost::shared_ptr<Account> AccountPtr;

	}
}

#endif //__ELASTOS_SDK_ACCOUNT_H__
