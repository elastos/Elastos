// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDASSET_H__
#define __ELASTOS_SDK__GROUPEDASSET_H__

#include "UTXO.h"

#include <Common/ElementSet.h>
#include <Common/Lockable.h>
#include <Account/SubAccount.h>
#include <Common/BigInt.h>
#include <Plugin/Transaction/Payload/IPayload.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Wallet;
		class Asset;
		class Transaction;
		class TransactionInput;
		class VoteContent;
		typedef boost::shared_ptr<Asset> AssetPtr;
		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef std::vector<InputPtr> InputArray;
		typedef std::vector<VoteContent> VoteContentArray;

		class GroupedAsset {
		public:
			GroupedAsset();

			GroupedAsset(Wallet *parent, const AssetPtr &asset);

			GroupedAsset(const GroupedAsset &proto);

			GroupedAsset &operator=(const GroupedAsset &proto);

			void ClearData();

			UTXOArray GetUTXOs(const std::string &addr) const;

			const UTXOSet &GetVoteUTXO() const;

			const UTXOSet &GetCoinBaseUTXOs() const;

			TransactionPtr CreateRetrieveDepositTx(uint8_t type, const PayloadPtr &payload, const BigInt &amount,
												   const AddressPtr &fromAddress, const std::string &memo);

			TransactionPtr Vote(const VoteContent &voteContent, const std::string &memo, bool max,
			                    VoteContentArray &dropedVotes);

			TransactionPtr Consolidate(const std::string &memo);

			TransactionPtr CreateTxForOutputs(uint8_t type,
											  const PayloadPtr &payload,
											  const OutputArray &outputs,
											  const AddressPtr &fromAddress,
											  const std::string &memo,
											  bool max,
											  const BigInt &fee,
											  bool pickVoteFirst = false);

			const AssetPtr &GetAsset() const;

		private:
			UTXOSet _utxos, _utxosVote, _utxosCoinbase, _utxosDeposit, _utxosLocked;

			AssetPtr _asset;

			Wallet *_parent;
		};

		typedef boost::shared_ptr<GroupedAsset> GroupedAssetPtr;

	}
}


#endif //__ELASTOS_SDK__GROUPEDASSET_H__
