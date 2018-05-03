// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRMerkleBlock.h>
#include "BRCrypto.h"
#include "BRMerkleBlock.h"
#include "Utils.h"

#include "MerkleBlock.h"

namespace Elastos {
	namespace SDK {

		namespace {
#define MAX_PROOF_OF_WORK 0x1d00ffff    // highest value for difficulty target (higher values are less difficult)
#define TARGET_TIMESPAN   (14*24*60*60) // the targeted timespan between difficulty target adjustments

			int _ceil_log2(int x)
			{
				int r = (x & (x - 1)) ? 1 : 0;

				while ((x >>= 1) != 0) r++;
				return r;
			}

			UInt256 BRMerkleBlockRootR(const BRMerkleBlock *block, size_t *hashIdx, size_t *flagIdx, int depth)
			{
				uint8_t flag;
				UInt256 hashes[2], md = UINT256_ZERO;

				if (block->flagsLen == 0 && block->hashesCount == 1 && UInt256Eq(&(block->hashes[0]), &(block->merkleRoot))) {
					return block->hashes[0];
				}

				if (*flagIdx/8 < block->flagsLen && *hashIdx < block->hashesCount) {
					flag = (block->flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
					(*flagIdx)++;

					if (flag && depth != _ceil_log2(block->totalTx)) {
						hashes[0] = BRMerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // left branch
						hashes[1] = BRMerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // right branch

						if (! UInt256IsZero(&hashes[0]) && ! UInt256Eq(&(hashes[0]), &(hashes[1]))) {
							if (UInt256IsZero(&hashes[1])) hashes[1] = hashes[0]; // if right branch is missing, dup left branch
							BRSHA256_2(&md, hashes, sizeof(hashes));
						}
						else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
					}
					else md = block->hashes[(*hashIdx)++]; // leaf
				}

				return md;
			}

			size_t BRMerkleBlockTxHashesR(const BRMerkleBlock *block, UInt256 *txHashes, size_t hashesCount, size_t *idx,
										   size_t *hashIdx, size_t *flagIdx, int depth)
			{
				uint8_t flag;

				if (block->flagsLen == 0 && block->hashesCount == 1 && UInt256Eq(&(block->hashes[0]), &(block->merkleRoot))) {
					txHashes[*idx] = block->hashes[*hashIdx];
					(*idx)++;
				}

				if (*flagIdx/8 < block->flagsLen && *hashIdx < block->hashesCount) {
					flag = (block->flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
					(*flagIdx)++;

					if (! flag || depth == _ceil_log2(block->totalTx)) {
						if (flag && *idx < hashesCount) {
							if (txHashes) txHashes[*idx] = block->hashes[*hashIdx]; // leaf
							(*idx)++;
						}

						(*hashIdx)++;
					}
					else {
						BRMerkleBlockTxHashesR(block, txHashes, hashesCount, idx, hashIdx, flagIdx, depth + 1); // left branch
						BRMerkleBlockTxHashesR(block, txHashes, hashesCount, idx, hashIdx, flagIdx, depth + 1); // right branch
					}
				}

				return *idx;
			}
		}

		MerkleBlock::MerkleBlock() {
			_merkleBlock = BRMerkleBlockNew();
		}

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
			assert(_merkleBlock != NULL);

			// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _merkleBlock->target >> 24, target = _merkleBlock->target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = BRMerkleBlockRootR(_merkleBlock, &hashIdx, &flagIdx, 0), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_merkleBlock->totalTx > 0 && !UInt256Eq(&(merkleRoot), &(_merkleBlock->merkleRoot))) r = 0;

			// check if timestamp is too far in future
			if (_merkleBlock->timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;

			// check if proof-of-work target is out of range
			//fixme check pow later
			//if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget)) r = 0;

			if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
			else UInt32SetLE(t.u8, target >> (3 - size) * 8);

			for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
				if (_merkleBlock->blockHash.u8[i] < t.u8[i]) break;
				if (_merkleBlock->blockHash.u8[i] > t.u8[i]) r = 0;
			}

			return r;
		}

		bool MerkleBlock::containsTransactionHash(UInt256 hash) const {
			return BRMerkleBlockContainsTxHash(_merkleBlock, hash) != 0;
		}

