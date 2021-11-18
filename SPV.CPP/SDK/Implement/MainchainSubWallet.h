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
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

#include <IMainchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class IOutputPayload;
		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

		class MainchainSubWallet : public IMainchainSubWallet, public ElastosBaseSubWallet {
		public:
			~MainchainSubWallet();

			nlohmann::json CreateDepositTransaction(
			        uint8_t version,
					const nlohmann::json &inputsJson,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
                    const std::string &lockAddress,
					const std::string &fee,
					const std::string &memo) const override;

		public:
			//////////////////////////////////////////////////
			/*                      Vote                    */
			//////////////////////////////////////////////////
			nlohmann::json CreateVoteTransaction(
				const nlohmann::json &inputsJson,
				const nlohmann::json &voteContentsJson,
				const std::string &fee,
				const std::string &memo) const override;

		public:
            std::string GetDepositAddress(const std::string &pubkey) const override;
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

			nlohmann::json CreateRegisterProducerTransaction(const nlohmann::json &inputsJson,
															 const nlohmann::json &payload,
															 const std::string &amount,
															 const std::string &fee,
															 const std::string &memo) const override;

			nlohmann::json CreateUpdateProducerTransaction(const nlohmann::json &inputsJson,
														   const nlohmann::json &payload,
														   const std::string &fee,
														   const std::string &memo) const override;

			nlohmann::json CreateCancelProducerTransaction(const nlohmann::json &inputsJson,
														   const nlohmann::json &payload,
														   const std::string &fee,
														   const std::string &emmo) const override;

			nlohmann::json CreateRetrieveDepositTransaction(const nlohmann::json &inputsJson,
                                                            const std::string &amount,
                                                            const std::string &fee,
															const std::string &memo) const override;

			std::string GetOwnerPublicKey() const override;

			std::string GetOwnerAddress() const override;

            std::string GetOwnerDepositAddress() const override;

		public:
			//////////////////////////////////////////////////
			/*                      CRC                     */
			//////////////////////////////////////////////////
            std::string GetCRDepositAddress() const override;

			nlohmann::json GenerateCRInfoPayload(const std::string &crPublicKey,
												 const std::string &did,
												 const std::string &nickName,
												 const std::string &url,
												 uint64_t location) const override;

			nlohmann::json GenerateUnregisterCRPayload(const std::string &CID) const override;

			nlohmann::json CreateRegisterCRTransaction(const nlohmann::json &inputsJson,
													   const nlohmann::json &payloadJSON,
													   const std::string &amount,
													   const std::string &fee,
													   const std::string &memo) const override;

			nlohmann::json CreateUpdateCRTransaction(const nlohmann::json &inputsJson,
													 const nlohmann::json &payloadJSON,
													 const std::string &fee,
													 const std::string &memo) const override;

			nlohmann::json CreateUnregisterCRTransaction(const nlohmann::json &inputsJson,
														 const nlohmann::json &payloadJSON,
														 const std::string &fee,
														 const std::string &memo) const override;

			nlohmann::json CreateRetrieveCRDepositTransaction(const nlohmann::json &inputsJson,
															  const std::string &amount,
															  const std::string &fee,
															  const std::string &memo) const override;

			std::string CRCouncilMemberClaimNodeDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateCRCouncilMemberClaimNodeTransaction(const nlohmann::json &inputsJson,
                                                                     const nlohmann::json &payloadJson,
                                                                     const std::string &fee,
                                                                     const std::string &memo = "") const override;

		public:
			//////////////////////////////////////////////////
			/*                     Proposal                 */
			//////////////////////////////////////////////////
			std::string ProposalOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalCRCouncilMemberDigest(const nlohmann::json &payload) const override;

			std::string CalculateProposalHash(const nlohmann::json &payload) const override ;

			nlohmann::json CreateProposalTransaction(
			        const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo) const override;

			//////////////////////////////////////////////////
			/*               Proposal Review                */
			//////////////////////////////////////////////////
			std::string ProposalReviewDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateProposalReviewTransaction(const nlohmann::json &inputsJson,
                                                           const nlohmann::json &payload,
														   const std::string &fee,
														   const std::string &memo) const override;

			//////////////////////////////////////////////////
			/*               Proposal Tracking              */
			//////////////////////////////////////////////////
			std::string ProposalTrackingOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalTrackingNewOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalTrackingSecretaryDigest(const nlohmann::json &payload) const override;

            nlohmann::json CreateProposalTrackingTransaction(const nlohmann::json &inputsJson,
                                                             const nlohmann::json &payload,
															 const std::string &fee,
															 const std::string &memo) const override;

			//////////////////////////////////////////////////
			/*      Proposal Secretary General Election     */
			//////////////////////////////////////////////////
			std::string ProposalSecretaryGeneralElectionDigest(
				const nlohmann::json &payload) const override;

			std::string ProposalSecretaryGeneralElectionCRCouncilMemberDigest(
				const nlohmann::json &payload) const override;

			nlohmann::json CreateSecretaryGeneralElectionTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

			//////////////////////////////////////////////////
			/*             Proposal Change Owner            */
			//////////////////////////////////////////////////
			std::string ProposalChangeOwnerDigest(const nlohmann::json &payload) const override;

			std::string ProposalChangeOwnerCRCouncilMemberDigest(const nlohmann::json &payload) const override ;

			nlohmann::json CreateProposalChangeOwnerTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

			//////////////////////////////////////////////////
			/*           Proposal Terminate Proposal        */
			//////////////////////////////////////////////////
			std::string TerminateProposalOwnerDigest(const nlohmann::json &payload) const override ;

			std::string TerminateProposalCRCouncilMemberDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateTerminateProposalTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

            //////////////////////////////////////////////////
            /*              Reserve Custom ID               */
            //////////////////////////////////////////////////
            std::string ReserveCustomIDOwnerDigest(const nlohmann::json &payload) const override;

            std::string ReserveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const override;

            nlohmann::json CreateReserveCustomIDTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

            //////////////////////////////////////////////////
            /*               Receive Custom ID              */
            //////////////////////////////////////////////////
            std::string ReceiveCustomIDOwnerDigest(const nlohmann::json &payload) const override;

            std::string ReceiveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const override;

            nlohmann::json CreateReceiveCustomIDTransaction(
                    const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

            //////////////////////////////////////////////////
            /*              Change Custom ID Fee            */
            //////////////////////////////////////////////////
            std::string ChangeCustomIDFeeOwnerDigest(const nlohmann::json &payload) const override;

            std::string ChangeCustomIDFeeCRCouncilMemberDigest(const nlohmann::json &payload) const override;

            nlohmann::json CreateChangeCustomIDFeeTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

			//////////////////////////////////////////////////
			/*               Proposal Withdraw              */
			//////////////////////////////////////////////////
			std::string ProposalWithdrawDigest(const nlohmann::json &payload) const override;

			nlohmann::json CreateProposalWithdrawTransaction(
			        const nlohmann::json &inputsJson,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo) const override;

            //////////////////////////////////////////////////
            /*               Proposal Register side-chain   */
            //////////////////////////////////////////////////
            std::string RegisterSidechainOwnerDigest(const nlohmann::json &payload) const override;

            std::string RegisterSidechainCRCouncilMemberDigest(const nlohmann::json &payload) const override;

            nlohmann::json CreateRegisterSidechainTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const override;

		private:
			bool VoteContentFromJson(VoteContentArray &voteContents, BigInt &maxAmount, const nlohmann::json &j) const;

			bool VoteAmountFromJson(BigInt &voteAmount, const nlohmann::json &j) const;

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfoPtr &info,
							   const ChainConfigPtr &config,
							   MasterWallet *parent,
							   const std::string &netType);
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
