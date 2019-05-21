// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

#include "SubWallet.h"
#include <Interface/IMainchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class MainchainSubWallet : public IMainchainSubWallet, public SubWallet {
		public:
			~MainchainSubWallet();

			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &lockedAddress,
					uint64_t Amount,
					const std::string &sideChainAddress,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false);

			virtual nlohmann::json GenerateProducerPayload(
				const std::string &publicKey,
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
				uint64_t amount,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false);

			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false);

			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &emmo,
				const std::string &remark,
				bool useVotedUTXO = false);

			virtual nlohmann::json CreateRetrieveDepositTransaction(
				uint64_t amount,
				const std::string &memo,
				const std::string &remark);

			virtual std::string GetPublicKeyForVote() const;

			virtual nlohmann::json CreateVoteProducerTransaction(
					const std::string &fromAddress,
					uint64_t stake,
					const nlohmann::json &publicKeys,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false);

			virtual	nlohmann::json GetVotedProducerList() const;

			virtual nlohmann::json GetRegisteredProducerInfo() const;

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const PluginType &pluginTypes,
							   MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
