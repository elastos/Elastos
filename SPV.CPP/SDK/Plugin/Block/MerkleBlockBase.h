// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCKBASE_H__
#define __ELASTOS_SDK_MERKLEBLOCKBASE_H__

#include <SDK/Plugin/Interface/IMerkleBlock.h>
#include <SDK/Plugin/Block/AuxPow.h>

namespace Elastos {
	namespace ElaWallet {

		class MerkleBlockBase : public IMerkleBlock {
		public:
			MerkleBlockBase();

			virtual ~MerkleBlockBase();

			uint32_t GetVersion() const;

			void SetVersion(uint32_t version);

			const UInt256 &GetPrevBlockHash() const;

			void SetPrevBlockHash(const UInt256 &hash);

			virtual const UInt256 &GetRootBlockHash() const;

			virtual void SetRootBlockHash(const UInt256 &hash);

			virtual uint32_t GetTimestamp() const;

			virtual void SetTimestamp(uint32_t timestamp);

			uint32_t GetTarget() const;

			void SetTarget(uint32_t target);

			uint32_t GetNonce() const;

			void SetNonce(uint32_t nonce);

			uint32_t GetTransactionCount() const;

			void SetTransactionCount(uint32_t count);

			const std::vector<UInt256> &GetHashes() const;

			void SetHashes(const std::vector<UInt256> &hashes);

			const std::vector<uint8_t> &GetFlags() const;

			void SetFlags(const std::vector<uint8_t> &flags);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual const UInt256 &GetHash() const { return _blockHash;}

			virtual void SetHash(const UInt256 &hash);

			virtual uint32_t GetHeight() const;

			virtual void SetHeight(uint32_t height);

			virtual bool IsValid(uint32_t currentTime) const { return false;}

			virtual bool IsEqual(const IMerkleBlock *block) const;

			virtual std::string GetBlockType() const { return "";}

			size_t MerkleBlockTxHashes(std::vector<UInt256> &txHashes) const;

		protected:
			void SerializeNoAux(ByteStream &ostream) const;

			bool DeserializeNoAux(ByteStream &istream);

			void SerializeAfterAux(ByteStream &ostream) const;

			bool DeserializeAfterAux(ByteStream &istream);

			UInt256 MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const;

			size_t MerkleBlockTxHashesR(std::vector<UInt256> &txHashes, size_t &hashIdx, size_t &flagIdx, int depth) const;

			int ceilLog2(int x) const;

		protected:
			UInt256 _blockHash;
			uint32_t _version;
			UInt256 _prevBlock;
			UInt256 _merkleRoot;
			uint32_t _timestamp; // time interval since unix epoch
			uint32_t _target;
			uint32_t _nonce;
			uint32_t _totalTx;
			std::vector<UInt256> _hashes;
			std::vector<uint8_t> _flags;
			uint32_t _height;
		};

	}
}


#endif //SPVSDK_MERKLEBLOCKBASE_H
