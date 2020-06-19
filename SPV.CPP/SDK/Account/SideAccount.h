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
#include <Plugin/Transaction/Transaction.h>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>
#include <WalletCore/BitcoreWalletClientJson.h>
#include <WalletCore/CoinInfo.h>
#include <WalletCore/KeyStore.h>

#include "IAccount.h"
#include "ISubAccount.h"

#ifndef __ELASTOS_SDK_SIDEACCOUNT_H__
#define __ELASTOS_SDK_SIDEACCOUNT_H__

namespace Elastos {
	namespace ElaWallet {
		class FakeParent : public IAccount {
		public:
			bytes_t RequestPubKey() const { return bytes_t(0); }

			HDKeychainPtr RootKey(const std::string &) const { return nullptr; }

			Key RequestPrivKey(const std::string &) const { return Key(); }

			HDKeychainPtr MasterPubKey() const { return nullptr; }

			std::string GetxPrvKeyString(const std::string &) const { return ""; }

			std::string MasterPubKeyString() const { return ""; }

			std::string MasterPubKeyHDPMString() const { return ""; }

			std::vector<PublicKeyRing> MasterPubKeyRing() const { return _empty_pub_keys; }

			bytes_t OwnerPubKey() const { return bytes_t(); }

			void ChangePassword(const std::string &, const std::string &) {}

			nlohmann::json GetBasicInfo() const {
				nlohmann::json j;
				j["Type"] = "MultiSign";
				j["Readonly"] = true;
				j["SingleAddress"] = true;
				j["M"] = 0;
				j["N"] = 0;
				j["HasPassPhrase"] = false;
				return j;
			}

			SignType GetSignType() const { return MultiSign; }

			bool Readonly() const { return true; }

			bool SingleAddress() const { return true; }

			bool Equal(const boost::shared_ptr<IAccount> &account) const { return false; }

			int GetM() const { return 0; }

			int GetN() const { return 0; }

			std::string DerivationStrategy() const { return ""; }

			nlohmann::json GetPubKeyInfo() const { return nlohmann::json(); }

			HDKeychainPtr MultiSignSigner() const { return nullptr; }

			HDKeychainArray MultiSignCosigner() const { return HDKeychainArray(); }

			int CosignerIndex() const { return 0; }

			std::vector<CoinInfoPtr> SubWalletInfoList() const { return _empty_coin_info; }

			void AddSubWalletInfoList(const CoinInfoPtr &) {}

			void SetSubWalletInfoList(const std::vector<CoinInfoPtr> &) {}

			void RemoveSubWalletInfo(const CoinInfoPtr &) {}

			KeyStore ExportKeystore(const std::string &) const { return KeyStore(); }

			nlohmann::json ExportReadonlyWallet() const { return nlohmann::json(); }

			bool ImportReadonlyWallet(const nlohmann::json &walletJSON) { return true; }

			std::string ExportMnemonic(const std::string &) const { return ""; }

			void RegenerateKey(const std::string &) const {}

			uint512 GetSeed(const std::string &payPasswd) const { return uint512(); }

			bytes_t GetETHSCPubKey() const { return bytes_t(); }

			bool HasMnemonic() const { return false; }

			bool HasPassphrase() const { return false; }

			bool VerifyPrivateKey(const std::string &, const std::string &) const { return false; }

			bool VerifyPassPhrase(const std::string &, const std::string &) const { return false; }

			bool VerifyPayPassword(const std::string &) const { return false; }

			void Save() {}

			void Remove() {}

			std::string GetDataPath() const { return ""; }

		private:
			std::vector<PublicKeyRing> _empty_pub_keys;
			std::vector<CoinInfoPtr> _empty_coin_info;
		};

		class SideAccount : public ISubAccount {
		public:
			SideAccount(const uint256 &genesis_hash);

			~SideAccount();

			nlohmann::json GetBasicInfo() const;

			void Init();

			void InitCID();

			bool IsSingleAddress() const;

			bool IsProducerDepositAddress(const AddressPtr &address) const;

			bool IsOwnerAddress(const AddressPtr &address) const;

			bool IsCRDepositAddress(const AddressPtr &address) const;

			void SetUsedAddresses(const AddressSet &addresses);

			bool AddUsedAddress(const AddressPtr &address);

			size_t GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const;

			size_t GetAllCID(AddressArray &did, uint32_t start, size_t count) const;

			size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
			                        bool containInternal) const;

			AddressArray UnusedAddresses(uint32_t gapLimit, bool internal);

			bool ContainsAddress(const AddressPtr &address) const;

			bytes_t OwnerPubKey() const;

			bytes_t DIDPubKey() const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) const;

			Key GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const;

			Key DeriveOwnerKey(const std::string &payPasswd);

			Key DeriveDIDKey(const std::string &payPasswd);

			bool GetCodeAndPath(const AddressPtr &addr, bytes_t &code, std::string &path) const;

			size_t InternalChainIndex(const TransactionPtr &tx) const;

			size_t ExternalChainIndex(const TransactionPtr &tx) const;

			AccountPtr Parent() const;

		private:
			// The 'X' address generated by the side chain genesis hash.
			AddressPtr side_address;
			AccountPtr parent;
		};

	} // namespace ElaWallet
} // namespace Elastos
#endif // __ELASTOS_SDK_SIDEACCOUNT_H__
