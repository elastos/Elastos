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

#include "ISubWallet.h"

#ifndef __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class IMainchainSubWallet : public virtual IElastosBaseSubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~IMainchainSubWallet() noexcept {}

			/**
			 * Deposit token from the main chain to side chains, such as ID chain or token chain, etc
			 *
			 * @version 0x00 means old deposit tx, 0x01 means new deposit tx, other value will throw exception.
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * NOTE:  (utxo input amount) >= amount + 10000 sela + fee
			 * @param sideChainID Chain id of the side chain
			 * @param amount The amount that will be deposit to the side chain.
			 * @param sideChainAddress Receive address of side chain
			 * @param lockAddress Generate from genesis block hash
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string
			 * @return The transaction in JSON format to be signed and published
			 */
			virtual nlohmann::json CreateDepositTransaction(
			        uint8_t version,
					const nlohmann::json &inputs,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &lockAddress,
					const std::string &fee,
					const std::string &memo) const = 0;

		public:
		    virtual std::string GetDepositAddress(const std::string &pubkey) const = 0;
			//////////////////////////////////////////////////
			/*                      Vote                    */
			//////////////////////////////////////////////////
			/**
			 * Create vote transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param voteContents Including all kinds of vote. eg
			 *
			 * [
			 *   {
			 *     "Type":"CRC",
			 *     "Candidates":
			 *     {
			 *       "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "100000000",
			 *       ...
			 *     }
			 *   },
			 *   {
			 *     "Type":"CRCProposal",
			 *     "Candidates":
			 *     {
			 *       "109780cf45c7a6178ad674ac647545b47b10c2c3e3b0020266d0707e5ca8af7c": "100000000",
			 *       ...
			 *     }
			 *   },
			 *   {
			 *     "Type": "CRCImpeachment",
			 *     "Candidates":
			 *     {
			 *       "innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs": "100000000",
			 *       ...
			 *     }
			 *   },
			 *   {
			 *     "Type":"Delegate",
			 *     "Candidates":
			 *     {
			 *       "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4": "100000000",
			 *       ...
			 *     }
			 *   }
			 * ]
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.

			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateVoteTransaction(
				const nlohmann::json &inputs,
				const nlohmann::json &voteContents,
				const std::string &fee,
				const std::string &memo) const = 0;

		public:
			//////////////////////////////////////////////////
			/*                    Producer                  */
			//////////////////////////////////////////////////
			/**
			 * Generate payload for registering or updating producer.
			 *
			 * @param ownerPublicKey The public key to identify a producer. Can't change later. The producer reward will
			 *                       be sent to address of this public key.
			 * @param nodePublicKey  The public key to identify a node. Can be update
			 *                       by CreateUpdateProducerTransaction().
			 * @param nickName       Nickname of producer.
			 * @param url            URL of producer.
			 * @param ipAddress      IP address of node. This argument is deprecated.
			 * @param location       Location code.
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateProducerPayload(
				const std::string &publicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const = 0;

			/**
			 * Generate payaload for unregistering producer.
			 *
			 * @param ownerPublicKey The public key to identify a producer.
			 * @param payPasswd Pay password is using for signing the payload with the owner private key.
			 *
			 * @return The payload in JSON format.
			 */
			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &publicKey,
				const std::string &payPasswd) const = 0;

			/**
			 * Create register producer transaction
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Generate by GenerateProducerPayload()
			 * @param amount Amount must lager than 500,000,000,000 sela
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string
			 * @return The transaction in JSON format to be signed and published
			 */
			virtual nlohmann::json CreateRegisterProducerTransaction(
				const nlohmann::json &inputs,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &fee,
				const std::string &memo) const = 0;

			/**
			 * Create update producer transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Generate by GenerateProducerPayload().
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 *
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateProducerTransaction(
				const nlohmann::json &inputs,
				const nlohmann::json &payload,
				const std::string &fee,
				const std::string &memo) const = 0;

			/**
			 * Create cancel producer transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Generate by GenerateCancelProducerPayload().
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCancelProducerTransaction(
				const nlohmann::json &inputs,
				const nlohmann::json &payload,
				const std::string &fee,
				const std::string &memo) const = 0;

			/**
			 * Create retrieve deposit transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param amount Retrieve amount including fee
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 *
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveDepositTransaction(
                    const nlohmann::json &inputs,
                    const std::string &amount,
                    const std::string &fee,
                    const std::string &memo) const = 0;

			/**
			 * Get owner public key.
			 *
			 * @return Owner public key.
			 */
			virtual std::string GetOwnerPublicKey() const = 0;

			/**
			 * Get address of owner public key
			 * @return Address of owner public key
			 */
			virtual std::string GetOwnerAddress() const = 0;

			/**
			 * Get deposit address of owner.
			 */
			 virtual std::string GetOwnerDepositAddress() const = 0;

		public:
			//////////////////////////////////////////////////
			/*                      CRC                     */
			//////////////////////////////////////////////////

			/**
			 * Get CR deposit
			 * @return
			 */
			virtual std::string GetCRDepositAddress() const = 0;

			/**
			 * Generate cr info payload digest for signature.
			 *
			 * @param crPublicKey    The public key to identify a cr. Can't change later.
			 * @param did            DID to be bonded
			 * @param nickName       Nickname of cr.
			 * @param url            URL of cr.
			 * @param location       Location code.
			 *
			 * @return               The payload in JSON format contains the "Digest" field to be signed and then set the "Signature" field. Such as
			 * {
			 * 	"Code":"210370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280ac",
			 * 	"CID":"iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP",
			 * 	"DID":"icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
			 * 	"Location":86,
			 * 	"NickName":"test",
			 * 	"Url":"test.com",
			 * 	"Digest":"9970b0612f9146f3f5744f7a843dfa6aac3534a6f44232e08469b212323be573",
			 * 	"Signature":""
			 * 	}
			 */
			virtual nlohmann::json GenerateCRInfoPayload(
					const std::string &crPublicKey,
					const std::string &did,
					const std::string &nickName,
					const std::string &url,
					uint64_t location) const = 0;

			/**
			 * Generate unregister cr payload digest for signature.
			 *
			 * @param CID          The id of cr will unregister
			 * @return               The payload in JSON format contains the "Digest" field to be signed and then set the "Signature" field. Such as
			 * {
			 * 	"CID":"iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP",
			 * 	"Digest":"8e17a8bcacc5d70b5b312fccefc19d25d88ac6450322a846132e859509b88001",
			 * 	"Signature":""
			 * 	}
			 */
			virtual nlohmann::json GenerateUnregisterCRPayload(const std::string &CID) const = 0;

			/**
			 * Create register cr transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payloadJSON Generate by GenerateCRInfoPayload().
			 * @param amount Amount must lager than 500,000,000,000 sela
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterCRTransaction(
					const nlohmann::json &inputs,
					const nlohmann::json &payloadJSON,
					const std::string &amount,
					const std::string &fee,
					const std::string &memo) const = 0;

			/**
			 * Create update cr transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payloadJSON Generate by GenerateCRInfoPayload().
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateCRTransaction(
					const nlohmann::json &inputs,
					const nlohmann::json &payloadJSON,
					const std::string &fee,
					const std::string &memo) const = 0;

			/**
			 * Create unregister cr transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payloadJSON Generate by GenerateUnregisterCRPayload().
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUnregisterCRTransaction(
					const nlohmann::json &inputs,
					const nlohmann::json &payloadJSON,
					const std::string &fee,
					const std::string &memo) const = 0;

			/**
			 * Create retrieve deposit cr transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param amount Retrieve amount including fee
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveCRDepositTransaction(
					const nlohmann::json &inputs,
					const std::string &amount,
					const std::string &fee,
					const std::string &memo) const = 0;

			/**
			 * Generate digest for signature of CR council members
			 * @param payload
			 * {
			 *   "NodePublicKey": "...",
			 *   "CRCouncilMemberDID": "...",
			 * }
			 * @return
			 */
			virtual std::string CRCouncilMemberClaimNodeDigest(const nlohmann::json &payload) const = 0;

			/**
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload
			 * {
			 *   "NodePublicKey": "...",
			 *   "CRCouncilMemberDID": "...",
			 *   "CRCouncilMemberSignature": "..."
			 * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return
			 */
			virtual nlohmann::json CreateCRCouncilMemberClaimNodeTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;


		public:
			//////////////////////////////////////////////////
			/*                     Proposal                 */
			//////////////////////////////////////////////////
			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal payload. Must contain the following:
			 * {
			 *    "Type": 0,
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "Budgets": [{"Type":0,"Stage":0,"Amount":"300"},{"Type":1,"Stage":1,"Amount":"33"},{"Type":2,"Stage":2,"Amount":"344"}],
			 *    "Recipient": "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", // address
			 * }
			 *
			 * Type can be value as below:
			 * {
			 *	 Normal: 0x0000
			 *	 ELIP: 0x0100
			 * }
			 *
			 * Budget must contain the following:
			 * {
			 *   "Type": 0,             // imprest = 0, normalPayment = 1, finalPayment = 2
			 *   "Stage": 0,            // value can be [0, 128)
			 *   "Amount": "100000000"  // sela
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalOwnerDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal payload. Must contain the following:
			 * {
			 *    "Type": 0,                   // same as mention on method ProposalOwnerDigest()
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4", // Owner DID public key
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "Budgets": [                 // same as mention on method ProposalOwnerDigest()
			 *      {"Type":0,"Stage":0,"Amount":"300"},{"Type":1,"Stage":1,"Amount":"33"},{"Type":2,"Stage":2,"Amount":"344"}
			 *    ],
			 *    "Recipient": "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", // address
			 *
			 *    // signature of owner
			 *    "Signature": "ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76",
			 *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY"
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Calculate proposal hash.
			 *
			 * @param payload Proposal payload signed by owner and CR committee. Same as payload of CreateProposalTransaction()
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual std::string CalculateProposalHash(const nlohmann::json &payload) const = 0;

			/**
			 * Create proposal transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Proposal payload signed by owner and CR committee.
			 * {
			 *    "Type": 0,                   // same as mention on method ProposalOwnerDigest()
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4", // Owner DID public key
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "Budgets": [                 // same as mention on method ProposalOwnerDigest()
			 *      {"Type":0,"Stage":0,"Amount":"300"},{"Type":1,"Stage":1,"Amount":"33"},{"Type":2,"Stage":2,"Amount":"344"}
			 *    ],
			 *    "Recipient": "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", // address
			 *
			 *    // signature of owner
			 *    "Signature": "ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76",
			 *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
			 *    "CRCouncilMemberSignature": "ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76"
			 * }
			 *
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string
			 * @return The transaction in JSON format to be signed and published
			 */
			virtual nlohmann::json CreateProposalTransaction(
			        const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;


			//////////////////////////////////////////////////
			/*               Proposal Review                */
			//////////////////////////////////////////////////
			/**
			 * Generate digest of payload.
			 *
			 * @param payload Payload proposal review.
			 * {
			 *   "ProposalHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "VoteResult": 1,    // approve = 0, reject = 1, abstain = 2
			 *   "OpinionHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "OpinionData": "", // Optional, string format, limit 1 Mbytes
			 *   "DID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY", // did of CR council member
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalReviewDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Create proposal review transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Signed payload.
 			 * {
			 *   "ProposalHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "VoteResult": 1,    // approve = 0, reject = 1, abstain = 2
			 *   "OpinionHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "OpinionData": "", // Optional, string format, limit 1 Mbytes
			 *   "DID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY", // did of CR council member's did
			 *   // signature of CR council member
			 *   "Signature": "ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76"
			 * }
			 *
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateProposalReviewTransaction(const nlohmann::json &inputs,
                                                                   const nlohmann::json &payload,
																   const std::string &fee,
																   const std::string &memo = "") const = 0;


			//////////////////////////////////////////////////
			/*               Proposal Tracking              */
			//////////////////////////////////////////////////
			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal tracking payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "MessageHash": "0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4",
			 *   "MessageData": "", // Optional, string format, limit 800 Kbytes
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty, eg: "NewOwnerPublicKey":"". Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 * }
			 *
			 * @return Digest of payload
			 */
			virtual std::string ProposalTrackingOwnerDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal tracking payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "MessageHash": "0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4",
			 *   "MessageData": "", // Optional, string format, limit 800 Kbytes
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty, eg: "NewOwnerPublicKey":"". Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "OwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalTrackingNewOwnerDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal tracking payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "MessageHash": "0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4",
			 *   "MessageData": "", // Optional, string format, limit 800 Kbytes
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty, eg: "NewOwnerPublicKey":"". Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "OwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   // If NewOwnerPubKey is empty, this must be empty. eg: "NewOwnerSignature":""
			 *   "NewOwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   "Type": 0, // common = 0, progress = 1, rejected = 2, terminated = 3, changeOwner = 4, finalized = 5
			 *   "SecretaryGeneralOpinionHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "SecretaryGeneralOpinionData": "", // Optional, string format, limit 200 Kbytes
			 * }
			 *
			 * @return Digest of payload
			 */
			virtual std::string ProposalTrackingSecretaryDigest(const nlohmann::json &payload) const = 0;


			/**
			 * Create proposal tracking transaction.
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Proposal tracking payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "MessageHash": "0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4",
			 *   "MessageData": "", // Optional, string format, limit 800 Kbytes
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty, eg: "NewOwnerPublicKey":"". Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "OwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   // If NewOwnerPubKey is empty, this must be empty. eg: "NewOwnerSignature":""
			 *   "NewOwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   "Type": 0, // common = 0, progress = 1, rejected = 2, terminated = 3, changeOwner = 4, finalized = 5
			 *   "SecretaryGeneralOpinionHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "SecretaryGeneralOpinionData": "", // Optional, string format, limit 200 Kbytes
			 *   "SecretaryGeneralSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109"
			 * }
			 *
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
            virtual nlohmann::json CreateProposalTrackingTransaction(const nlohmann::json &inputs,
                                                                     const nlohmann::json &payload,
                                                                     const std::string &fee,
                                                                     const std::string &memo = "") const = 0;

			//////////////////////////////////////////////////
			/*      Proposal Secretary General Election     */
			//////////////////////////////////////////////////
			/**
			 * @param payload Proposal secretary election payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "SecretaryGeneralPublicKey": "...",
			 *    "SecretaryGeneralDID": "...",
			 * }
			 * @return
			 */
			virtual std::string ProposalSecretaryGeneralElectionDigest(
				const nlohmann::json &payload) const = 0;

			/**
			 * @param payload Proposal secretary election payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "SecretaryGeneralPublicKey": "...",
			 *    "SecretaryGeneralDID": "...",
			 *    "Signature": "...",
			 *    "SecretaryGeneralSignature": "...",
			 *    "CRCouncilMemberDID": "...",
			 * }
			 * @return
			 */
			virtual std::string ProposalSecretaryGeneralElectionCRCouncilMemberDigest(
				const nlohmann::json &payload) const = 0;

			/**
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Proposal secretary election payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "SecretaryGeneralPublicKey": "...",
			 *    "SecretaryGeneralDID": "...",
			 *    "Signature": "...",
			 *    "SecretaryGeneralSignature": "...",
			 *    "CRCouncilMemberDID": "...",
			 *    "CRCouncilMemberSignature": "..."
			 * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remarks string
			 * @return
			 */
			virtual nlohmann::json CreateSecretaryGeneralElectionTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

			//////////////////////////////////////////////////
			/*             Proposal Change Owner            */
			//////////////////////////////////////////////////
			/**
			 * Use for owner & new owner sign
			 * @param payload Proposal change owner payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 *    "NewRecipient": "...",
			 *    "NewOwnerPublicKey": "...",
			 * }
			 * @return
			 */
			virtual std::string ProposalChangeOwnerDigest(const nlohmann::json &payload) const = 0;

			/**
			 * @param payload Proposal change owner payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 *    "NewRecipient": "...",
			 *    "NewOwnerPublicKey": "...",
			 *    "Signature": "...",
			 *    "NewOwnerSignature": "...",
			 *    "CRCouncilMemberDID": "..."
			 * }
			 * @return
			 */
			virtual std::string ProposalChangeOwnerCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

			/**
			 *
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Proposal change owner payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 *    "NewRecipient": "...",
			 *    "NewOwnerPublicKey": "...",
			 *    "Signature": "...",
			 *    "NewOwnerSignature": "...",
			 *    "CRCouncilMemberDID": "...",
			 *    "CRCouncilMemberSignature": "...",
			 * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remark string.
			 * @return
			 */
			virtual nlohmann::json CreateProposalChangeOwnerTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

			//////////////////////////////////////////////////
			/*           Proposal Terminate Proposal        */
			//////////////////////////////////////////////////
			/**
			 * @param payload Terminate proposal payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 * }
			 * @return
			 */
			virtual std::string TerminateProposalOwnerDigest(const nlohmann::json &payload) const = 0;

			/**
			 * @param payload Terminate proposal payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 *    "Signature": "...",
			 *    "CRCouncilMemberDID": "...",
			 * }
			 * @return
			 */
			virtual std::string TerminateProposalCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

			/**
			 * @param inputs UTXO which will be used. eg
			 * [
			 *   {
			 *     "TxHash": "...", // string
			 *     "Index": 123, // int
			 *     "Address": "...", // string
			 *     "Amount": "100000000" // bigint string in SELA
			 *   },
			 *   ...
			 * ]
			 * @param payload Terminate proposal payload
			 * {
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "...",
			 *    "DraftHash": "...",
			 *    "DraftData": "", // Optional, string format, limit 1 Mbytes
			 *    "TargetProposalHash": "...",
			 *    "Signature": "...",
			 *    "CRCouncilMemberDID": "...",
			 *    "CRCouncilMemberSignature": "...",
			 * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remark string
			 * @return
			 */
			virtual nlohmann::json CreateTerminateProposalTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

            //////////////////////////////////////////////////
            /*              Reserve Custom ID               */
            //////////////////////////////////////////////////
            /**
             * @param payload Reserve Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReservedCustomIDList": ["...", "...", ...],
             * }
             * @return
             */
            virtual std::string ReserveCustomIDOwnerDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param payload Reserve Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReservedCustomIDList": ["...", "...", ...],
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             * }
             * @return
             */
            virtual std::string ReserveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @param payload Reserve Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReservedCustomIDList": ["...", "...", ...],
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             *    "CRCouncilMemberSignature": "...",
             * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remark string
			 * @return
             */
            virtual nlohmann::json CreateReserveCustomIDTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

            //////////////////////////////////////////////////
            /*               Receive Custom ID              */
            //////////////////////////////////////////////////
            /**
             * @param payload Receive Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReceivedCustomIDList": ["...", "...", ...],
             *    "ReceiverDID": "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"
             * }
             * @return
             */
            virtual std::string ReceiveCustomIDOwnerDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param payload Receive Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReceivedCustomIDList": ["...", "...", ...],
             *    "ReceiverDID": "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             * }
             * @return
             */
            virtual std::string ReceiveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @param payload Receive Custom ID payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "ReceivedCustomIDList": ["...", "...", ...],
             *    "ReceiverDID": "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP"
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             *    "CRCouncilMemberSignature": "...",
             * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remark string
			 * @return
             */
            virtual nlohmann::json CreateReceiveCustomIDTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

            //////////////////////////////////////////////////
            /*              Change Custom ID Fee            */
            //////////////////////////////////////////////////
            /**
             * @param payload Change custom ID fee payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "CustomIDFeeRateInfo": {
             *      "RateOfCustomIDFee": 10000,
             *      "EIDEffectiveHeight": 10000
             *    }
             * }
             * @return
             */
            virtual std::string ChangeCustomIDFeeOwnerDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param payload Change custom ID fee payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "CustomIDFeeRateInfo": {
             *      "RateOfCustomIDFee": 10000,
             *      "EIDEffectiveHeight": 10000
             *    },
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             * }
             * @return
             */
            virtual std::string ChangeCustomIDFeeCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

            /**
             * @param inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @param payload Change custom ID fee payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "CustomIDFeeRateInfo": {
             *      "RateOfCustomIDFee": 10000,
             *      "EIDEffectiveHeight": 10000
             *    },
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             *    "CRCouncilMemberSignature": "...",
             * }
			 * @param fee Fee amount. Bigint string in SELA
			 * @param memo Remark string
			 * @return
             */
            virtual nlohmann::json CreateChangeCustomIDFeeTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;

			//////////////////////////////////////////////////
			/*               Proposal Withdraw              */
			//////////////////////////////////////////////////
			/**
			 * Generate digest of payload.
			 *
			 * @param payload Proposal payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "Recipient": "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", // address
			 *   "Amount": "100000000", // 1 ela = 100000000 sela
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalWithdrawDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Create proposal withdraw transaction.
             * @inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
			 * @payload Proposal payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "Recipient": "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv", // address
			 *   "Amount": "100000000", // 1 ela = 100000000 sela
			 *   "Signature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109"
			 * }
			 * @fee Fee amount. Bigint string in SELA
			 * @memo Remarks string. Can be empty string.
			 *
			 * @return Transaction in JSON format.
			 */
			 virtual nlohmann::json CreateProposalWithdrawTransaction(
			         const nlohmann::json &inputs,
                     const nlohmann::json &payload,
                     const std::string &fee,
                     const std::string &memo = "") const = 0;

            //////////////////////////////////////////////////
            /*               Proposal Register side-chain   */
            //////////////////////////////////////////////////
            /**
             * @payload Change custom ID fee payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "SidechainInfo": {
             *      "SideChainName": "...",
             *      "MagicNumber": 0, // uint32_t
             *      "GenesisHash": "...", // hexstring of uint256
             *      "ExchangeRate": 1, // uint64_t
             *      "EffectiveHeight": 1000, // uint32_t
             *      "ResourcePath": "..." // path string
             *    }
             * }
             * @return
             */
            virtual std::string RegisterSidechainOwnerDigest(const nlohmann::json &payload) const = 0;

            /**
             * @payload Change custom ID fee payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "SidechainInfo": {
             *      "SideChainName": "...",
             *      "MagicNumber": 0, // uint32_t
             *      "GenesisHash": "...", // hexstring of uint256
             *      "ExchangeRate": 1, // uint64_t
             *      "EffectiveHeight": 1000, // uint32_t
             *      "ResourcePath": "..." // path string
             *    }
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             * }
             * @return
             */
            virtual std::string RegisterSidechainCRCouncilMemberDigest(const nlohmann::json &payload) const = 0;

            /**
             * @inputs UTXO which will be used. eg
             * [
             *   {
             *     "TxHash": "...", // string
             *     "Index": 123, // int
             *     "Address": "...", // string
             *     "Amount": "100000000" // bigint string in SELA
             *   },
             *   ...
             * ]
             * @payload Register side-chain payload
             * {
             *    "CategoryData": "testdata",  // limit: 4096 bytes
             *    "OwnerPublicKey": "...",
             *    "DraftHash": "...",
             *    "DraftData": "", // Optional, string format, limit 1 Mbytes
             *    "SidechainInfo": {
             *      "SideChainName": "...",
             *      "MagicNumber": 0, // uint32_t
             *      "GenesisHash": "...", // hexstring of uint256
             *      "ExchangeRate": 1, // uint64_t
             *      "EffectiveHeight": 1000, // uint32_t
             *      "ResourcePath": "..." // path string
             *    }
             *    "Signature": "...",
             *    "CRCouncilMemberDID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
             *    "CRCouncilMemberSignature": "...",
             * }
			 * @fee Fee amount. Bigint string in SELA
			 * @memo Remark string
			 * @return
             */
            virtual nlohmann::json CreateRegisterSidechainTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &payload,
                    const std::string &fee,
                    const std::string &memo = "") const = 0;
        };

	}
}

#endif //__ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
