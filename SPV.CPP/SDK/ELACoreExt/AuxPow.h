// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AUXPOW_H__
#define __ELASTOS_SDK_AUXPOW_H__

#include <vector>

#include "BRInt.h"
#include "BRTransaction.h"
#include "BRMerkleBlock.h"

#include "SDK/Plugin/Interface/ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class AuxPow :
				public ELAMessageSerializable {
		public:
			AuxPow();

			AuxPow(const AuxPow &auxPow);

			~AuxPow();

			void setAuxMerkleBranch(const std::vector<UInt256> &hashes);
			void setCoinBaseMerkle(const std::vector<UInt256> &hashes);
			void setAuxMerkleIndex(uint32_t index);
			void setParMerkleIndex(uint32_t index);
			void setParentHash(const UInt256 &hash);
			uint32_t getAuxMerkleIndex() const;
			uint32_t getParMerkleIndex() const;
			const UInt256 &getParentHash() const;
			const std::vector<UInt256> &getAuxMerkleBranch() const;
			const std::vector<UInt256> &getParCoinBaseMerkle() const;

			UInt256 getParBlockHeaderHash() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

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

			void deserializeBtcTxIn(ByteStream &istream, BRTransaction *tx);

			void serializeBtcTxOut(ByteStream &ostream, const BRTxOutput &output) const;

			void deserializeBtcTxOut(ByteStream &istream, BRTransaction *tx);

			void serializeBtcBlockHeader(ByteStream &ostream) const;

			void deserializeBtcBlockHeader(ByteStream &istream);

			nlohmann::json transactionToJson() const;

			void transactionFromJson(const nlohmann::json &jsonData);

			nlohmann::json txInputsToJson(const BRTxInput &input) const;

			void txInputsFromJson(const nlohmann::json &input);

			nlohmann::json txOutputsToJson(const BRTxOutput &output) const;

			void txOutputsFromJson(const nlohmann::json &output);

			nlohmann::json  merkleBlockToJson() const;

			void merkleBlockFromJson(nlohmann::json jsonData);
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
