// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IACCOUNT_H__
#define __ELASTOS_SDK_IACCOUNT_H__

#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>

class uchar_vector;
typedef uchar_vector bytes_t;

namespace Elastos {
	namespace ElaWallet {

		class Key;
		class HDKeychain;
		typedef boost::shared_ptr<HDKeychain> HDKeychainPtr;
		typedef std::vector<HDKeychainPtr> HDKeychainArray;
		class PublicKeyRing;
		class CoinInfo;
		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;
		class KeyStore;

		class IAccount {
		public:
			enum SignType {
				Standard,
				MultiSign,
			};

		public:
			virtual ~IAccount() {}

			virtual bytes_t RequestPubKey() const = 0;

			virtual Key RequestPrivKey(const std::string &payPassword) const = 0;

			virtual HDKeychainPtr RootKey(const std::string &payPassword) const = 0;

			virtual HDKeychainPtr MasterPubKey() const = 0;

			virtual std::string GetxPrvKeyString(const std::string &payPasswd) const = 0;

			virtual std::string MasterPubKeyString() const = 0;

			virtual std::string MasterPubKeyHDPMString() const = 0;

			virtual std::vector<PublicKeyRing> MasterPubKeyRing() const = 0;

			virtual bytes_t OwnerPubKey() const = 0;

			virtual void ChangePassword(const std::string &oldPassword, const std::string &newPassword) = 0;

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual SignType GetSignType() const = 0;

			virtual bool Readonly() const = 0;

			virtual bool SingleAddress() const = 0;

			virtual bool Equal(const boost::shared_ptr<IAccount> &account) const = 0;

			virtual int GetM() const = 0;

			virtual int GetN() const = 0;

			virtual std::string DerivationStrategy() const = 0;

			virtual nlohmann::json GetPubKeyInfo() const = 0;

			virtual HDKeychainPtr MultiSignSigner() const = 0;

			virtual HDKeychainArray MultiSignCosigner() const = 0;

			virtual int CosignerIndex() const = 0;

			virtual std::vector<CoinInfoPtr> SubWalletInfoList() const = 0;

			virtual void AddSubWalletInfoList(const CoinInfoPtr &info) = 0;

			virtual void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &info) = 0;

			virtual void RemoveSubWalletInfo(const CoinInfoPtr &info) = 0;

			virtual KeyStore ExportKeystore(const std::string &payPasswd) const = 0;

			virtual nlohmann::json ExportReadonlyWallet() const = 0;

			virtual bool ImportReadonlyWallet(const nlohmann::json &walletJSON) = 0;

			virtual std::string ExportMnemonic(const std::string &payPasswd) const = 0;

			virtual bool VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const = 0;

			virtual bool VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const = 0;

			virtual bool VerifyPayPassword(const std::string &payPasswd) const = 0;

			virtual void Save() = 0;

			virtual void Remove() = 0;

			virtual std::string GetDataPath() const = 0;

			virtual void RegenerateKey(const std::string &payPasswd) const = 0;
		};

		typedef boost::shared_ptr<IAccount> AccountPtr;

	}
}

#endif // __ELASTOS_SDK_IACCOUNT_H__