		void MerkleBlock::Serialize(ByteStream &ostream) const {
			uint8_t versionData[32 / 8];
			UInt32SetLE(versionData, _merkleBlock->version);
			ostream.putBytes(versionData, 32 / 8);

			uint8_t prevBlockData[256 / 8];
			UInt256Set(prevBlockData, _merkleBlock->prevBlock);
			ostream.putBytes(prevBlockData, 256 / 8);

			uint8_t merkleRootData[256 / 8];
			UInt256Set(merkleRootData, _merkleBlock->merkleRoot);
			ostream.putBytes(merkleRootData, 256 / 8);

			uint8_t timeStampData[32 / 8];
			UInt32SetLE(timeStampData, _merkleBlock->timestamp);
			ostream.putBytes(timeStampData, 32 / 8);

			uint8_t bitsData[32 / 8];
			UInt32SetLE(bitsData, _merkleBlock->target);
			ostream.putBytes(bitsData, 32 / 8);

			uint8_t nonceData[32 / 8];
			UInt32SetLE(nonceData, _merkleBlock->nonce);
			ostream.putBytes(nonceData, 32 / 8);

			uint8_t heightData[32 / 8];
			UInt32SetLE(heightData, _merkleBlock->height);
			ostream.putBytes(heightData, 32 / 8);

			_auxPow.Serialize(ostream);

			ostream.put(1);    //correspond to serialization of node, should add one byte here

			uint8_t totalTxData[32 / 8];
			UInt32SetLE(totalTxData, _merkleBlock->totalTx);
			ostream.putBytes(totalTxData, 32 / 8);

			uint8_t hashesCountData[32 / 8];
			UInt32SetLE(hashesCountData, uint32_t(_merkleBlock->hashesCount));
			ostream.putBytes(hashesCountData, 32 / 8);

			uint8_t hashData[256 / 8];
			for (uint32_t i = 0; i < _merkleBlock->hashesCount; ++i) {
				UInt256Set(hashData, _merkleBlock->hashes[i]);
				ostream.putBytes(hashData, 256 / 8);
			}

			assert(_merkleBlock->hashesCount == _merkleBlock->flagsLen);
			ostream.putVarUint(_merkleBlock->flagsLen);
			for (uint32_t i = 0; i < _merkleBlock->hashesCount; ++i) {
				ostream.put(_merkleBlock->flags[i]);
			}
		}

		void MerkleBlock::Deserialize(ByteStream &istream) {
			uint8_t versionData[32 / 8];
			istream.getBytes(versionData, 32 / 8);
			_merkleBlock->version = UInt32GetLE(versionData);

			uint8_t prevBlockData[256 / 8];
			istream.getBytes(prevBlockData, 256 / 8);
			UInt256Get(&_merkleBlock->prevBlock, prevBlockData);

			uint8_t merkleRootData[256 / 8];
			istream.getBytes(merkleRootData, 256 / 8);
			UInt256Get(&_merkleBlock->merkleRoot, merkleRootData);

			uint8_t timeStampData[32 / 8];
			istream.getBytes(timeStampData, 32 / 8);
			_merkleBlock->timestamp = UInt32GetLE(timeStampData);

			uint8_t bitsData[32 / 8];
			istream.getBytes(bitsData, 32 / 8);
			_merkleBlock->target = UInt32GetLE(bitsData);

			uint8_t nonceData[32 / 8];
			istream.getBytes(nonceData, 32 / 8);
			_merkleBlock->nonce = UInt32GetLE(nonceData);

			uint8_t heightData[32 / 8];
			istream.getBytes(heightData, 32 / 8);
			_merkleBlock->height = UInt32GetLE(heightData);

			_auxPow.Deserialize(istream);

			istream.get();    //correspond to serialization of node, should get one byte here

			uint8_t totalTxData[32 / 8];
			istream.getBytes(totalTxData, 32 / 8);
			_merkleBlock->totalTx = UInt32GetLE(totalTxData);

			uint8_t hashesCountData[32 / 8];
			istream.getBytes(hashesCountData, 32 / 8);
			_merkleBlock->hashesCount = UInt32GetLE(hashesCountData);

			_merkleBlock->hashes = (UInt256 *) calloc(_merkleBlock->hashesCount, sizeof(UInt256));
			uint8_t hashData[256 / 8];
			for (uint32_t i = 0; i < _merkleBlock->hashesCount; ++i) {
				istream.getBytes(hashData, 256 / 8);
				UInt256Get(&_merkleBlock->hashes[i], hashData);
			}

			_merkleBlock->flagsLen = istream.getVarUint();
			_merkleBlock->flags = (_merkleBlock->flagsLen > 0) ? (uint8_t *)malloc(_merkleBlock->flagsLen) : NULL;
			assert(_merkleBlock->hashesCount == _merkleBlock->flagsLen);
			for (uint32_t i = 0; i < _merkleBlock->flagsLen; ++i) {
				_merkleBlock->flags[i] = istream.get();
			}
		}

		size_t MerkleBlock::getRawBlockTxHashes(UInt256 *txHashes, size_t hashesCount) {
			size_t idx = 0, hashIdx = 0, flagIdx = 0;

			assert(_merkleBlock != NULL);

			return BRMerkleBlockTxHashesR(_merkleBlock, txHashes, (txHashes) ? hashesCount : SIZE_MAX, &idx, &hashIdx, &flagIdx, 0);
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
