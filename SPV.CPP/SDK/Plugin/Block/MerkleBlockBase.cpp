// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Utils.h>
#include <Core/BRCrypto.h>
#include "MerkleBlockBase.h"

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

		uint32_t MerkleBlockBase::getVersion() const {
			return _version;
		}

		void MerkleBlockBase::setVersion(uint32_t version) {
			_version = version;
		}

		const UInt256 &MerkleBlockBase::getPrevBlockHash() const {
			return _prevBlock;
		}

		const UInt256 &MerkleBlockBase::getRootBlockHash() const {
			return _merkleRoot;
		}

		uint32_t MerkleBlockBase::getTimestamp() const {
			return _timestamp;
		}

		uint32_t MerkleBlockBase::getTarget() const {
			return _target;
		}

		uint32_t MerkleBlockBase::getNonce() const {
			return _nonce;
		}

		uint32_t MerkleBlockBase::getTransactionCount() const {
			return _totalTx;
		}

		uint32_t MerkleBlockBase::getHeight() const {
			return _height;
		}

		void MerkleBlockBase::setHeight(uint32_t height) {
			_height = height;
		}

		nlohmann::json MerkleBlockBase::toJson() const {
			nlohmann::json j;

			std::vector<std::string> hashes;
			for (int i = 0; i < _hashes.size(); ++i) {
				hashes.push_back(Utils::UInt256ToString(_hashes[i]));
			}

			std::vector<uint8_t> flags;
			for (int i = 0; i < _flags.size(); ++i) {
				flags.push_back(_flags[i]);
			}

			j["BlockHash"] = Utils::UInt256ToString(_blockHash);
			j["Version"] = _version;
			j["PrevBlock"] = Utils::UInt256ToString(_prevBlock);
			j["MerkleRoot"] = Utils::UInt256ToString(_merkleRoot);
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

		void MerkleBlockBase::fromJson(const nlohmann::json &j) {

			_blockHash = Utils::UInt256FromString(j["BlockHash"].get<std::string>());
			_version = j["Version"].get<uint32_t>();
			_prevBlock = Utils::UInt256FromString(j["PrevBlock"].get<std::string>());
			_merkleRoot = Utils::UInt256FromString(j["MerkleRoot"].get<std::string>());
			_timestamp = j["Timestamp"].get<uint32_t>();
			_target = j["Target"].get<uint32_t>();
			_nonce = j["Nonce"].get<uint32_t>();
			_totalTx = j["TotalTx"].get<uint32_t>();

			_hashes.clear();
			std::vector<std::string> hashes = j["Hashes"].get<std::vector<std::string>>();
			for (int i = 0; i < hashes.size(); ++i) {
				_hashes.push_back(Utils::UInt256FromString(hashes[i]));
			}

			_flags.clear();
			std::vector<uint8_t> flags = j["Flags"].get<std::vector<uint8_t>>();
			for (int i = 0; i < flags.size(); ++i) {
				_flags.push_back(flags[i]);
			}

			_height = j["Height"].get<uint32_t>();
		}

		void MerkleBlockBase::setTimestamp(uint32_t timestamp) {
			_timestamp = timestamp;
		}

		void MerkleBlockBase::setPrevBlockHash(const UInt256 &hash) {
			UInt256Set(&_prevBlock, hash);
		}

		void MerkleBlockBase::setRootBlockHash(const UInt256 &hash) {
			UInt256Set(&_merkleRoot, hash);
		}

		void MerkleBlockBase::setTarget(uint32_t target) {
			_target = target;
		}

		void MerkleBlockBase::setNonce(uint32_t nonce) {
			_nonce = nonce;
		}

		void MerkleBlockBase::setTransactionCount(uint32_t count) {
			_totalTx = count;
		}

		const std::vector<UInt256> &MerkleBlockBase::getHashes() const {
			return _hashes;
		}

		void MerkleBlockBase::setHashes(const std::vector<UInt256> &hashes) {
			_hashes.clear();
			UInt256 temp;
			for (size_t i = 0; i < hashes.size(); ++i) {
				UInt256Set(&temp, hashes[i]);
				_hashes.push_back(temp);
			}
		}

		const std::vector<uint8_t> &MerkleBlockBase::getFlags() const {
			return _flags;
		}

		void MerkleBlockBase::setFlags(const std::vector<uint8_t> &flags) {
			_flags = flags;
		}

		void MerkleBlockBase::serializeNoAux(ByteStream &ostream) const {
			ostream.writeUint32(_version);
			ostream.writeBytes(_prevBlock.u8, sizeof(UInt256));
			ostream.writeBytes(_merkleRoot.u8, sizeof(UInt256));
			ostream.writeUint32(_timestamp);
			ostream.writeUint32(_target);
			ostream.writeUint32(_nonce);
			ostream.writeUint32(_height);
		}

		bool MerkleBlockBase::deserializeNoAux(ByteStream &istream) {
			if (!istream.readUint32(_version))
				return false;

			if (!istream.readBytes(_prevBlock.u8, sizeof(UInt256)))
				return false;

			if (!istream.readBytes(_merkleRoot.u8, sizeof(UInt256)))
				return false;

			if (!istream.readUint32(_timestamp))
				return false;

			if (!istream.readUint32(_target))
				return false;

			if (!istream.readUint32(_nonce))
				return false;

			if (!istream.readUint32(_height))
				return false;

			return true;
		}

		void MerkleBlockBase::serializeAfterAux(ByteStream &ostream) const {
			ostream.writeUint8(1);    //correspond to serialization of node, should add one byte here

			ostream.writeUint32(_totalTx);

			ostream.writeUint32((uint32_t) _hashes.size());
			for (size_t i = 0; i < _hashes.size(); ++i) {
				ostream.writeBytes(_hashes[i].u8, sizeof(UInt256));
			}

			ostream.writeVarBytes(_flags.data(), _flags.size());
		}

		bool MerkleBlockBase::deserializeAfterAux(ByteStream &istream) {
			istream.drop(1);    //correspond to serialization of node, should get one byte here

			if (!istream.readUint32(_totalTx))
				return false;

			uint32_t hashesCount = 0;
			if (!istream.readUint32(hashesCount))
				return false;

			for (size_t i = 0; i < hashesCount; ++i) {
				UInt256 hash;
				if (!istream.readBytes(hash.u8, sizeof(UInt256)))
					return false;
				_hashes.push_back(hash);
			}

			CMBlock flags;
			if (!istream.readVarBytes(flags))
				return false;

			for (int j = 0; j < flags.GetSize(); ++j) {
				_flags.push_back(flags[j]);
			}

			return true;
		}

		std::vector<UInt256> MerkleBlockBase::MerkleBlockTxHashes() const {
			size_t hashIdx = 0, flagIdx = 0;

			return merkleBlockTxHashesR(hashIdx, flagIdx, 0);
		}

		std::vector<UInt256> MerkleBlockBase::merkleBlockTxHashesR(size_t &hashIdx, size_t &flagIdx,
																   int depth) const {
			std::vector<UInt256> txHashes;
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
					merkleBlockTxHashesR(hashIdx, flagIdx, depth + 1); // left branch
					merkleBlockTxHashesR(hashIdx, flagIdx, depth + 1); // right branch
				}
			}

			return txHashes;
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

		void MerkleBlockBase::setHash(const UInt256 &hash) {
			memcpy(_blockHash.u8, hash.u8, sizeof(_blockHash));
		}

		bool MerkleBlockBase::isEqual(const IMerkleBlock *block) const {
			return (block == this || UInt256Eq(&getHash(), &block->getHash()));
		}
	}
}
