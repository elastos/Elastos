// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockBase.h"

#include <Common/ByteStream.h>
#include <Common/Utils.h>
#include <Common/hash.h>

namespace Elastos {
	namespace ElaWallet {

		namespace {
			namespace {

				inline static int _ceil_log2(int x) {
					int r = (x & (x - 1)) ? 1 : 0;

					while ((x >>= 1) != 0) r++;
					return r;
				}
			}
		}

		MerkleBlockBase::MerkleBlockBase() :
				_version(0),
				_timestamp(0),
				_target(0),
				_nonce(0),
				_totalTx(0),
				_height(0) {
		}

		MerkleBlockBase::~MerkleBlockBase() {

		}

		uint32_t MerkleBlockBase::GetTotalTx() const {
			return _totalTx;
		}

		uint32_t MerkleBlockBase::GetVersion() const {
			return _version;
		}

		void MerkleBlockBase::SetVersion(uint32_t version) {
			_version = version;
		}

		const uint256 &MerkleBlockBase::GetPrevBlockHash() const {
			return _prevBlock;
		}

		const uint256 &MerkleBlockBase::GetRootBlockHash() const {
			return _merkleRoot;
		}

		uint32_t MerkleBlockBase::GetTimestamp() const {
			return _timestamp;
		}

		uint32_t MerkleBlockBase::GetTarget() const {
			return _target;
		}

		uint32_t MerkleBlockBase::GetNonce() const {
			return _nonce;
		}

		uint32_t MerkleBlockBase::GetTransactionCount() const {
			return _totalTx;
		}

		uint32_t MerkleBlockBase::GetHeight() const {
			return _height;
		}

		void MerkleBlockBase::SetHeight(uint32_t height) {
			_height = height;
		}

		void MerkleBlockBase::SetTimestamp(uint32_t timestamp) {
			_timestamp = timestamp;
		}

		void MerkleBlockBase::SetPrevBlockHash(const uint256 &hash) {
			_prevBlock = hash;
		}

		void MerkleBlockBase::SetRootBlockHash(const uint256 &hash) {
			_merkleRoot = hash;
		}

		void MerkleBlockBase::SetTarget(uint32_t target) {
			_target = target;
		}

		void MerkleBlockBase::SetNonce(uint32_t nonce) {
			_nonce = nonce;
		}

		void MerkleBlockBase::SetTransactionCount(uint32_t count) {
			_totalTx = count;
		}

		const std::vector<uint256> &MerkleBlockBase::GetHashes() const {
			return _hashes;
		}

		void MerkleBlockBase::SetHashes(const std::vector<uint256> &hashes) {
			_hashes = hashes;
		}

		const std::vector<uint8_t> &MerkleBlockBase::GetFlags() const {
			return _flags;
		}

		void MerkleBlockBase::SetFlags(const std::vector<uint8_t> &flags) {
			_flags = flags;
		}

		void MerkleBlockBase::SerializeNoAux(ByteStream &ostream) const {
			ostream.WriteUint32(_version);
			ostream.WriteBytes(_prevBlock);
			ostream.WriteBytes(_merkleRoot);
			ostream.WriteUint32(_timestamp);
			ostream.WriteUint32(_target);
			ostream.WriteUint32(_nonce);
			ostream.WriteUint32(_height);
		}

		bool MerkleBlockBase::DeserializeNoAux(const ByteStream &istream) {
			if (!istream.ReadUint32(_version))
				return false;

			if (!istream.ReadBytes(_prevBlock))
				return false;

			if (!istream.ReadBytes(_merkleRoot))
				return false;

			if (!istream.ReadUint32(_timestamp))
				return false;

			if (!istream.ReadUint32(_target))
				return false;

			if (!istream.ReadUint32(_nonce))
				return false;

			if (!istream.ReadUint32(_height))
				return false;

			return true;
		}

		void MerkleBlockBase::SerializeAfterAux(ByteStream &ostream) const {
			ostream.WriteUint8(1);    //correspond to serialization of node, should add one byte here

			ostream.WriteUint32(_totalTx);

			ostream.WriteUint32((uint32_t) _hashes.size());
			for (size_t i = 0; i < _hashes.size(); ++i) {
				ostream.WriteBytes(_hashes[i]);
			}

			ostream.WriteVarBytes(_flags);
		}

		bool MerkleBlockBase::DeserializeAfterAux(const ByteStream &istream) {
			istream.Skip(1);    //correspond to serialization of node, should get one byte here

			if (!istream.ReadUint32(_totalTx))
				return false;

			uint32_t hashesCount = 0;
			if (!istream.ReadUint32(hashesCount))
				return false;

			for (size_t i = 0; i < hashesCount; ++i) {
				uint256 hash;
				if (!istream.ReadBytes(hash))
					return false;
				_hashes.push_back(hash);
			}

			if (!istream.ReadVarBytes(_flags))
				return false;

			return true;
		}

		size_t MerkleBlockBase::MerkleBlockTxHashes(std::vector<uint256> &txHashes) const {
			size_t hashIdx = 0, flagIdx = 0;

			return MerkleBlockTxHashesR(txHashes, hashIdx, flagIdx, 0);
		}

		size_t MerkleBlockBase::MerkleBlockTxHashesR(std::vector<uint256> &txHashes, size_t &hashIdx, size_t &flagIdx,
													 int depth) const {
			uint8_t flag;

			if (flagIdx / 8 < _flags.size() && hashIdx < _hashes.size()) {
				flag = (_flags[flagIdx / 8] & (1 << (flagIdx % 8)));
				flagIdx++;

				if (! flag || depth == ceilLog2(_totalTx)) {
					if (flag) {
						txHashes.push_back(_hashes[hashIdx]); // leaf
					}

					hashIdx++;
				} else {
					MerkleBlockTxHashesR(txHashes, hashIdx, flagIdx, depth + 1); // left branch
					MerkleBlockTxHashesR(txHashes, hashIdx, flagIdx, depth + 1); // right branch
				}
			}

			return txHashes.size();
		}

		int MerkleBlockBase::ceilLog2(int x) const {
			int r = (x & (x - 1)) ? 1 : 0;

			while ((x >>= 1) != 0) r++;
			return r;
		}

		// recursively walks the merkle tree to calculate the merkle root
		// NOTE: this merkle tree design has a security vulnerability (CVE-2012-2459), which can be defended against by
		// considering the merkle root invalid if there are duplicate hashes in any rows with an even number of elements
		uint256 MerkleBlockBase::MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const {
			uint8_t flag;
			uint256 hashes[2], md;

			if (*flagIdx / 8 < _flags.size() && *hashIdx < _hashes.size()) {
				flag = (_flags[*flagIdx / 8] & (1 << (*flagIdx % 8)));
				(*flagIdx)++;

				if (flag && depth != _ceil_log2(_totalTx)) {
					hashes[0] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // left branch
					hashes[1] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // right branch

					if (hashes[0] != 0 && hashes[0] != hashes[1]) {
						if (hashes[1] == 0)
							hashes[1] = hashes[0]; // if right branch is missing, dup left branch

						bytes_t data(hashes[0].begin(), hashes[0].size());
						data += bytes_t(hashes[1].begin(), hashes[1].size());
						md = sha256_2(data);
					} else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
				} else md = _hashes[(*hashIdx)++]; // leaf
			}

			return md;
		}

		void MerkleBlockBase::SetHash(const uint256 &hash) {
			_blockHash = hash;
		}

		bool MerkleBlockBase::IsEqual(const IMerkleBlock *block) const {
			return (block == this || GetHash() == block->GetHash());
		}
	}
}
