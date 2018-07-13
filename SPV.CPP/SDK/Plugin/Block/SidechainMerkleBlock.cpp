// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <Core/BRMerkleBlock.h>
#include <SDK/Common/Log.h>

#include "Utils.h"
#include "MerkleBlock.h"
#include "SidechainMerkleBlock.h"

#define MAX_PROOF_OF_WORK 0xff7fffff    // highest value for difficulty target

namespace Elastos {
	namespace ElaWallet {

		SidechainMerkleBlock::SidechainMerkleBlock() :
			_manageRaw(true) {
			_merkleBlock = IdMerkleBlockNew();
		}

		SidechainMerkleBlock::SidechainMerkleBlock(IdMerkleBlock *merkleBlock, bool manageRaw) :
			_merkleBlock(merkleBlock),
			_manageRaw(manageRaw) {
		}

		SidechainMerkleBlock::~SidechainMerkleBlock() {
			if (_merkleBlock != nullptr && _manageRaw)
				IdMerkleBlockFree(_merkleBlock);
		}

		std::string SidechainMerkleBlock::toString() const {
			//todo complete me
			return "";
		}

		BRMerkleBlock *SidechainMerkleBlock::getRaw() const {
			return (BRMerkleBlock *)_merkleBlock;
		}

		IMerkleBlock *SidechainMerkleBlock::CreateMerkleBlock(bool manageRaw) {
			return new SidechainMerkleBlock(IdMerkleBlockNew(), manageRaw);
		}

		IMerkleBlock *SidechainMerkleBlock::CreateFromRaw(BRMerkleBlock *block, bool manageRaw) {
			return new SidechainMerkleBlock((IdMerkleBlock *)block, manageRaw);
		}

		IMerkleBlock *SidechainMerkleBlock::Clone(bool manageRaw) const {
			return new SidechainMerkleBlock(IdMerkleBlockCopy(_merkleBlock), manageRaw);
		}

		void SidechainMerkleBlock::Serialize(ByteStream &ostream) const {
			MerkleBlock::serializeNoAux(ostream, _merkleBlock->raw);

			_merkleBlock->idAuxPow.Serialize(ostream);

			ostream.put(1);
			ostream.put(1);

			ostream.writeUint32(_merkleBlock->raw.totalTx);

			ostream.writeUint32((uint32_t)_merkleBlock->raw.hashesCount);
			for (size_t i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				ostream.writeBytes(_merkleBlock->raw.hashes[i].u8, sizeof(_merkleBlock->raw.hashes[i].u8));
			}

			ostream.writeVarBytes(_merkleBlock->raw.flags, _merkleBlock->raw.flagsLen);
		}

		bool SidechainMerkleBlock::Deserialize(ByteStream &istream) {
			if (!istream.readUint32(_merkleBlock->raw.version))
				return false;

			if (!istream.readBytes(_merkleBlock->raw.prevBlock.u8, sizeof(_merkleBlock->raw.prevBlock.u8)))
				return false;

			if (!istream.readBytes(_merkleBlock->raw.merkleRoot.u8, sizeof(_merkleBlock->raw.merkleRoot.u8)))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.timestamp))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.target))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.nonce))
				return false;

			if (!istream.readUint32(_merkleBlock->raw.height))
				return false;

			if (!_merkleBlock->idAuxPow.Deserialize(istream))
				return false;

			// TODO fix me later
			istream.get();
			istream.get();

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

		nlohmann::json SidechainMerkleBlock::toJson() const {
			nlohmann::json j;
			if (_merkleBlock == nullptr)
				return j;

			std::vector<std::string> hashes;
			for (int i = 0; i < _merkleBlock->raw.hashesCount; ++i) {
				hashes.push_back(Utils::UInt256ToString(_merkleBlock->raw.hashes[i]));
			}

			std::vector<uint8_t> flags;
			for (int i = 0; i < _merkleBlock->raw.flagsLen; ++i) {
				flags.push_back(_merkleBlock->raw.flags[i]);
			}

			j["BlockHash"] = Utils::UInt256ToString(_merkleBlock->raw.blockHash);
			j["Version"] = _merkleBlock->raw.version;
			j["PrevBlock"] = Utils::UInt256ToString(_merkleBlock->raw.prevBlock);
			j["MerkleRoot"] = Utils::UInt256ToString(_merkleBlock->raw.merkleRoot);
			j["Timestamp"] = _merkleBlock->raw.timestamp;
			j["Target"] = _merkleBlock->raw.target;
			j["Nonce"] = _merkleBlock->raw.nonce;
			j["TotalTx"] = _merkleBlock->raw.totalTx;
			j["Hashes"] = hashes;
			j["Flags"] = flags;
			j["Height"] = _merkleBlock->raw.height;

			j["IdAuxPow"] = _merkleBlock->idAuxPow.toJson();

			return j;
		}

		void SidechainMerkleBlock::fromJson(const nlohmann::json &j) {
			assert(_merkleBlock != nullptr);

			if (_merkleBlock == nullptr) {
				return;
			}

			_merkleBlock->raw.blockHash = Utils::UInt256FromString(j["BlockHash"].get<std::string>());
			_merkleBlock->raw.version = j["Version"].get<uint32_t>();
			_merkleBlock->raw.prevBlock = Utils::UInt256FromString(j["PrevBlock"].get<std::string>());
			_merkleBlock->raw.merkleRoot = Utils::UInt256FromString(j["MerkleRoot"].get<std::string>());
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
				UInt256 hash = Utils::UInt256FromString(hashes[i]);
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

			nlohmann::json auxPowJson = j["IdAuxPow"];
			_merkleBlock->idAuxPow.fromJson(auxPowJson);
		}

		BRMerkleBlock *SidechainMerkleBlock::getRawBlock() const {
			return getRaw();
		}

		void SidechainMerkleBlock::deleteRawBlock() {
			if (_merkleBlock)
				IdMerkleBlockFree(_merkleBlock);
			_merkleBlock = nullptr;
		}

		void SidechainMerkleBlock::initFromRaw(BRMerkleBlock *block, bool manageRaw) {
			if (_merkleBlock) {
				IdMerkleBlockFree((IdMerkleBlock *)block);
			}
			_merkleBlock = (IdMerkleBlock *)block;
			_manageRaw = manageRaw;
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