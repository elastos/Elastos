// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AUXPOW_H__
#define __ELASTOS_SDK_AUXPOW_H__

#include <Plugin/Interface/ELAMessageSerializable.h>

#include <bitcoin/BRTransaction.h>
#include <bitcoin/BRMerkleBlock.h>

#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class AuxPow {
		public:
			AuxPow();

			AuxPow(const AuxPow &auxPow);

			~AuxPow();

			void SetAuxMerkleBranch(const std::vector<uint256> &hashes);
			void SetCoinBaseMerkle(const std::vector<uint256> &hashes);
			void SetAuxMerkleIndex(uint32_t index);
			void SetParMerkleIndex(uint32_t index);
			void SetParentHash(const uint256 &hash);
			uint32_t GetAuxMerkleIndex() const;
			uint32_t GetParMerkleIndex() const;
			const uint256 &GetParentHash() const;
			const std::vector<uint256> &GetAuxMerkleBranch() const;
			const std::vector<uint256> &GetParCoinBaseMerkle() const;

			uint256 GetParBlockHeaderHash() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			BRTransaction *GetBTCTransaction() const;

			void SetBTCTransaction(BRTransaction *transaction);

			BRMerkleBlock *GetParBlockHeader() const;

			void SetParBlockHeader(BRMerkleBlock *block);

			AuxPow &operator=(const AuxPow &auxPow);

		private:
			void SerializeBtcTransaction(ByteStream &ostream, const BRTransaction *tx) const;

			bool DeserializeBtcTransaction(const ByteStream &istream, BRTransaction *tx);

			void SerializeBtcTxIn(ByteStream &ostream, const BRTxInput *in) const;

			bool DeserializeBtcTxIn(const ByteStream &istream, BRTransaction *tx);

			void SerializeBtcTxOut(ByteStream &ostream, const BRTxOutput *out) const;

			bool DeserializeBtcTxOut(const ByteStream &istream, BRTransaction *tx);

			void SerializeBtcBlockHeader(ByteStream &ostream, const BRMerkleBlock *b) const;

			bool DeserializeBtcBlockHeader(const ByteStream &istream, BRMerkleBlock *b);

		private:
			std::vector<uint256> _auxMerkleBranch;
			uint32_t             _auxMerkleIndex;
			BRTransaction       *_parCoinBaseTx;
			std::vector<uint256> _parCoinBaseMerkle;
			uint32_t             _parMerkleIndex;
			BRMerkleBlock       *_parBlockHeader;
			uint256              _parentHash;
		};

	}
}

#endif //SPVSDK_AUXPOW_H
