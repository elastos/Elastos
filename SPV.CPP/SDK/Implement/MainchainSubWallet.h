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
#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

#include "SidechainSubWallet.h"
#include <IMainchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class IOutputPayload;
		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

		class MainchainSubWallet : public IMainchainSubWallet, public SubWallet {
		public:
			~MainchainSubWallet();

			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &memo);

		public:
			//////////////////////////////////////////////////
			/*                      Vote                    */
			//////////////////////////////////////////////////
			virtual nlohmann::json CreateVoteProducerTransaction(
				const std::string &fromAddress,
				const std::string &stake,
				const nlohmann::json &publicKeys,
				const std::string &memo,
				const nlohmann::json &invalidCandidates);

			virtual	nlohmann::json GetVotedProducerList() const;

			virtual nlohmann::json CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates);

			virtual	nlohmann::json GetVotedCRList() const;

			virtual nlohmann::json CreateVoteCRCProposalTransaction(const std::string &fromAddress,
																	const nlohmann::json &votes,
																	const std::string &memo,
																	const nlohmann::json &invalidCandidates);

			virtual nlohmann::json CreateImpeachmentCRCTransaction(const std::string &fromAddress,
																   const nlohmann::json &votes,
																   const std::string &memo,
																   const nlohmann::json &invalidCandidates);

			virtual nlohmann::json GetVoteInfo(const std::string &type) const;

		public:
			//////////////////////////////////////////////////
			/*                    Producer                  */
			//////////////////////////////////////////////////
			virtual nlohmann::json GenerateProducerPayload(
				const std::string &ownerPublicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const;

			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &ownerPublicKey,
				const std::string &payPasswd) const;

			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &memo);

			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo);

			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &emmo);

			virtual nlohmann::json CreateRetrieveDepositTransaction(
				const std::string &amount,
				const std::string &memo);

			virtual std::string GetOwnerPublicKey() const;

			virtual std::string GetOwnerAddress() const;

			virtual nlohmann::json GetRegisteredProducerInfo() const;

		public:
			//////////////////////////////////////////////////
			/*                      CRC                     */
			//////////////////////////////////////////////////
			virtual nlohmann::json GenerateCRInfoPayload(
					const std::string &crPublicKey,
					const std::string &did,
					const std::string &nickName,
					const std::string &url,
					uint64_t location) const;

			virtual nlohmann::json GenerateUnregisterCRPayload(const std::string &CID) const;

			virtual nlohmann::json CreateRegisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json CreateUpdateCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo);

			virtual nlohmann::json CreateUnregisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo);

			virtual nlohmann::json CreateRetrieveCRDepositTransaction(
					const std::string &crPublicKey,
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json GetRegisteredCRInfo() const;

		public:
			//////////////////////////////////////////////////
			/*                     Proposal                 */
			//////////////////////////////////////////////////
			virtual nlohmann::json SponsorProposalDigest(uint16_t type,
			                                             const std::string &categoryData,
			                                             const std::string &sponsorPublicKey,
			                                             const std::string &draftHash,
			                                             const nlohmann::json &budgets,
			                                             const std::string &recipient) const;

			virtual nlohmann::json CRSponsorProposalDigest(const nlohmann::json &sponsorSignedProposal,
			                                               const std::string &crSponsorDID,
			                                               const std::string &crOpinionHash) const;

			virtual nlohmann::json CreateCRCProposalTransaction(nlohmann::json crSignedProposal,
			                                                    const std::string &memo);

			virtual nlohmann::json GenerateCRCProposalReview(const std::string &proposalHash,
			                                                 uint8_t voteResult,
			                                                 const std::string &did) const;

			virtual nlohmann::json CreateCRCProposalReviewTransaction(const nlohmann::json &proposalReview,
			                                                          const std::string &memo);

			virtual nlohmann::json LeaderProposalTrackDigest(uint8_t type,
			                                                 const std::string &proposalHash,
			                                                 const std::string &documentHash,
			                                                 uint8_t stage,
			                                                 const std::string &appropriation,
			                                                 const std::string &leaderPubKey,
			                                                 const std::string &newLeaderPubKey) const;

			virtual nlohmann::json
			NewLeaderProposalTrackDigest(const nlohmann::json &leaderSignedProposalTracking) const;

			virtual nlohmann::json
			SecretaryGeneralProposalTrackDigest(const nlohmann::json &leaderSignedProposalTracking) const;

			virtual nlohmann::json
			CreateProposalTrackingTransaction(const nlohmann::json &SecretaryGeneralSignedPayload,
			                                  const std::string &memo);

		private:
			PayloadPtr GenerateCRCProposalPayload(uint16_t type,
			                                      const std::string &categoryData,
			                                      const std::string &sponsorPublicKey,
			                                      const std::string &crSponsorDID,
			                                      const std::string &draftHash,
			                                      const nlohmann::json &budgets,
			                                      const std::string &recipient) const;

			void FilterVoteCandidates(TransactionPtr &tx, const nlohmann::json &invalidCandidates) const;
		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfoPtr &info,
							   const ChainConfigPtr &config,
							   MasterWallet *parent,
							   const std::string &netType);

			TransactionPtr CreateVoteTx(const VoteContent &voteContent, const std::string &memo, bool max,
			                            VoteContentArray &dropedVotes);
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
