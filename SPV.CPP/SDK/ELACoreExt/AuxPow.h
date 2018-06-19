// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AUXPOW_H__
#define __ELASTOS_SDK_AUXPOW_H__

#include <vector>

#include "BRInt.h"
#include "BRTransaction.h"
#include "BRMerkleBlock.h"

#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class AuxPow :
				public ELAMessageSerializable {
		public:
			AuxPow();

			AuxPow(const AuxPow &auxPow);

			~AuxPow();

			UInt256 getParBlockHeaderHash() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &jsonData);

			BRTransaction *getBTCTransaction() const;

			void setBTCTransaction(BRTransaction *transaction);

			BRMerkleBlock *getParBlockHeader() const;

			void setParBlockHeader(BRMerkleBlock *block);

			AuxPow &operator=(const AuxPow &auxPow);

		private:
			void serializeBtcTransaction(ByteStream &ostream) const;

			void deserializeBtcTransaction(ByteStream &istream);

			void serializeBtcTxIn(ByteStream &ostream, const BRTxInput &input) const;

			void deserializeBtcTxIn(ByteStream &istream, BRTxInput &input);

			void serializeBtcTxOut(ByteStream &ostream, const BRTxOutput &output) const;

			void deserializeBtcTxOut(ByteStream &istream, BRTxOutput &output);

			void serializeBtcBlockHeader(ByteStream &ostream) const;

			void deserializeBtcBlockHeader(ByteStream &istream);

			nlohmann::json transactionToJson();

			void transactionFromJson(nlohmann::json jsonData) const;

			nlohmann::json txInputsToJson(size_t index);

			void txInputsFromJson(std::vector<nlohmann::json> inputs) const;

			nlohmann::json txOutputsToJson(size_t index);

			void txOutputsFromJson(std::vector<nlohmann::json> outputs) const;

			nlohmann::json  merkleBlockToJson();

			void merkleBlockFromJson(nlohmann::json jsonData) const;
		private:
			std::vector<UInt256> _auxMerkleBranch;
			std::vector<UInt256> _parCoinBaseMerkle;
			uint32_t _auxMerkleIndex;
			BRTransaction *_btcTransaction;
			uint32_t _parMerkleIndex;
			BRMerkleBlock *_parBlockHeader;
			UInt256 _parentHash;
		};

	}
}

#endif //SPVSDK_AUXPOW_H
