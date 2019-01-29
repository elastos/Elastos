// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
			 * Create a deposit transaction and return the content of transaction in json format. Note that \p amount should greater than sum of \p so that we will leave enough fee for sidechain.
			 * @param fromAddress specify which address we want to spend, or just input empty string to let wallet choose UTXOs automatically.
			 * @param toAddress specify which address we want to send, in this method to address shall be genesis address of the side chain
			 * @param amount specify amount we want to send.
			 * @param sidechainAccounts a list of sidechain accounts in json format.
			 * @param sidechainAmounts a list of sidechain amounts in json format, each amount should correspond to \p sidechainAccounts by order.
			 * @param sidechainIndices a list of sidechain indices in json format, each index should correspond to \p sidechainAccounts by order.
			 * @param memo input memo attribute for describing.
			 * @param remark is used to record message of local wallet.
			 * @return　If success return the content of transaction in json format.
			 */
			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &lockedAddress,
					uint64_t amount,
					const std::string &sideChainAddress,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false) = 0;

			virtual nlohmann::json GenerateProducerPayload(
				const std::string &publicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const = 0;

			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &publicKey,
				const std::string &payPasswd) const = 0;

			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				uint64_t amount,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false) = 0;

			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false) = 0;

			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false) = 0;

			virtual nlohmann::json CreateRetrieveDepositTransaction(
				uint64_t amount,
				const std::string &memo,
				const std::string &remark) = 0;

			virtual std::string GetPublicKeyForVote() const = 0;

			virtual nlohmann::json CreateVoteProducerTransaction(
					const std::string &fromAddress,
					uint64_t stake,
					const nlohmann::json &pubicKeys,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false) = 0;

			virtual	nlohmann::json GetVotedProducerList() const = 0;

			virtual nlohmann::json GetRegisteredProducerInfo() const = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
