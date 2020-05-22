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

		class IMainchainSubWallet : public virtual ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~IMainchainSubWallet() noexcept {}

			/**
			 * Deposit token from the main chain to side chains, such as ID chain or token chain, etc.
			 *
			 * @param fromAddress      If this address is empty, wallet will pick available UTXO automatically.
			 *                         Otherwise, wallet will pick UTXO from the specific address.
			 * @param sideChainID      Chain id of the side chain.
			 * @param amount           The amount that will be deposit to the side chain.
			 * @param sideChainAddress Receive address of side chain.
			 * @memo                   Remarks string. Can be empty string.
			 * @return                 The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &memo) = 0;

		public:
			//////////////////////////////////////////////////
			/*                      Vote                    */
			//////////////////////////////////////////////////
			/**
			 * Create vote transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param stake        Vote amount in sela. "-1" means max.
			 * @param publicKeys   Public keys array in JSON format.
			 * example:
			 * 	[ "pubkey_1", "pubkey_2", ..., "pubkey_N" ]
			 * @param memo         Remarks string. Can be empty string.
			 * @invalidCandidates  invalid candidate except current vote candidates. Such as:
			  						[
								      	{
								            "Type":"CRC",
								            "Candidates":[ "cid", ...]
								        },
								        {
								        	"Type":"CRCProposal",
								        	"Candidates":[ "proposal hash", ...]
								        },
								        {
								        	"Type": "CRCImpeachment",
								        	"Candidates": [ "cid", ...]
								        }
								    ]
			 * @return             The transaction in JSON format to be signed and published. Note: "DropVotes" means the old vote will be dropped.
			 */
			virtual nlohmann::json CreateVoteProducerTransaction(
				const std::string &fromAddress,
				const std::string &stake,
				const nlohmann::json &pubicKeys,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) = 0;

			/**
			 * Create vote cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        Candidate's cid and votes in JSON format.
			 * example:
			 * {
			 *      "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "100000000",
			 *      "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP": "200000000",
			 *      ...
			 * }
			 * @param memo         Remarks string. Can be empty string.
			 * @param invalidCandidates  invalid candidate except current vote candidates.
			 * example:
			  						[
								      	{
								            "Type":"CRCImpeachment",
								            "Candidates":[ "cid", ...]
								        },
								        {
								            "Type":"Delegate",
								            "Candidates":[ "pubkey", ...]
								        },
								        {
								        	"Type":"CRCProposal",
								        	"Candidates":[ "proposal hash", ...]
								        }
								    ]
			 * @return             The transaction in JSON format to be signed and published. Note: "DropVotes" means the old vote will be dropped.
			 */
			virtual nlohmann::json CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) = 0;

			/**
			 * Create vote crc proposal transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        Proposal hash and votes in JSON format. Such as:
			 *                     {
			 *                          "109780cf45c7a6178ad674ac647545b47b10c2c3e3b0020266d0707e5ca8af7c": "100000000",
			 *                          "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7": "200000000"
			 *                     }
			 * @param memo         Remarks string. Can be empty string.
			 * @param invalidCandidates  invalid candidate except current vote candidates. Such as:
			  						[
								      	{
								            "Type":"CRC",
								            "Candidates":[ "cid", ...]
								        },
								        {
								            "Type":"Delegate",
								            "Candidates":[ "pubkey", ...]
								        },
								        {
								        	"Type":"CRCImpeachment",
								        	"Candidates":[ "cid", ...]
								        }
								    ]
			 * @return             The transaction in JSON format to be signed and published. Note: "DropVotes" means the old vote will be dropped.
			 */
			virtual nlohmann::json CreateVoteCRCProposalTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) = 0;

			/**
			 * Create impeachment crc transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        CRC cid and votes in JSON format. Such as:
			 *                     {
			 *                          "innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs": "100000000",
			 *                          "iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww": "200000000"
			 *                     }
			 * @param memo         Remarks string. Can be empty string.
			 * @param invalidCandidates  invalid candidate except current vote candidates. Such as:
			  						[
								      	{
								            "Type":"CRC",
								            "Candidates":[ "cid", ...]
								        },
								        {
								            "Type":"Delegate",
								            "Candidates":[ "pubkey", ...]
								        },
								        {
								        	"Type":"CRCProposal",
								        	"Candidates":[ "proposal hash", ...]
								        }
								    ]
			 * @return             The transaction in JSON format to be signed and published. Note: "DropVotes" means the old vote will be dropped.
			 */
			virtual nlohmann::json CreateImpeachmentCRCTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo,
				const nlohmann::json &invalidCandidates) = 0;

			/**
			 * deprecated. Use GetVoteInfo instead.
			 * Get vote information of current wallet.
			 *
			 * @return Vote information in JSON format.
			 * example:
			 * {
			 * 	 "02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5": "100000000",
			 * 	 "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D": "100000000",
			 * 	 ...
			 * }
			 */
			virtual	nlohmann::json GetVotedProducerList() const = 0;

			/**
			 * deprecated. Use GetVoteInfo instead.
			 * Get CR vote information of current wallet.
			 *
			 * @return Vote information in JSON format. The key is cid, and the value is the stake.
			 * example:
			 * {
			 * 	 "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "10000000",
			 * 	 "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP": "200000000",
			 * 	 ...
			 * }
			 */
			virtual	nlohmann::json GetVotedCRList() const = 0;

			/**
			 * Get summary or details of all types of votes
			 * @type if the type is empty, a summary of all types of votes will return. Otherwise, the details of the specified type will return.
			 *   type can be value: "Delegate", "CRC", "CRCProposal", "CRCImpeachment"
			 * @return vote info in JSON format. Such as:
			 *
			 * details:
			 *  [{
			 *      "Type": "Delegate",
			 *      "Amount": "200000000",
			 *      "Votes": {"pubkey_1": "200000000","pubkey_2": "200000000"}
			 *  },
			 *  {
			 *      "Type": "CRC",
			 *      "Amount": "100000000",
			 *      "Votes": {"cid_1": "50000000", "cid_2": "50000000"}
			 *  },
			 *  {
			 *  	"Type": "CRCProposal",
			 *  	"Amount": "100000000",
			 *      "Votes": {"proposalHash_1": "100000000", "proposalHash_2": "100000000"}
			 *  },
			 *  {
			 *  	"Type": "CRCImpeachment",
			 *  	"Amount": "100000000",
			 *  	"Votes": {"cid_1": "30000000", "cid_2": "70000000"}
			 *  }]
			 */
			virtual nlohmann::json GetVoteInfo(const std::string &type) const = 0;

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
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &publicKey,
				const std::string &payPasswd) const = 0;

			/**
			 * Create register producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param amount       Amount must lager than 500,000,000,000 sela
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &memo) = 0;

			/**
			 * Create update producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo) = 0;

			/**
			 * Create cancel producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateCancelProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo) = 0;

			/**
			 * Create retrieve deposit transaction.
			 *
			 * @param amount     The available amount to be retrieved back.
			 * @param memo       Remarks string. Can be empty string.
			 *
			 * @return           The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveDepositTransaction(
				const std::string &amount,
				const std::string &memo) = 0;

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
			 * Get information about whether the current wallet has been registered the producer.
			 *
			 * @return Information in JSON format. Such as:
			 * { "Status": "Unregistered", "Info": null }
			 *
			 * {
			 *    "Status": "Registered",
			 *    "Info": {
			 *      "OwnerPublicKey": "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D",
			 *      "NodePublicKey": "02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5",
			 *      "NickName": "hello nickname",
			 *      "URL": "www.google.com",
			 *      "Location": 86,
			 *      "Address": 127.0.0.1,
			 *    }
			 * }
			 *
			 * { "Status": "Canceled", "Info": { "Confirms": 2016 } }
			 *
			 * { "Status": "ReturnDeposit", "Info": null }
			 */
			virtual nlohmann::json GetRegisteredProducerInfo() const = 0;


		public:
			//////////////////////////////////////////////////
			/*                      CRC                     */
			//////////////////////////////////////////////////

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
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateCRInfoPayload().
			 * @param amount       Amount must lager than 500,000,000,000 sela
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &amount,
					const std::string &memo) = 0;

			/**
			 * Create update cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateCRInfoPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo) = 0;

			/**
			 * Create unregister cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateUnregisterCRPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUnregisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo) = 0;

			/**
			 * Create retrieve deposit cr transaction.
			 *
			 * @param crPublicKey The public key to identify a cr.
			 * @param amount      The available amount to be retrieved back.
			 * @param memo        Remarks string. Can be empty string.
			 * @return            The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveCRDepositTransaction(
					const std::string &crPublicKey,
					const std::string &amount,
					const std::string &memo) = 0;

			/**
			 * Get information about whether the current wallet has been registered the producer.
			 *
			 * @return Information in JSON format. Such as:
			 * { "Status": "Unregistered", "Info": null }
			 *
			 * {
			 *    "Status": "Registered",
			 *    "Info": {
			 *      "CROwnerPublicKey": "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D",
			 *      "CID": "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP",
			 *      "DID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY",
			 *      "BondedDID": true,
			 *      "NickName": "hello nickname",
			 *      "URL": "www.google.com",
			 *      "Location": 86,
			 *    }
			 * }
			 *
			 * { "Status": "Canceled", "Info": { "Confirms": 2016 } }
			 *
			 * { "Status": "ReturnDeposit", "Info": null }
			 */
			virtual nlohmann::json GetRegisteredCRInfo() const = 0;


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
			 * @param payload Proposal payload signed by owner and CR committee.
			 * {
			 *    "Type": 0,                   // same as mention on method ProposalOwnerDigest()
			 *    "CategoryData": "testdata",  // limit: 4096 bytes
			 *    "OwnerPublicKey": "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4", // Owner DID public key
			 *    "DraftHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
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
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateProposalTransaction(const nlohmann::json &payload,
															 const std::string &memo = "") = 0;


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
			 *   "DID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY", // did of CR council member
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalReviewDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Create proposal review transaction.
			 *
			 * @param payload Signed payload.
 			 * {
			 *   "ProposalHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "VoteResult": 1,    // approve = 0, reject = 1, abstain = 2
			 *   "OpinionHash": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "DID": "icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY", // did of CR council member's did
			 *   // signature of CR council member
			 *   "Signature": "ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76"
			 * }
			 *
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateProposalReviewTransaction(const nlohmann::json &payload,
																   const std::string &memo = "") = 0;


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
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty. Otherwise not empty.
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
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty. Otherwise not empty.
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
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty. Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "OwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   // If NewOwnerPubKey is empty, this must be empty.
			 *   "NewOwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   "Type": 0, // common = 0, progress = 1, rejected = 2, terminated = 3, changeOwner = 4, finalized = 5
			 *   "SecretaryGeneralOpinionHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 * }
			 *
			 * @return Digest of payload
			 */
			virtual std::string ProposalTrackingSecretaryDigest(const nlohmann::json &payload) const = 0;


			/**
			 * Create proposal tracking transaction.
			 *
			 * @param payload Proposal tracking payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "MessageHash": "0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4",
			 *   "Stage": 0, // value can be [0, 128)
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   // If this proposal tracking is not use for changing owner, will be empty. Otherwise not empty.
			 *   "NewOwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "OwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   // If NewOwnerPubKey is empty, this must be empty.
			 *   "NewOwnerSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109",
			 *   "Type": 0, // common = 0, progress = 1, rejected = 2, terminated = 3, changeOwner = 4, finalized = 5
			 *   "SecretaryGeneralOpinionHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "SecretaryGeneralSignature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109"
			 * }
			 *
			 * @param memo Remarks string. Can be empty string.
			 * @return The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json
			CreateProposalTrackingTransaction(const nlohmann::json &payload, const std::string &memo = "") = 0;

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
			 * }
			 *
			 * @return Digest of payload.
			 */
			virtual std::string ProposalWithdrawDigest(const nlohmann::json &payload) const = 0;

			/**
			 * Create proposal withdraw transaction.
			 * Note: This tx does not need to be signed.
			 *
			 * @param recipient Recipient of proposal.
			 * @param amount Withdraw amount.
			 * @param utxo UTXO json array of address CREXPENSESXXXXXXXXXXXXXXXXXX4UdT6b.
			 * [{
			 *   "Hash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "Index": 0,
			 *   "Amount": "100000000",   // 1 ela = 100000000 sela
			 * },{
			 *   "Hash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "Index": 2,
			 *   "Amount": "200000000",   // 2 ela = 200000000 sela
			 * }]
			 * @param payload Proposal payload.
			 * {
			 *   "ProposalHash": "7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513",
			 *   "OwnerPublicKey": "02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331",
			 *   "Signature": "9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109"
			 * }
			 *
			 * @param memo Remarks string. Can be empty string.
			 *
			 * @return Transaction in JSON format.
			 */
			 virtual nlohmann::json CreateProposalWithdrawTransaction(const std::string &recipient,
																	  const std::string &amount,
																	  const nlohmann::json &utxo,
																	  const nlohmann::json &payload,
																	  const std::string &memo = "") = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
