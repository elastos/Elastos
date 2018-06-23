// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <Core/BRMerkleBlock.h>

#include "MerkleBlock.h"
#include "SidechainMerkleBlock.h"

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty target

namespace Elastos {
	namespace ElaWallet {

		SidechainMerkleBlock::SidechainMerkleBlock() {
			_merkleBlock = IdMerkleBlockNew();
		}

		SidechainMerkleBlock::SidechainMerkleBlock(Elastos::ElaWallet::IdMerkleBlock *merkleBlock) :
			_merkleBlock(merkleBlock) {
		}

		SidechainMerkleBlock::~SidechainMerkleBlock() {
			if (_merkleBlock != nullptr)
				IdMerkleBlockFree(_merkleBlock);
		}

		std::string SidechainMerkleBlock::toString() const {
			//todo complete me
			return "";
		}

		BRMerkleBlock *SidechainMerkleBlock::getRaw() const {
			return (BRMerkleBlock *)_merkleBlock;
		}

		IMerkleBlock *SidechainMerkleBlock::Clone() const {
			return new SidechainMerkleBlock(IdMerkleBlockCopy(_merkleBlock));
		}

		void SidechainMerkleBlock::Serialize(ByteStream &ostream) const {
			//todo complete me
		}

		bool SidechainMerkleBlock::Deserialize(ByteStream &istream) {
			//todo complete me
			return false;
		}

		nlohmann::json SidechainMerkleBlock::toJson() {
			//todo complete me
			return nlohmann::json();
		}

		void SidechainMerkleBlock::fromJson(const nlohmann::json &) {
			//todo complete me
		}

		BRMerkleBlock *SidechainMerkleBlock::getRawBlock() const {
			return getRaw();
		}

		void SidechainMerkleBlock::initFromRaw(BRMerkleBlock *block) {
			_merkleBlock = (IdMerkleBlock *)block;
		}

		UInt256 SidechainMerkleBlock::getBlockHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_merkleBlock->raw.blockHash, &zero)) {
				ByteStream ostream;
				MerkleBlock::serializeNoAux(ostream, _merkleBlock->raw);
				UInt256 hash = UINT256_ZERO;
				CMBlock buf = ostream.getBuffer();
				BRSHA256_2(&hash, buf, buf.GetSize());
				UInt256Set(&_merkleBlock->raw.blockHash, hash);
			}
			return _merkleBlock->raw.blockHash;
		}

		uint32_t SidechainMerkleBlock::getHeight() const {
			return _merkleBlock->raw.height;
		}

		void SidechainMerkleBlock::setHeight(uint32_t height) {
			_merkleBlock->raw.height = height;
		}

		bool SidechainMerkleBlock::isValid(uint32_t currentTime) const {
			// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _merkleBlock->raw.target >> 24, target = _merkleBlock->raw.target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = MerkleBlock::MerkleBlockRootR(&hashIdx, &flagIdx, 0, _merkleBlock->raw), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_merkleBlock->raw.totalTx > 0 && ! UInt256Eq(&(merkleRoot), &(_merkleBlock->raw.merkleRoot))) r = 0;

			// check if timestamp is too far in future
			if (_merkleBlock->raw.timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;

			// check if proof-of-work target is out of range
			if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget)) r = 0;

			if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
			else UInt32SetLE(t.u8, target >> (3 - size)*8);

			//todo verify block hash
//			UInt256 auxBlockHash = _merkleBlock->auxPow.getParBlockHeaderHash();
//			for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
//				if (auxBlockHash.u8[i] < t.u8[i]) break;
//				if (auxBlockHash.u8[i] > t.u8[i]) r = 0;
//			}

			return r;
		}

		std::string SidechainMerkleBlock::getBlockType() const {
			return "SideStandard";
		}

		REGISTER_MERKLEBLOCKPLUGIN(SidechainMerkleBlock);
	}
}