// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include "IIDChainSubWallet.h"

namespace Elastos {
	namespace ElaWallet {
		class CredentialSubject;
		class VerifiableCredential;
		class DIDInfo;

		class DIDDetail {
		public:
			DIDDetail();

			~DIDDetail();

			void SetDIDInfo(const PayloadPtr &didInfo);

			const PayloadPtr &GetDIDInfo() const;

			void SetIssuanceTime(time_t issuanceTime);

			time_t GetIssuanceTime() const;

			void SetBlockHeighht(uint32_t blockHeight);

			uint32_t GetBlockHeight() const;

			void SetTxHash(const std::string &txHash);

			const std::string &GetTxHash() const;

			uint32_t GetConfirms(uint32_t walletBlockHeight) const;
		private:
			PayloadPtr _didInfo;
			time_t _issuanceTime;
			uint32_t _blockHeight;
			std::string _txHash;
		};

		typedef boost::shared_ptr<DIDDetail> DIDDetailPtr;

		class IDChainSubWallet : public SidechainSubWallet, public IIDChainSubWallet {
		public:
			virtual ~IDChainSubWallet();

			virtual nlohmann::json CreateIDTransaction(
					const nlohmann::json &payloadJson,
					const std::string &memo = "");

			virtual nlohmann::json GetAllDID(uint32_t start, uint32_t count) const;

			virtual std::string Sign(const std::string &did, const std::string &message, const std::string &payPassword) const;

			virtual std::string SignDigest(const std::string &did, const std::string &digest,
			                               const std::string &payPassword) const;

			virtual bool VerifySignature(const std::string &publicKey, const std::string &message, const std::string &signature);

			virtual std::string GetPublicKeyDID(const std::string &pubkey) const;

			virtual nlohmann::json GenerateDIDInfoPayload(
				const nlohmann::json &didInfo,
				const std::string &paypasswd);

			virtual nlohmann::json GetResolveDIDInfo(uint32_t start, uint32_t count, const std::string &did) const;

		protected:
			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

		private:
			std::vector<std::string> GetVerifiableCredentialTypes(const CredentialSubject &subject) const;

			nlohmann::json ToDIDInfoJson(const DIDDetailPtr &didDetailPtr, bool isDetail) const;

			void InitDIDList();

			void InsertDID(const DIDDetailPtr &didDetailPtr);

			VerifiableCredential GetSelfProclaimedCredential(const std::string &didName) const;

			VerifiableCredential GetPersonalInfoCredential(const nlohmann::json &didInfo) const;
		protected:
			friend class MasterWallet;

			IDChainSubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent);

		private:
			std::vector<DIDDetailPtr> _didList;
		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
