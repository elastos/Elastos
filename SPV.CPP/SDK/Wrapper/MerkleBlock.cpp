// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRMerkleBlock.h"
#include "Utils.h"

#include "MerkleBlock.h"

namespace Elastos {
	namespace SDK {

		MerkleBlock::MerkleBlock(BRMerkleBlock *merkleBlock) :
			_merkleBlock(merkleBlock) {
		}

		MerkleBlock::MerkleBlock(const ByteData &block, int blockHeight) {
			_merkleBlock = BRMerkleBlockParse((const uint8_t *) block.data, block.length);
			if (_merkleBlock != nullptr && blockHeight != -1) {
				_merkleBlock->height = (uint32_t) blockHeight;
			}
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
			size_t len = BRMerkleBlockSerialize(_merkleBlock, nullptr, 0);
			uint8_t *data = new uint8_t[len];
			BRMerkleBlockSerialize(_merkleBlock, data, len);
			return ByteData(data, len);
		}

		bool MerkleBlock::isValid(uint32_t currentTime) const {
			return BRMerkleBlockIsValid(_merkleBlock, currentTime) != 0;
		}

		bool MerkleBlock::containsTransactionHash(UInt256 hash) const {
			return BRMerkleBlockContainsTxHash(_merkleBlock, hash) != 0;
		}

		void to_json(nlohmann::json &j, const MerkleBlock &p) {
			BRMerkleBlock *pblock = p.getRaw();

			std::vector<std::string> hashes;
			for (int i = 0; i < pblock->hashesCount; ++i) {
				hashes.push_back(Utils::UInt256ToString(pblock->hashes[i]));
			}

			std::vector<uint8_t> flags;
			for (int i = 0; i < pblock->flagsLen; ++i) {
				flags.push_back(pblock->flags[i]);
			}

			j["blockHash"] = Utils::UInt256ToString(pblock->blockHash);
			j["version"] = pblock->version;
			j["prevBlock"] = Utils::UInt256ToString(pblock->prevBlock);
			j["merkleRoot"] = Utils::UInt256ToString(pblock->merkleRoot);
			j["timestamp"] = pblock->timestamp;
			j["target"] = pblock->target;
			j["nonce"] = pblock->nonce;
			j["totalTx"] = pblock->totalTx;
			j["hashes"] = hashes;
			j["flags"] = flags;
			j["height"] = pblock->height;
		}

		void from_json(const nlohmann::json &j, MerkleBlock &p) {
			BRMerkleBlock *pblock = p.getRaw();
			if (pblock == nullptr) {
				pblock = BRMerkleBlockNew();
				p = MerkleBlock(pblock);
			}

			pblock->blockHash = Utils::UInt256FromString(j["blockHash"].get<std::string>());
			pblock->version = j["version"].get<uint32_t>();
			pblock->prevBlock = Utils::UInt256FromString(j["prevBlock"].get<std::string>());
			pblock->merkleRoot = Utils::UInt256FromString(j["merkleRoot"].get<std::string>());
			pblock->timestamp = j["timestamp"].get<uint32_t>();
			pblock->target = j["target"].get<uint32_t>();
			pblock->nonce = j["nonce"].get<uint32_t>();
			pblock->totalTx = j["totalTx"].get<uint32_t>();

			if (pblock->hashes != nullptr) {
				free(pblock->hashes);
				pblock->hashes = nullptr;
			}

			std::vector<std::string> hashes = j["hashes"].get<std::vector<std::string>>();
			pblock->hashesCount = hashes.size();
			pblock->hashes = (pblock->hashesCount > 0) ? (UInt256 *) malloc(sizeof(UInt256) * pblock->hashesCount)
													   : nullptr;
			for (int i = 0; i < pblock->hashesCount; ++i) {
				UInt256 hash = Utils::UInt256FromString(hashes[i]);
				memcpy(&pblock->hashes[i], &hash, sizeof(hash));
			}

			if (pblock->flags != nullptr) {
				free(pblock->flags);
				pblock->flags = nullptr;
			}

			std::vector<uint8_t> flags = j["flags"].get<std::vector<uint8_t>>();
			pblock->flagsLen = flags.size();
			pblock->flags = (pblock->flagsLen > 0) ? (uint8_t *) malloc(pblock->flagsLen) : nullptr;
			for (int i = 0; i < pblock->flagsLen; ++i) {
				pblock->flags[i] = flags[i];
			}

			pblock->height = j["height"].get<uint32_t>();
		}

	}
}
