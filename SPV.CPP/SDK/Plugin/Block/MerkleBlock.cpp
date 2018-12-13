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

namespace Elastos {
	namespace ElaWallet {

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

		MerkleBlock::MerkleBlock() :
			_manageRaw(true) {
			_merkleBlock = ELAMerkleBlockNew();
		}

		MerkleBlock::MerkleBlock(ELAMerkleBlock *merkleBlock, bool manageRaw) :
			_manageRaw(manageRaw),
			_merkleBlock(merkleBlock) {
		}

		MerkleBlock::MerkleBlock(const ELAMerkleBlock &merkleBlock) :
			_manageRaw(true) {
			_merkleBlock = ELAMerkleBlockCopy(&merkleBlock);
		}

		MerkleBlock::~MerkleBlock() {
			if (_merkleBlock != nullptr && _manageRaw)
				ELAMerkleBlockFree(_merkleBlock);
		}

		std::string MerkleBlock::toString() const {
			//todo complete me
			return "";
		}

		BRMerkleBlock *MerkleBlock::getRaw() const {
			return (BRMerkleBlock *) _merkleBlock;
		}

		void MerkleBlock::initFromRaw(BRMerkleBlock *block, bool manageRaw) {
			if (_merkleBlock) {
				ELAMerkleBlockFree((ELAMerkleBlock *)block);
			}
			_merkleBlock = (ELAMerkleBlock *)block;
			_manageRaw = manageRaw;
		}

		IMerkleBlock *MerkleBlock::CreateMerkleBlock(bool manageRaw) {
			return new MerkleBlock(ELAMerkleBlockNew(), manageRaw);
		}

		IMerkleBlock *MerkleBlock::CreateFromRaw(BRMerkleBlock *block, bool manageRaw) {
			return new MerkleBlock((ELAMerkleBlock *)block, manageRaw);
		}

		IMerkleBlock* MerkleBlock::Clone(const BRMerkleBlock *block, bool manageRaw) const {
			return new MerkleBlock(ELAMerkleBlockCopy((const ELAMerkleBlock *)block), manageRaw);
		}

