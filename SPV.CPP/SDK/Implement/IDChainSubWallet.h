// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include "Interface/IIDChainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {
		class CredentialSubject;
		class DIDInfo;

		class IDChainSubWallet : public SidechainSubWallet, public IIDChainSubWallet {
		public:
			virtual ~IDChainSubWallet();

			virtual nlohmann::json CreateIDTransaction(
					const nlohmann::json &payloadJson,
					const std::string &memo = "");

			virtual nlohmann::json GetAllDID(uint32_t start, uint32_t count) const;

			virtual std::string Sign(const std::string &did, const std::string &message, const std::string &payPassword) const;

			virtual bool VerifySignature(const std::string &publicKey, const std::string &message, const std::string &signature);

			virtual std::string GetDIDPublicKey(const std::string &pubkey) const;

			virtual nlohmann::json GenerateDIDInfoPayload(
				const nlohmann::json &didInfo,
				const std::string &paypasswd);

			virtual nlohmann::json GetDIDInfo(const std::string &did) const;

		private:
			std::vector<std::string> getVerifiableCredentialTypes(const CredentialSubject &subject);

			nlohmann::json toOutJsonInfo(const DIDInfo *didInfo) const;
		protected:
			friend class MasterWallet;

			IDChainSubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent);

		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
