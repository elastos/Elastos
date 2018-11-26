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

			uint32_t getVersion() const;

			void setVersion(uint32_t version);

			const UInt256 &getPrevBlockHash() const;

			void setPrevBlockHash(const UInt256 &hash);

			virtual const UInt256 &getRootBlockHash() const;

			virtual void setRootBlockHash(const UInt256 &hash);

			virtual uint32_t getTimestamp() const;

			virtual void setTimestamp(uint32_t timestamp);

			uint32_t getTarget() const;

			void setTarget(uint32_t target);

			uint32_t getNonce() const;

			void setNonce(uint32_t nonce);

			uint32_t getTransactionCount() const;

			void setTransactionCount(uint32_t count);

			const std::vector<UInt256> &getHashes() const;

			void setHashes(const std::vector<UInt256> &hashes);

			const std::vector<uint8_t> &getFlags() const;

			void setFlags(const std::vector<uint8_t> &flags);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual const UInt256 &getHash() const { return _blockHash;}

			virtual void setHash(const UInt256 &hash);

			virtual uint32_t getHeight() const;

			virtual void setHeight(uint32_t height);

			virtual bool isValid(uint32_t currentTime) { return false;}

			virtual bool isEqual(const IMerkleBlock *block) const;

			virtual std::string getBlockType() const { return "";}

			std::vector<UInt256> MerkleBlockTxHashes() const;

		protected:
			void serializeNoAux(ByteStream &ostream) const;

			bool deserializeNoAux(ByteStream &istream);

			void serializeAfterAux(ByteStream &ostream) const;

			bool deserializeAfterAux(ByteStream &istream);

			UInt256 MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const;

			std::vector<UInt256> merkleBlockTxHashesR(size_t &hashIdx, size_t &flagIdx, int depth) const;

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
