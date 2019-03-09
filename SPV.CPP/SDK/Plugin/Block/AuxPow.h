// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AUXPOW_H__
#define __ELASTOS_SDK_AUXPOW_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>

#include <Core/BRInt.h>
#include <Core/BRTransaction.h>
#include <Core/BRMerkleBlock.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class AuxPow :
				public ELAMessageSerializable {
		public:
			AuxPow();

			AuxPow(const AuxPow &auxPow);

			~AuxPow();

			void SetAuxMerkleBranch(const std::vector<UInt256> &hashes);
			void SetCoinBaseMerkle(const std::vector<UInt256> &hashes);
			void SetAuxMerkleIndex(uint32_t index);
			void SetParMerkleIndex(uint32_t index);
			void SetParentHash(const UInt256 &hash);
			uint32_t GetAuxMerkleIndex() const;
			uint32_t GetParMerkleIndex() const;
			const UInt256 &GetParentHash() const;
			const std::vector<UInt256> &GetAuxMerkleBranch() const;
			const std::vector<UInt256> &GetParCoinBaseMerkle() const;

			UInt256 GetParBlockHeaderHash() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			BRTransaction *GetBTCTransaction() const;

			void SetBTCTransaction(BRTransaction *transaction);

			BRMerkleBlock *GetParBlockHeader() const;

			void SetParBlockHeader(BRMerkleBlock *block);

			AuxPow &operator=(const AuxPow &auxPow);

		private:
			void SerializeBtcTransaction(ByteStream &ostream, const BRTransaction *tx) const;

			bool DeserializeBtcTransaction(ByteStream &istream, BRTransaction *tx);

			void SerializeBtcTxIn(ByteStream &ostream, const BRTxInput *in) const;

			bool DeserializeBtcTxIn(ByteStream &istream, BRTransaction *tx);

			void SerializeBtcTxOut(ByteStream &ostream, const BRTxOutput *out) const;

			bool DeserializeBtcTxOut(ByteStream &istream, BRTransaction *tx);

			void SerializeBtcBlockHeader(ByteStream &ostream, const BRMerkleBlock *b) const;

			bool DeserializeBtcBlockHeader(ByteStream &istream, BRMerkleBlock *b);

			nlohmann::json TransactionToJson() const;

			void TransactionFromJson(const nlohmann::json &jsonData);

			nlohmann::json TxInputsToJson(const BRTxInput &input) const;

			void TxInputsFromJson(const nlohmann::json &input);

			nlohmann::json TxOutputsToJson(const BRTxOutput &output) const;

			void TxOutputsFromJson(const nlohmann::json &output);

			nlohmann::json  MerkleBlockToJson() const;

			void MerkleBlockFromJson(nlohmann::json jsonData);
		private:
			std::vector<UInt256> _auxMerkleBranch;
			uint32_t             _auxMerkleIndex;
			BRTransaction       *_parCoinBaseTx;
			std::vector<UInt256> _parCoinBaseMerkle;
			uint32_t             _parMerkleIndex;
			BRMerkleBlock       *_parBlockHeader;
			UInt256              _parentHash;
		};

	}
}

#endif //SPVSDK_AUXPOW_H
