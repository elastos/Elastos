// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRMerkleBlock.h"

#include "MerkleBlock.h"

namespace Elastos {
	namespace SDK {

		MerkleBlock::MerkleBlock(BRMerkleBlock *merkleBlock) :
			_merkleBlock(merkleBlock) {
		}

		MerkleBlock::MerkleBlock(ByteData block, int blockHeight) {
			_merkleBlock = BRMerkleBlockParse((const uint8_t *) block.data, blockHeight);
		}

		MerkleBlock::~MerkleBlock() {
			if (_merkleBlock != nullptr)
				BRMerkleBlockFree(_merkleBlock);
		}

		std::string MerkleBlock::toString() const {
			//todo complete me
			return "";
		}

		BRMerkleBlock *MerkleBlock::getRaw() const {
			return _merkleBlock;
		}

		UInt256 MerkleBlock::getBlockHash() const {
			return _merkleBlock->blockHash;
		}

		uint32_t MerkleBlock::getVersion() const {
			return _merkleBlock->version;
		}

		UInt256 MerkleBlock::getPrevBlockHash() const {
			return _merkleBlock->prevBlock;
		}

		UInt256 MerkleBlock::getRootBlockHash() const {
			return _merkleBlock->merkleRoot;
		}

		uint32_t MerkleBlock::getTimestamp() const {
			return _merkleBlock->timestamp;
		}

		uint32_t MerkleBlock::getTarget() const {
			return _merkleBlock->target;
		}

		uint32_t MerkleBlock::getNonce() const {
			return _merkleBlock->nonce;
		}

		uint32_t MerkleBlock::getTransactionCount() const {
			return _merkleBlock->totalTx;
		}

		uint32_t MerkleBlock::getHeight() const {
			return _merkleBlock->height;
		}

		ByteData MerkleBlock::serialize() const {
			size_t len = BRMerkleBlockSerialize(_merkleBlock, NULL, 0);
			uint8_t* data = new uint8_t[len];
			BRMerkleBlockSerialize(_merkleBlock, data, len);
			return ByteData(data, len);
		}

		bool MerkleBlock::isValid(uint32_t currentTime) const {
			return BRMerkleBlockIsValid(_merkleBlock, (uint32_t) currentTime);
		}

		bool MerkleBlock::containsTransactionHash(UInt256 hash) const {
			return BRMerkleBlockContainsTxHash(_merkleBlock, hash);
		}

		void to_json(nlohmann::json& j, const MerkleBlock& p) {
			//todo complete me
		}

		void from_json(const nlohmann::json& j, MerkleBlock& p) {
			//todo complete me
		}

	}
}