		UInt256 MerkleBlock::getBlockHash() const {
			UInt256 zero = UINT256_ZERO;
			if (UInt256Eq(&_merkleBlock->raw.blockHash, &zero)) {
				ByteStream ostream;
				serializeNoAux(ostream, _merkleBlock->raw);
				UInt256 hash = UINT256_ZERO;
				CMBlock buf = ostream.getBuffer();
				BRSHA256_2(&hash, buf, buf.GetSize());
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
			UInt256 merkleRoot = MerkleBlockRootR(&hashIdx, &flagIdx, 0, _merkleBlock->raw), t = UINT256_ZERO;
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
			serializeNoAux(ostream, _merkleBlock->raw);

			_merkleBlock->auxPow.Serialize(ostream);

			ostream.put(1);    //correspond to serialization of node, should add one byte here

			ostream.writeUint32(_merkleBlock->raw.totalTx);

			ostream.writeUint32((uint32_t)_merkleBlock->raw.hashesCount);
			for (size_t i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				ostream.writeBytes(_merkleBlock->raw.hashes[i].u8, sizeof(UInt256));
			}

			ostream.writeVarBytes(_merkleBlock->raw.flags, _merkleBlock->raw.flagsLen);
		}

		void MerkleBlock::serializeNoAux(ByteStream &ostream, const BRMerkleBlock &raw) {
			ostream.writeUint32(raw.version);
			ostream.writeBytes(raw.prevBlock.u8, sizeof(UInt256));
			ostream.writeBytes(raw.merkleRoot.u8, sizeof(UInt256));
			ostream.writeUint32(raw.timestamp);
			ostream.writeUint32(raw.target);
			ostream.writeUint32(raw.nonce);
			ostream.writeUint32(raw.height);
		}

		bool MerkleBlock::Deserialize(ByteStream &istream) {
			if (!istream.readUint32(_merkleBlock->raw.version))
				return false;

			if (!istream.readBytes(_merkleBlock->raw.prevBlock.u8, sizeof(UInt256)))
				return false;

			if (!istream.readBytes(_merkleBlock->raw.merkleRoot.u8, sizeof(UInt256)))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.timestamp))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.target))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.nonce))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.height))
				return false;

			if (!_merkleBlock->auxPow.Deserialize(istream))
				return false;

			istream.get();    //correspond to serialization of node, should get one byte here

			if (!istream.readUint32(_merkleBlock->raw.totalTx))
				return false;

			uint32_t hashesCount = 0;
			if (!istream.readUint32(hashesCount))
				return false;

			_merkleBlock->raw.hashesCount = hashesCount;

			std::vector<UInt256> hashes;
			for (size_t i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				UInt256 hash;
				if (!istream.readBytes(hash.u8, sizeof(UInt256)))
					return false;
				hashes.push_back(hash);
			}

			CMBlock flags;
			if (!istream.readVarBytes(flags))
				return false;

			_merkleBlock->raw.flagsLen = flags.GetSize();

			BRMerkleBlockSetTxHashes(&_merkleBlock->raw, hashes.data(), hashesCount, flags, flags.GetSize());

			getBlockHash();

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
		UInt256 MerkleBlock::MerkleBlockRootR(size_t *hashIdx, size_t *flagIdx, int depth, const BRMerkleBlock &raw) {
			uint8_t flag;
			UInt256 hashes[2], md = UINT256_ZERO;

			if (*flagIdx/8 < raw.flagsLen && *hashIdx < raw.hashesCount) {
				flag = (raw.flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
				(*flagIdx)++;

				if (flag && depth != _ceil_log2(raw.totalTx)) {
					hashes[0] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1, raw); // left branch
					hashes[1] = MerkleBlockRootR(hashIdx, flagIdx, depth + 1, raw); // right branch

					if (! UInt256IsZero(&hashes[0]) && ! UInt256Eq(&(hashes[0]), &(hashes[1]))) {
						if (UInt256IsZero(&hashes[1])) hashes[1] = hashes[0]; // if right branch is missing, dup left branch
						BRSHA256_2(&md, hashes, sizeof(hashes));
					}
					else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
				}
				else md = raw.hashes[(*hashIdx)++]; // leaf
			}

			return md;
		}

		nlohmann::json MerkleBlock::toJson() const {
			nlohmann::json j;
			if (_merkleBlock == nullptr)
				return j;

			std::vector<std::string> hashes;
			for (int i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				hashes.push_back(Utils::UInt256ToString(_merkleBlock->raw.hashes[i], true));
			}

			std::vector<uint8_t> flags;
			for (int i = 0; i < _merkleBlock->raw.flagsLen; ++i) {
				flags.push_back(_merkleBlock->raw.flags[i]);
			}

			j["BlockHash"] = Utils::UInt256ToString(_merkleBlock->raw.blockHash, true);
			j["Version"] = _merkleBlock->raw.version;
			j["PrevBlock"] = Utils::UInt256ToString(_merkleBlock->raw.prevBlock, true);
			j["MerkleRoot"] = Utils::UInt256ToString(_merkleBlock->raw.merkleRoot, true);
			j["Timestamp"] = _merkleBlock->raw.timestamp;
			j["Target"] = _merkleBlock->raw.target;
			j["Nonce"] = _merkleBlock->raw.nonce;
			j["TotalTx"] = _merkleBlock->raw.totalTx;
			j["Hashes"] = hashes;
			j["Flags"] = flags;
			j["Height"] = _merkleBlock->raw.height;

//			j["AuxPow"] = _merkleBlock->auxPow.toJson();

			return j;
		}

		void MerkleBlock::fromJson(const nlohmann::json &j) {
			assert(_merkleBlock != nullptr);

			if (_merkleBlock == nullptr) {
				return;
			}

			_merkleBlock->raw.blockHash = Utils::UInt256FromString(j["BlockHash"].get<std::string>(), true);
			_merkleBlock->raw.version = j["Version"].get<uint32_t>();
			_merkleBlock->raw.prevBlock = Utils::UInt256FromString(j["PrevBlock"].get<std::string>(), true);
			_merkleBlock->raw.merkleRoot = Utils::UInt256FromString(j["MerkleRoot"].get<std::string>(), true);
			_merkleBlock->raw.timestamp = j["Timestamp"].get<uint32_t>();
			_merkleBlock->raw.target = j["Target"].get<uint32_t>();
			_merkleBlock->raw.nonce = j["Nonce"].get<uint32_t>();
			_merkleBlock->raw.totalTx = j["TotalTx"].get<uint32_t>();

			if (_merkleBlock->raw.hashes != nullptr) {
				free(_merkleBlock->raw.hashes);
				_merkleBlock->raw.hashes = nullptr;
			}

			std::vector<std::string> hashes = j["Hashes"].get<std::vector<std::string>>();
			_merkleBlock->raw.hashesCount = hashes.size();
			_merkleBlock->raw.hashes = (_merkleBlock->raw.hashesCount > 0) ?
				(UInt256 *) malloc(sizeof(UInt256) * _merkleBlock->raw.hashesCount) : nullptr;

			for (int i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				UInt256 hash = Utils::UInt256FromString(hashes[i], true);
				memcpy(&_merkleBlock->raw.hashes[i], &hash, sizeof(hash));
			}

			if (_merkleBlock->raw.flags != nullptr) {
				free(_merkleBlock->raw.flags);
				_merkleBlock->raw.flags = nullptr;
			}

			std::vector<uint8_t> flags = j["Flags"].get<std::vector<uint8_t>>();
			_merkleBlock->raw.flagsLen = flags.size();
			_merkleBlock->raw.flags = (_merkleBlock->raw.flagsLen > 0) ?
				(uint8_t *) malloc(_merkleBlock->raw.flagsLen) : nullptr;
			for (int i = 0; i < _merkleBlock->raw.flagsLen; ++i) {
				_merkleBlock->raw.flags[i] = flags[i];
			}

			_merkleBlock->raw.height = j["Height"].get<uint32_t>();

//			nlohmann::json auxPowJson = j["AuxPow"];
//			_merkleBlock->auxPow.fromJson(auxPowJson);
		}

		BRMerkleBlock *MerkleBlock::getRawBlock() const {
			return getRaw();
		}

		void MerkleBlock::deleteRawBlock() {
			if (_merkleBlock)
				ELAMerkleBlockFree(_merkleBlock);

			_merkleBlock = nullptr;
		}

		REGISTER_MERKLEBLOCKPLUGIN(MerkleBlock);

	}
}
