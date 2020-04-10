// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCKBASE_H__
#define __ELASTOS_SDK_MERKLEBLOCKBASE_H__

#include <Plugin/Interface/IMerkleBlock.h>
#include <Plugin/Block/AuxPow.h>

namespace Elastos {
	namespace ElaWallet {

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty _target

		class MerkleBlockBase : public IMerkleBlock {
		public:
			MerkleBlockBase();

			virtual ~MerkleBlockBase();

			virtual uint32_t GetTotalTx() const;

			uint32_t GetVersion() const;

			void SetVersion(uint32_t version);

			const uint256 &GetPrevBlockHash() const;

			void SetPrevBlockHash(const uint256 &hash);

			virtual const uint256 &GetRootBlockHash() const;

			virtual void SetRootBlockHash(const uint256 &hash);

			virtual uint32_t GetTimestamp() const;

			virtual void SetTimestamp(uint32_t timestamp);

			uint32_t GetTarget() const;

			void SetTarget(uint32_t target);

			uint32_t GetNonce() const;

			void SetNonce(uint32_t nonce);

			uint32_t GetTransactionCount() const;

			void SetTransactionCount(uint32_t count);

			const std::vector<uint256> &GetHashes() const;

			void SetHashes(const std::vector<uint256> &hashes);

			const std::vector<uint8_t> &GetFlags() const;

			void SetFlags(const std::vector<uint8_t> &flags);

			virtual const uint256 &GetHash() const { return _blockHash;}

			virtual void SetHash(const uint256 &hash);

			virtual uint32_t GetHeight() const;

			virtual void SetHeight(uint32_t height);

			virtual bool IsValid(uint32_t currentTime) const { return false;}

			virtual bool IsEqual(const IMerkleBlock *block) const;

			virtual std::string GetBlockType() const { return "";}

			size_t MerkleBlockTxHashes(std::vector<uint256> &txHashes) const;

		protected:
			void SerializeNoAux(ByteStream &ostream) const;

			bool DeserializeNoAux(const ByteStream &istream);

			void SerializeAfterAux(ByteStream &ostream) const;

			bool DeserializeAfterAux(const ByteStream &istream);

			uint256 MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const;

			size_t MerkleBlockTxHashesR(std::vector<uint256> &txHashes, size_t &hashIdx, size_t &flagIdx, int depth) const;

			int ceilLog2(int x) const;

		protected:
			mutable uint256 _blockHash;
			uint32_t _version;
			uint256 _prevBlock;
			uint256 _merkleRoot;
			uint32_t _timestamp; // time interval since unix epoch
			uint32_t _target;
			uint32_t _nonce;
			uint32_t _totalTx;
			std::vector<uint256> _hashes;
			bytes_t _flags;
			uint32_t _height;
		};

	}
}


#endif //SPVSDK_MERKLEBLOCKBASE_H
