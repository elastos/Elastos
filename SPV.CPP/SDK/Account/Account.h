/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_ACCOUNT_H__
#define __ELASTOS_SDK_ACCOUNT_H__

#include "IAccount.h"

#include <WalletCore/Mnemonic.h>
#include <WalletCore/Address.h>
#include <SpvService/LocalStore.h>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

#define MAX_MULTISIGN_COSIGNERS 6

		class Account : public IAccount {
		public:
			~Account();

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

			std::string MasterPubKeyString() const;

			std::string MasterPubKeyHDPMString() const;

			std::vector<PublicKeyRing> MasterPubKeyRing() const;

			bytes_t OwnerPubKey() const;

			void ChangePassword(const std::string &oldPassword, const std::string &newPassword);

			nlohmann::json GetBasicInfo() const;

			SignType GetSignType() const;

			bool Readonly() const;

			bool SingleAddress() const;

			bool Equal(const AccountPtr &account) const;

			int GetM() const;

			int GetN() const;

			std::string DerivationStrategy() const;

			nlohmann::json GetPubKeyInfo() const;

			HDKeychainPtr MultiSignSigner() const;

			HDKeychainArray MultiSignCosigner() const;

			int CosignerIndex() const;

			std::vector<CoinInfoPtr> SubWalletInfoList() const;

			void AddSubWalletInfoList(const CoinInfoPtr &info);

			void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &info);

			void RemoveSubWalletInfo(const CoinInfoPtr &info);

			KeyStore ExportKeystore(const std::string &payPasswd) const;

			nlohmann::json ExportReadonlyWallet() const;

			bool ImportReadonlyWallet(const nlohmann::json &walletJSON);

			std::string ExportMnemonic(const std::string &payPasswd) const;

			bool VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const;

			bool VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const;

			bool VerifyPayPassword(const std::string &payPasswd) const;

			void Save();

			void Remove();

			std::string GetDataPath() const;

			void RegenerateKey(const std::string &payPasswd) const;

			uint512 GetSeed(const std::string &payPasswd) const;

			bytes_t GetETHSCPubKey() const;

			bool HasMnemonic() const;

			bool HasPassphrase() const;
		private:
			void Init() const;

			void SupportETHSC(const uint512 &seed, const std::string &payPasswd) const;

		private:
			LocalStorePtr _localstore;
			mutable HDKeychainPtr _xpub;
			mutable int _cosignerIndex;
			mutable HDKeychainPtr _curMultiSigner; // multi sign current wallet signer
			mutable HDKeychainArray _allMultiSigners; // including _multiSigner and sorted
			mutable bytes_t _ownerPubKey, _requestPubKey;
		};

	}
}

#endif //__ELASTOS_SDK_ACCOUNT_H__
