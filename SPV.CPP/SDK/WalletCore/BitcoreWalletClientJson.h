// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
#define __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Elastos {
	namespace ElaWallet {
		class PublicKeyRing {
		public:
			PublicKeyRing() {}

			PublicKeyRing(const std::string &pubkey, const std::string &xpub) :
					_xPubKey(xpub), _requestPubKey(pubkey) {}

			const std::string &GetxPubKey() const { return _xPubKey; }

			const std::string &GetRequestPubKey() const { return _requestPubKey; }

			void SetRequestPubKey(const std::string &pubkey) { _requestPubKey = pubkey; }

			void SetxPubKey(const std::string &xpubkey) { _xPubKey = xpubkey; }

			nlohmann::json ToJson() const;

			void FromJson(const nlohmann::json &j);

		private:
			std::string _xPubKey;
			std::string _requestPubKey;
		};


		class BitcoreWalletClientJson {

		public:
			BitcoreWalletClientJson();

			virtual ~BitcoreWalletClientJson();

		public:
			const std::string &xPrivKey() const { return _xPrivKey; }

			void SetxPrivKey(const std::string &xprv) { _xPrivKey = xprv; }

			const std::string &xPubKey() const { return _xPubKey; }

			void SetxPubKey(const std::string &xpub) { _xPubKey = xpub; }

			const std::string &RequestPrivKey() const { return _requestPrivKey; }

			void SetRequestPrivKey(const std::string &key) { _requestPrivKey = key; }

			const std::string &RequestPubKey() const { return _requestPubKey; }

			void SetRequestPubKey(const std::string &pubkey) { _requestPubKey = pubkey; }

			bool HasPassPhrase() const { return _mnemonicHasPassphrase; }

			void SetHasPassPhrase(bool has) { _mnemonicHasPassphrase = has; }

			const std::vector<PublicKeyRing> &GetPublicKeyRing() const { return _publicKeyRing; }

			void AddPublicKeyRing(const PublicKeyRing &publicKeyRing) { _publicKeyRing.push_back(publicKeyRing); }

			void SetPublicKeyRing(const std::vector<PublicKeyRing> &ring) { _publicKeyRing = ring; }

			int GetM() const { return _m; }

			void SetM(int m) { _m = m; }

			int GetN() const { return _n; }

			void SetN(int n) { _n = n; }

			const std::string &DerivationStrategy() const { return _derivationStrategy; }

			void SetDerivationStrategy(const std::string &strategy) { _derivationStrategy = strategy; }

			int Account() const { return _account; }

			void SetAccount(int account) { _account = account; }

			virtual nlohmann::json ToJson(bool withPrivKey) const;

			virtual void FromJson(const nlohmann::json &j);

		protected:
			std::string _coin;
			std::string _network;
			std::string _xPrivKey;
			std::string _xPubKey;
			std::string _requestPrivKey;
			std::string _requestPubKey;
			std::string _copayerId;
			std::vector<PublicKeyRing> _publicKeyRing;
			std::string _walletId;
			std::string _walletName;
			int _m;
			int _n;
			std::string _walletPrivKey;
			std::string _personalEncryptingKey;
			std::string _sharedEncryptingKey;
			std::string _copayerName;
			std::string _entropySource;
			bool _mnemonicHasPassphrase;
			std::string _derivationStrategy;
			int _account;
			bool _compliantDerivation;
			std::string _addressType;
		};
	}
}

namespace nlohmann {
	template<>
	struct adl_serializer<Elastos::ElaWallet::PublicKeyRing> {
		static void to_json(json &j, const Elastos::ElaWallet::PublicKeyRing &r) {
			j = r.ToJson();
		}

		static void from_json(const json &j, Elastos::ElaWallet::PublicKeyRing &r) {
			r.FromJson(j);
		}
	};
}

#endif //__ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
