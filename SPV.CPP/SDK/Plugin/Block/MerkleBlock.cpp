// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRMerkleBlock.h>
#include <Core/BRMerkleBlock.h>
#include <SDK/Common/Log.h>
#include "BRCrypto.h"
#include "BRMerkleBlock.h"
#include "Utils.h"

#include "MerkleBlock.h"
#include "Plugin/Registry.h"

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty _target

namespace Elastos {
	namespace ElaWallet {

		MerkleBlock::MerkleBlock() {

		}

		MerkleBlock::MerkleBlock(const MerkleBlock &merkleBlock) {
			operator=(merkleBlock);
		}

		MerkleBlock::~MerkleBlock() {
		}

		MerkleBlock &MerkleBlock::operator=(const MerkleBlock &other) {
			UInt256Set(&_blockHash, other._blockHash);
			_version = other._version;
			UInt256Set(&_prevBlock, other._prevBlock);
			UInt256Set(&_merkleRoot, other._merkleRoot);
			_timestamp = other._timestamp;
			_target = other._target;
			_nonce = other._nonce;
			_totalTx = other._totalTx;
			setHashes(other._hashes);
			_flags = other._flags;
			_height = other._height;
			_auxPow = other._auxPow;

			return *this;
		}

		const UInt256 &MerkleBlock::getHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_blockHash, &zero)) {
				ByteStream ostream;
				MerkleBlockBase::serializeNoAux(ostream);
				UInt256 hash = UINT256_ZERO;
				CMBlock buf = ostream.getBuffer();
				BRSHA256_2(&hash, buf, buf.GetSize());
				UInt256Set((void *) &_blockHash, hash);
			}
			return _blockHash;
		}


		// true if merkle tree and _timestamp are valid, and proof-of-work matches the stated difficulty _target
		// NOTE: this only checks if the block difficulty matches the difficulty _target in the header, it does not check if the
		// _target is correct for the block's _height in the chain - use BRMerkleBlockVerifyDifficulty() for that
		bool MerkleBlock::isValid(uint32_t currentTime) const {
			// _target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _target >> 24, target = _target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			UInt256 merkleRoot = MerkleBlockRootR(&hashIdx, &flagIdx, 0), t = UINT256_ZERO;
			int r = 1;

			// check if merkle root is correct
			if (_totalTx > 0 && !UInt256Eq(&merkleRoot, &_merkleRoot))
				r = 0;

			// check if _timestamp is too far in future
			if (_timestamp > currentTime + BLOCK_MAX_TIME_DRIFT)
				r = 0;

			// check if proof-of-work _target is out of range
			if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget))
				r = 0;

			if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
			else UInt32SetLE(t.u8, target >> (3 - size) * 8);

			UInt256 auxBlockHash = _auxPow.getParBlockHeaderHash();
			for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
				if (auxBlockHash.u8[i] < t.u8[i])
					break;
				if (auxBlockHash.u8[i] > t.u8[i])
					r = 0;
			}

			return r;
		}

		void MerkleBlock::Serialize(ByteStream &ostream) const {
			MerkleBlockBase::serializeNoAux(ostream);
			_auxPow.Serialize(ostream);
			MerkleBlockBase::serializeAfterAux(ostream);
		}

		bool MerkleBlock::Deserialize(ByteStream &istream) {
			if (!MerkleBlockBase::deserializeNoAux(istream) || !_auxPow.Deserialize(istream) ||
				!MerkleBlockBase::deserializeAfterAux(istream))
				return false;

			getHash();
			return true;
		}

		const AuxPow &MerkleBlock::getAuxPow() const {
			return _auxPow;
		}

		void MerkleBlock::setAuxPow(const AuxPow &pow) {
			_auxPow = pow;
		}

		nlohmann::json MerkleBlock::toJson() const {
			return MerkleBlockBase::toJson();
		}

		void MerkleBlock::fromJson(const nlohmann::json &j) {
			MerkleBlockBase::fromJson(j);
		}

		MerkleBlockPtr MerkleBlockFactory::createBlock() {
			return MerkleBlockPtr(new MerkleBlock);
		}

		fruit::Component<IMerkleBlockFactory> getMerkleBlockFactoryComponent() {
			return fruit::createComponent()
					.bind<IMerkleBlockFactory, MerkleBlockFactory>();
		}
	}
}
