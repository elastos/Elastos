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
			namespace {

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty target

				inline static int _ceil_log2(int x)
				{
					int r = (x & (x - 1)) ? 1 : 0;

					while ((x >>= 1) != 0) r++;
					return r;
				}
			}
		}

		MerkleBlock::MerkleBlock() {
			_merkleBlock = ELAMerkleBlockNew();
		}

		MerkleBlock::MerkleBlock(ELAMerkleBlock *merkleBlock) :
				_merkleBlock(merkleBlock) {
		}

		MerkleBlock::~MerkleBlock() {
			if (_merkleBlock != nullptr)
				ELAMerkleBlockFree(_merkleBlock);
		}

		std::string MerkleBlock::toString() const {
			//todo complete me
			return "";
		}

		BRMerkleBlock *MerkleBlock::getRaw() const {
			return (BRMerkleBlock *) _merkleBlock;
		}

		UInt256 MerkleBlock::getBlockHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_merkleBlock->raw.blockHash, &zero)) {
				ByteStream ostream;
				serializeNoAux(ostream);
				UInt256 hash = UINT256_ZERO;
				BRSHA256_2(&hash, ostream.getBuf(), ostream.position());
				UInt256Set(&_merkleBlock->raw.blockHash, hash);
			}
			return _merkleBlock->raw.blockHash;
		}

		uint32_t MerkleBlock::getVersion() const {
			return _merkleBlock->raw.version;
		}

		UInt256 MerkleBlock::getPrevBlockHash() const {
			return _merkleBlock->raw.prevBlock;
		}

		UInt256 MerkleBlock::getRootBlockHash() const {
			return _merkleBlock->raw.merkleRoot;
		}

		uint32_t MerkleBlock::getTimestamp() const {
			return _merkleBlock->raw.timestamp;
		}

		uint32_t MerkleBlock::getTarget() const {
			return _merkleBlock->raw.target;
		}

		uint32_t MerkleBlock::getNonce() const {
			return _merkleBlock->raw.nonce;
		}

		uint32_t MerkleBlock::getTransactionCount() const {
			return _merkleBlock->raw.totalTx;
		}

		uint32_t MerkleBlock::getHeight() const {
			return _merkleBlock->raw.height;
		}

		// true if merkle tree and timestamp are valid, and proof-of-work matches the stated difficulty target
		// NOTE: this only checks if the block difficulty matches the difficulty target in the header, it does not check if the
		// target is correct for the block's height in the chain - use BRMerkleBlockVerifyDifficulty() for that
		bool MerkleBlock::isValid(uint32_t currentTime) const {
			// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _merkleBlock->raw.target >> 24, target = _merkleBlock->raw.target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = MerkleBlockRootR(&hashIdx, &flagIdx, 0), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_merkleBlock->raw.totalTx > 0 && ! UInt256Eq(&(merkleRoot), &(_merkleBlock->raw.merkleRoot))) r = 0;

			// check if timestamp is too far in future
			if (_merkleBlock->raw.timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;

			// check if proof-of-work target is out of range
			if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget)) r = 0;

			if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
			else UInt32SetLE(t.u8, target >> (3 - size)*8);

			UInt256 auxBlockHash = _merkleBlock->auxPow.getParBlockHeaderHash();
			for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
				if (auxBlockHash.u8[i] < t.u8[i]) break;
				if (auxBlockHash.u8[i] > t.u8[i]) r = 0;
			}

			return r;
		}

		bool MerkleBlock::containsTransactionHash(UInt256 hash) const {
			return BRMerkleBlockContainsTxHash(&_merkleBlock->raw, hash) != 0;
		}

		void MerkleBlock::Serialize(ByteStream &ostream) const {
			serializeNoAux(ostream);

			_merkleBlock->auxPow.Serialize(ostream);

			ostream.put(1);    //correspond to serialization of node, should add one byte here

			uint8_t totalTxData[32 / 8];
			UInt32SetLE(totalTxData, _merkleBlock->raw.totalTx);
			ostream.putBytes(totalTxData, 32 / 8);

			uint8_t hashesCountData[32 / 8];
			UInt32SetLE(hashesCountData, uint32_t(_merkleBlock->raw.hashesCount));
			ostream.putBytes(hashesCountData, 32 / 8);

			uint8_t hashData[256 / 8];
			for (uint32_t i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				UInt256Set(hashData, _merkleBlock->raw.hashes[i]);
				ostream.putBytes(hashData, 256 / 8);
			}

			ostream.putVarUint(_merkleBlock->raw.flagsLen);
			for (uint32_t i = 0; i < _merkleBlock->raw.flagsLen; ++i) {
				ostream.put(_merkleBlock->raw.flags[i]);
			}
		}

		void MerkleBlock::serializeNoAux(ByteStream &ostream) const {
			uint8_t versionData[32 / 8];
			UInt32SetLE(versionData, _merkleBlock->raw.version);
			ostream.putBytes(versionData, 32 / 8);

			uint8_t prevBlockData[256 / 8];
			UInt256Set(prevBlockData, _merkleBlock->raw.prevBlock);
			ostream.putBytes(prevBlockData, 256 / 8);

			uint8_t merkleRootData[256 / 8];
			UInt256Set(merkleRootData, _merkleBlock->raw.merkleRoot);
			ostream.putBytes(merkleRootData, 256 / 8);

			uint8_t timeStampData[32 / 8];
			UInt32SetLE(timeStampData, _merkleBlock->raw.timestamp);
			ostream.putBytes(timeStampData, 32 / 8);

			uint8_t bitsData[32 / 8];
			UInt32SetLE(bitsData, _merkleBlock->raw.target);
			ostream.putBytes(bitsData, 32 / 8);

			uint8_t nonceData[32 / 8];
			UInt32SetLE(nonceData, _merkleBlock->raw.nonce);
			ostream.putBytes(nonceData, 32 / 8);

			uint8_t heightData[32 / 8];
			UInt32SetLE(heightData, _merkleBlock->raw.height);
			ostream.putBytes(heightData, 32 / 8);
		}

		bool MerkleBlock::Deserialize(ByteStream &istream) {
			uint8_t versionData[32 / 8];
			istream.getBytes(versionData, 32 / 8);
			_merkleBlock->raw.version = UInt32GetLE(versionData);

			uint8_t prevBlockData[256 / 8];
			istream.getBytes(prevBlockData, 256 / 8);
			UInt256Get(&_merkleBlock->raw.prevBlock, prevBlockData);

			uint8_t merkleRootData[256 / 8];
			istream.getBytes(merkleRootData, 256 / 8);
			UInt256Get(&_merkleBlock->raw.merkleRoot, merkleRootData);

			uint8_t timeStampData[32 / 8];
			istream.getBytes(timeStampData, 32 / 8);
			_merkleBlock->raw.timestamp = UInt32GetLE(timeStampData);

			uint8_t bitsData[32 / 8];
			istream.getBytes(bitsData, 32 / 8);
			_merkleBlock->raw.target = UInt32GetLE(bitsData);

			uint8_t nonceData[32 / 8];
			istream.getBytes(nonceData, 32 / 8);
			_merkleBlock->raw.nonce = UInt32GetLE(nonceData);

			uint8_t heightData[32 / 8];
			istream.getBytes(heightData, 32 / 8);
			_merkleBlock->raw.height = UInt32GetLE(heightData);

			_merkleBlock->auxPow.Deserialize(istream);

			istream.get();    //correspond to serialization of node, should get one byte here

			uint8_t totalTxData[32 / 8];
			istream.getBytes(totalTxData, 32 / 8);
			_merkleBlock->raw.totalTx = UInt32GetLE(totalTxData);

			uint8_t hashesCountData[32 / 8];
			istream.getBytes(hashesCountData, 32 / 8);
			_merkleBlock->raw.hashesCount = UInt32GetLE(hashesCountData);

			_merkleBlock->raw.hashes = (UInt256 *) calloc(_merkleBlock->raw.hashesCount, sizeof(UInt256));
			uint8_t hashData[256 / 8];
			for (uint32_t i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				istream.getBytes(hashData, 256 / 8);
				UInt256Get(&_merkleBlock->raw.hashes[i], hashData);
			}

			_merkleBlock->raw.flagsLen = istream.getVarUint();
			_merkleBlock->raw.flags = (_merkleBlock->raw.flagsLen > 0) ? (uint8_t *) malloc(_merkleBlock->raw.flagsLen)
																	   : NULL;
			for (uint32_t i = 0; i < _merkleBlock->raw.flagsLen; ++i) {
				_merkleBlock->raw.flags[i] = istream.get();
			}

			return true;
		}

		const AuxPow &MerkleBlock::getAuxPow() const {
			return _merkleBlock->auxPow;
		}

		void MerkleBlock::setHeight(uint32_t height) {
			_merkleBlock->raw.height = height;
		}

		// recursively walks the merkle tree to calculate the merkle root
		// NOTE: this merkle tree design has a security vulnerability (CVE-2012-2459), which can be defended against by
		// considering the merkle root invalid if there are duplicate hashes in any rows with an even number of elements
		UInt256 MerkleBlock::MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const {
			uint8_t flag;
			UInt256 hashes[2], md = UINT256_ZERO;

			if (*flagIdx/8 < _merkleBlock->raw.flagsLen && *hashIdx < _merkleBlock->raw.hashesCount) {
				flag = (_merkleBlock->raw.flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
				(*flagIdx)++;

				if (flag && depth != _ceil_log2(_merkleBlock->raw.totalTx)) {
					hashes[0] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // left branch
					hashes[1] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // right branch

					if (! UInt256IsZero(&hashes[0]) && ! UInt256Eq(&(hashes[0]), &(hashes[1]))) {
						if (UInt256IsZero(&hashes[1])) hashes[1] = hashes[0]; // if right branch is missing, dup left branch
						BRSHA256_2(&md, hashes, sizeof(hashes));
					}
					else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
				}
				else md = _merkleBlock->raw.hashes[(*hashIdx)++]; // leaf
			}

			return md;
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
			ELAMerkleBlock *pblock = (ELAMerkleBlock *) p.getRaw();
			if (pblock == nullptr) {
				pblock = ELAMerkleBlockNew();
				p = MerkleBlock(pblock);
			}

			BRMerkleBlock *block = (BRMerkleBlock *) pblock;
			block->blockHash = Utils::UInt256FromString(j["blockHash"].get<std::string>());
			block->version = j["version"].get<uint32_t>();
			block->prevBlock = Utils::UInt256FromString(j["prevBlock"].get<std::string>());
			block->merkleRoot = Utils::UInt256FromString(j["merkleRoot"].get<std::string>());
			block->timestamp = j["timestamp"].get<uint32_t>();
			block->target = j["target"].get<uint32_t>();
			block->nonce = j["nonce"].get<uint32_t>();
			block->totalTx = j["totalTx"].get<uint32_t>();

			if (block->hashes != nullptr) {
				free(block->hashes);
				block->hashes = nullptr;
			}

			std::vector<std::string> hashes = j["hashes"].get<std::vector<std::string>>();
			block->hashesCount = hashes.size();
			block->hashes = (block->hashesCount > 0) ? (UInt256 *) malloc(sizeof(UInt256) * block->hashesCount)
													 : nullptr;
			for (int i = 0; i < block->hashesCount; ++i) {
				UInt256 hash = Utils::UInt256FromString(hashes[i]);
				memcpy(&block->hashes[i], &hash, sizeof(hash));
			}

			if (block->flags != nullptr) {
				free(block->flags);
				block->flags = nullptr;
			}

			std::vector<uint8_t> flags = j["flags"].get<std::vector<uint8_t>>();
			block->flagsLen = flags.size();
			block->flags = (block->flagsLen > 0) ? (uint8_t *) malloc(block->flagsLen) : nullptr;
			for (int i = 0; i < block->flagsLen; ++i) {
				block->flags[i] = flags[i];
			}

			block->height = j["height"].get<uint32_t>();
		}

	}
}
