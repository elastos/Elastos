// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlock.h"

#include <Plugin/Registry.h>
#include <Common/hash.h>

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
			_blockHash = other._blockHash;
			_version = other._version;
			_prevBlock = other._prevBlock;
			_merkleRoot = other._merkleRoot;
			_timestamp = other._timestamp;
			_target = other._target;
			_nonce = other._nonce;
			_totalTx = other._totalTx;
			SetHashes(other._hashes);
			_flags = other._flags;
			_height = other._height;
			_auxPow = other._auxPow;

			return *this;
		}

		const uint256 &MerkleBlock::GetHash() const {
			if (_blockHash == 0) {
				ByteStream ostream;
				MerkleBlockBase::SerializeNoAux(ostream);
				_blockHash = sha256_2(ostream.GetBytes());
			}
			return _blockHash;
		}


		// true if merkle tree and _timestamp are valid, and proof-of-work matches the stated difficulty _target
		// NOTE: this only checks if the block difficulty matches the difficulty _target in the header, it does not check if the
		// _target is correct for the block's _height in the chain - use BRMerkleBlockVerifyDifficulty() for that
		bool MerkleBlock::IsValid(uint32_t currentTime) const {
			// _target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
			// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
			static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
			const uint32_t size = _target >> 24, target = _target & 0x00ffffff;
			size_t hashIdx = 0, flagIdx = 0;
			uint256 merkleRoot = MerkleBlockRootR(&hashIdx, &flagIdx, 0), t;
			int r = 1;

			// check if merkle root is correct
			if (_totalTx > 0 && merkleRoot != _merkleRoot)
				r = 0;

			// check if _timestamp is too far in future
			if (_timestamp > currentTime + BLOCK_MAX_TIME_DRIFT)
				r = 0;

			// check if proof-of-work _target is out of range
			if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget))
				r = 0;

			if (size > 3)
				*(uint32_t *)(t.begin() + size - 3) = target;
			else
				*(uint32_t *)t.begin() = target >> (3 - size) * 8;

			uint256 auxBlockHash = _auxPow.GetParBlockHeaderHash();
			for (int i = t.size() - 1; r && i >= 0; i--) { // check proof-of-work
				if (auxBlockHash.begin()[i] < t.begin()[i])
					break;
				if (auxBlockHash.begin()[i] > t.begin()[i])
					r = 0;
			}

			return r;
		}

		void MerkleBlock::Serialize(ByteStream &ostream, int version) const {
			MerkleBlockBase::SerializeNoAux(ostream);
			_auxPow.Serialize(ostream);
			MerkleBlockBase::SerializeAfterAux(ostream);
		}

		bool MerkleBlock::Deserialize(const ByteStream &istream, int version) {
			if (!MerkleBlockBase::DeserializeNoAux(istream) || !_auxPow.Deserialize(istream) ||
				!MerkleBlockBase::DeserializeAfterAux(istream))
				return false;

			GetHash();
			return true;
		}

		const AuxPow &MerkleBlock::GetAuxPow() const {
			return _auxPow;
		}

		void MerkleBlock::SetAuxPow(const AuxPow &pow) {
			_auxPow = pow;
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
