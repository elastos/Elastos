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

			nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &memo) override;

		public:
			//////////////////////////////////////////////////
			/*                      Vote                    */
			//////////////////////////////////////////////////
			nlohmann::json CreateVoteProducerTransaction(
				const std::string &fromAddress,
				const std::string &stake,
				const nlohmann::json &publicKeys,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) override;

			nlohmann::json GetVotedProducerList() const override;

			nlohmann::json CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) override;

			nlohmann::json GetVotedCRList() const override;

			nlohmann::json CreateVoteCRCProposalTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) override;

			nlohmann::json CreateImpeachmentCRCTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) override;

			nlohmann::json GetVoteInfo(const std::string &type) const override;

		public:
			//////////////////////////////////////////////////
			/*                    Producer                  */
			//////////////////////////////////////////////////
			nlohmann::json GenerateProducerPayload(const std::string &ownerPublicKey,
												   const std::string &nodePublicKey,
												   const std::string &nickName,
												   const std::string &url,
												   const std::string &ipAddress,
												   uint64_t location,
												   const std::string &payPasswd) const override;

			nlohmann::json GenerateCancelProducerPayload(const std::string &ownerPublicKey,
														 const std::string &payPasswd) const override;

			nlohmann::json CreateRegisterProducerTransaction(const std::string &fromAddress,
															 const nlohmann::json &payload,
															 const std::string &amount,
															 const std::string &memo) override;

			nlohmann::json CreateUpdateProducerTransaction(const std::string &fromAddress,
														   const nlohmann::json &payload,
														   const std::string &memo) override;

			nlohmann::json CreateCancelProducerTransaction(const std::string &fromAddress,
														   const nlohmann::json &payload,
														   const std::string &emmo) override;

			nlohmann::json CreateRetrieveDepositTransaction(const std::string &amount,
															const std::string &memo) override;

			std::string GetOwnerPublicKey() const override;

			std::string GetOwnerAddress() const override;

			nlohmann::json GetRegisteredProducerInfo() const override;

		public:
			//////////////////////////////////////////////////
			/*                      CRC                     */
			//////////////////////////////////////////////////
			nlohmann::json GenerateCRInfoPayload(const std::string &crPublicKey,
												 const std::string &did,
												 const std::string &nickName,
												 const std::string &url,
												 uint64_t location) const override;

			nlohmann::json GenerateUnregisterCRPayload(const std::string &CID) const override;

			nlohmann::json CreateRegisterCRTransaction(const std::string &fromAddress,
													   const nlohmann::json &payloadJSON,
													   const std::string &amount,
													   const std::string &memo) override;

			nlohmann::json CreateUpdateCRTransaction(const std::string &fromAddress,
													 const nlohmann::json &payloadJSON,
													 const std::string &memo) override;

			nlohmann::json CreateUnregisterCRTransaction(const std::string &fromAddress,
														 const nlohmann::json &payloadJSON,
														 const std::string &memo) override;

			nlohmann::json CreateRetrieveCRDepositTransaction(const std::string &crPublicKey,
															  const std::string &amount,
															  const std::string &memo) override;

			nlohmann::json GetRegisteredCRInfo() const override;

		public:
			//////////////////////////////////////////////////
			/*                     Proposal                 */
			//////////////////////////////////////////////////
			std::string ProposalOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalCRCouncilMemberDigest(const nlohmann::json &payload) const override;

			std::string CalculateProposalHash(const nlohmann::json &payload) const override ;

			nlohmann::json CreateProposalTransaction(const nlohmann::json &payload,
													 const std::string &memo) override;

			//////////////////////////////////////////////////
			/*               Proposal Review                */
			//////////////////////////////////////////////////
			std::string ProposalReviewDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateProposalReviewTransaction(const nlohmann::json &payload,
														   const std::string &memo) override;

			//////////////////////////////////////////////////
			/*               Proposal Tracking              */
			//////////////////////////////////////////////////
			std::string ProposalTrackingOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalTrackingNewOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalTrackingSecretaryDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateProposalTrackingTransaction(const nlohmann::json &payload,
															 const std::string &memo) override;

			//////////////////////////////////////////////////
			/*               Proposal Withdraw              */
			//////////////////////////////////////////////////
			std::string ProposalWithdrawDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateProposalWithdrawTransaction(const std::string &recipient,
															 const std::string &amount,
															 const nlohmann::json &utxo,
															 const nlohmann::json &payload,
															 const std::string &memo) override;

		private:
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
