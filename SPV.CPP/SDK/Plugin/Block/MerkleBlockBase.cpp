// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockBase.h"

#include <Core/BRCrypto.h>

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Utils.h>

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
				_blockHash(UINT256_ZERO),
				_version(0),
				_prevBlock(UINT256_ZERO),
				_merkleRoot(UINT256_ZERO),
				_timestamp(0),
				_target(0),
				_nonce(0),
				_totalTx(0),
				_height(0) {
		}

		MerkleBlockBase::~MerkleBlockBase() {

		}

		uint32_t MerkleBlockBase::GetVersion() const {
			return _version;
		}

		void MerkleBlockBase::SetVersion(uint32_t version) {
			_version = version;
		}

		const UInt256 &MerkleBlockBase::GetPrevBlockHash() const {
			return _prevBlock;
		}

		const UInt256 &MerkleBlockBase::GetRootBlockHash() const {
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

		nlohmann::json MerkleBlockBase::ToJson() const {
			nlohmann::json j;

			std::vector<std::string> hashes;
			for (int i = 0; i < _hashes.size(); ++i) {
				hashes.push_back(Utils::UInt256ToString(_hashes[i], true));
			}

			std::vector<uint8_t> flags;
			for (int i = 0; i < _flags.size(); ++i) {
				flags.push_back(_flags[i]);
			}

			j["BlockHash"] = Utils::UInt256ToString(_blockHash, true);
			j["Version"] = _version;
			j["PrevBlock"] = Utils::UInt256ToString(_prevBlock, true);
			j["MerkleRoot"] = Utils::UInt256ToString(_merkleRoot, true);
			j["Timestamp"] = _timestamp;
			j["Target"] = _target;
			j["Nonce"] = _nonce;
			j["TotalTx"] = _totalTx;
			j["Hashes"] = hashes;
			j["Flags"] = flags;
			j["Height"] = _height;

//			j["AuxPow"] = _merkleBlock->_auxPow.toJson();

			return j;
		}

		void MerkleBlockBase::FromJson(const nlohmann::json &j) {

			_blockHash = Utils::UInt256FromString(j["BlockHash"].get<std::string>(), true);
			_version = j["Version"].get<uint32_t>();
			_prevBlock = Utils::UInt256FromString(j["PrevBlock"].get<std::string>(), true);
			_merkleRoot = Utils::UInt256FromString(j["MerkleRoot"].get<std::string>(), true);
			_timestamp = j["Timestamp"].get<uint32_t>();
			_target = j["Target"].get<uint32_t>();
			_nonce = j["Nonce"].get<uint32_t>();
			_totalTx = j["TotalTx"].get<uint32_t>();

			_hashes.clear();
			std::vector<std::string> hashes = j["Hashes"].get<std::vector<std::string>>();
			for (int i = 0; i < hashes.size(); ++i) {
				_hashes.push_back(Utils::UInt256FromString(hashes[i], true));
			}

			_flags.clear();
			std::vector<uint8_t> flags = j["Flags"].get<std::vector<uint8_t>>();
			for (int i = 0; i < flags.size(); ++i) {
				_flags.push_back(flags[i]);
			}

			_height = j["Height"].get<uint32_t>();
		}

		void MerkleBlockBase::SetTimestamp(uint32_t timestamp) {
			_timestamp = timestamp;
		}

		void MerkleBlockBase::SetPrevBlockHash(const UInt256 &hash) {
			UInt256Set(&_prevBlock, hash);
		}

		void MerkleBlockBase::SetRootBlockHash(const UInt256 &hash) {
			UInt256Set(&_merkleRoot, hash);
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

		const std::vector<UInt256> &MerkleBlockBase::GetHashes() const {
			return _hashes;
		}

		void MerkleBlockBase::SetHashes(const std::vector<UInt256> &hashes) {
			_hashes.clear();
			UInt256 temp;
			for (size_t i = 0; i < hashes.size(); ++i) {
				UInt256Set(&temp, hashes[i]);
				_hashes.push_back(temp);
			}
		}

		const std::vector<uint8_t> &MerkleBlockBase::GetFlags() const {
			return _flags;
		}

		void MerkleBlockBase::SetFlags(const std::vector<uint8_t> &flags) {
			_flags = flags;
		}

		void MerkleBlockBase::SerializeNoAux(ByteStream &ostream) const {
			ostream.WriteUint32(_version);
			ostream.WriteBytes(_prevBlock.u8, sizeof(UInt256));
			ostream.WriteBytes(_merkleRoot.u8, sizeof(UInt256));
			ostream.WriteUint32(_timestamp);
			ostream.WriteUint32(_target);
			ostream.WriteUint32(_nonce);
			ostream.WriteUint32(_height);
		}

		bool MerkleBlockBase::DeserializeNoAux(ByteStream &istream) {
			if (!istream.ReadUint32(_version))
				return false;

			if (!istream.ReadBytes(_prevBlock.u8, sizeof(UInt256)))
				return false;

			if (!istream.ReadBytes(_merkleRoot.u8, sizeof(UInt256)))
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
				ostream.WriteBytes(_hashes[i].u8, sizeof(UInt256));
			}

			ostream.WriteVarBytes(_flags.data(), _flags.size());
		}

		bool MerkleBlockBase::DeserializeAfterAux(ByteStream &istream) {
			istream.Drop(1);    //correspond to serialization of node, should get one byte here

			if (!istream.ReadUint32(_totalTx))
				return false;

			uint32_t hashesCount = 0;
			if (!istream.ReadUint32(hashesCount))
				return false;

			for (size_t i = 0; i < hashesCount; ++i) {
				UInt256 hash;
				if (!istream.ReadBytes(hash.u8, sizeof(UInt256)))
					return false;
				_hashes.push_back(hash);
			}

			CMBlock flags;
			if (!istream.ReadVarBytes(flags))
				return false;

			for (int j = 0; j < flags.GetSize(); ++j) {
				_flags.push_back(flags[j]);
			}

			return true;
		}

		size_t MerkleBlockBase::MerkleBlockTxHashes(std::vector<UInt256> &txHashes) const {
			size_t hashIdx = 0, flagIdx = 0;

			return MerkleBlockTxHashesR(txHashes, hashIdx, flagIdx, 0);
		}

		size_t MerkleBlockBase::MerkleBlockTxHashesR(std::vector<UInt256> &txHashes, size_t &hashIdx, size_t &flagIdx,
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
		UInt256 MerkleBlockBase::MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth) const {
			uint8_t flag;
			UInt256 hashes[2], md = UINT256_ZERO;

			if (*flagIdx / 8 < _flags.size() && *hashIdx < _hashes.size()) {
				flag = (_flags[*flagIdx / 8] & (1 << (*flagIdx % 8)));
				(*flagIdx)++;

				if (flag && depth != _ceil_log2(_totalTx)) {
					hashes[0] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // left branch
					hashes[1] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1); // right branch

					if (!UInt256IsZero(&hashes[0]) && !UInt256Eq(&(hashes[0]), &(hashes[1]))) {
						if (UInt256IsZero(&hashes[1]))
							hashes[1] = hashes[0]; // if right branch is missing, dup left branch
						BRSHA256_2(&md, hashes, sizeof(hashes));
					} else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
				} else md = _hashes[(*hashIdx)++]; // leaf
			}

			return md;
		}

		void MerkleBlockBase::SetHash(const UInt256 &hash) {
			memcpy(_blockHash.u8, hash.u8, sizeof(_blockHash));
		}

		bool MerkleBlockBase::IsEqual(const IMerkleBlock *block) const {
			return (block == this || UInt256Eq(&GetHash(), &block->GetHash()));
		}
	}
}
