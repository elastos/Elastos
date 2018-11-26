// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdAuxPow.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>

#include <Core/BRMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		IdAuxPow::IdAuxPow() {
			_idAuxMerkleIndex = 0;
			_mainBlockHeader = ELAMerkleBlockNew();
		}

		IdAuxPow::IdAuxPow(const IdAuxPow &idAuxPow) {
			_idAuxMerkleBranch = idAuxPow._idAuxMerkleBranch;
			_idAuxMerkleIndex = idAuxPow._idAuxMerkleIndex;
			_idAuxBlockTx = idAuxPow._idAuxBlockTx;
			_mainBlockHeader = ELAMerkleBlockCopy(idAuxPow._mainBlockHeader);
		}

		IdAuxPow &IdAuxPow::operator=(const IdAuxPow &idAuxPow) {
			_idAuxMerkleBranch = idAuxPow._idAuxMerkleBranch;
			_idAuxMerkleIndex = idAuxPow._idAuxMerkleIndex;
			_idAuxBlockTx = idAuxPow._idAuxBlockTx;
			_mainBlockHeader = ELAMerkleBlockCopy(idAuxPow._mainBlockHeader);

			return *this;
		}

		IdAuxPow::~IdAuxPow() {
			ELAMerkleBlockFree(_mainBlockHeader);
		}

		void IdAuxPow::Serialize(ByteStream &stream) const {
			_idAuxBlockTx.Serialize(stream);

			stream.writeUint32((uint32_t)_idAuxMerkleBranch.size());

			for (size_t i = 0; i < _idAuxMerkleBranch.size(); ++i) {
				stream.writeBytes(_idAuxMerkleBranch[i].u8, sizeof(_idAuxMerkleBranch[i]));
			}

			stream.writeUint32(_idAuxMerkleIndex);

			stream.writeUint32(_mainBlockHeader->raw.version);
			stream.writeBytes(_mainBlockHeader->raw.prevBlock.u8, sizeof(_mainBlockHeader->raw.prevBlock.u8));
			stream.writeBytes(_mainBlockHeader->raw.merkleRoot.u8, sizeof(_mainBlockHeader->raw.merkleRoot.u8));
			stream.writeUint32(_mainBlockHeader->raw.timestamp);
			stream.writeUint32(_mainBlockHeader->raw.target);
			stream.writeUint32(_mainBlockHeader->raw.nonce);
			stream.writeUint32(_mainBlockHeader->raw.height);

			_mainBlockHeader->auxPow.Serialize(stream);
		}

		bool IdAuxPow::Deserialize(ByteStream &stream) {
			if (!_idAuxBlockTx.Deserialize(stream)) {
				return false;
			}

			uint32_t size = 0;

			if (!stream.readUint32(size)) {
				return false;
			}

			_idAuxMerkleBranch.resize(size);
			for (size_t i = 0; i < size; ++i) {
				if (!stream.readBytes(_idAuxMerkleBranch[i].u8, sizeof(_idAuxMerkleBranch[i]))) {
					return false;
				}
			}

			if (!stream.readUint32(_idAuxMerkleIndex)) {
				return false;
			}

			if (!stream.readUint32(_mainBlockHeader->raw.version)) {
				return false;
			}
			if (!stream.readBytes(_mainBlockHeader->raw.prevBlock.u8, sizeof(_mainBlockHeader->raw.prevBlock.u8))) {
				return false;
			}
			if (!stream.readBytes(_mainBlockHeader->raw.merkleRoot.u8, sizeof(_mainBlockHeader->raw.merkleRoot.u8))) {
				return false;
			}
			if (!stream.readUint32(_mainBlockHeader->raw.timestamp)) {
				return false;
			}
			if (!stream.readUint32(_mainBlockHeader->raw.target)) {
				return false;
			}
			if (!stream.readUint32(_mainBlockHeader->raw.nonce)) {
				return false;
			}
			if (!stream.readUint32(_mainBlockHeader->raw.height)) {
				return false;
			}

			return _mainBlockHeader->auxPow.Deserialize(stream);
		}

		nlohmann::json IdAuxPow::mainBlockHeaderToJson() const {
			nlohmann::json j;

			j["BlockHash"] = Utils::UInt256ToString(_mainBlockHeader->raw.blockHash);
			j["Version"] = _mainBlockHeader->raw.version;
			j["PrevBlock"] = Utils::UInt256ToString(_mainBlockHeader->raw.prevBlock);
			j["MerkleRoot"] = Utils::UInt256ToString(_mainBlockHeader->raw.merkleRoot);
			j["Timestamp"] = _mainBlockHeader->raw.timestamp;
			j["Target"] = _mainBlockHeader->raw.target;
			j["Nonce"] = _mainBlockHeader->raw.nonce;
			j["TotalTx"] = _mainBlockHeader->raw.totalTx;

			std::vector<std::string> hashes(_mainBlockHeader->raw.hashesCount);
			for (size_t i = 0; i < _mainBlockHeader->raw.hashesCount; ++i) {
				hashes[i] = Utils::UInt256ToString(_mainBlockHeader->raw.hashes[i]);
			}
			j["Hashes"] = hashes;

			j["Flags"] = Utils::encodeHex(_mainBlockHeader->raw.flags, _mainBlockHeader->raw.flagsLen);;
			j["Height"] = _mainBlockHeader->raw.height;

			j["AuxPow"] = _mainBlockHeader->auxPow.toJson();

			return j;
		}

		void IdAuxPow::mainBlockHeaderFromJson(const nlohmann::json &j) {

			std::string blockHash = j["BlockHash"].get<std::string>();
			_mainBlockHeader->raw.blockHash = Utils::UInt256FromString(blockHash);
			_mainBlockHeader->raw.version = j["Version"].get<uint32_t>();
			std::string prevBlock = j["PrevBlock"].get<std::string>();
			_mainBlockHeader->raw.prevBlock = Utils::UInt256FromString(prevBlock);
			std::string merkleRoot = j["MerkleRoot"].get<std::string>();
			_mainBlockHeader->raw.merkleRoot = Utils::UInt256FromString(merkleRoot);
			_mainBlockHeader->raw.timestamp = j["Timestamp"].get<uint32_t>();
			_mainBlockHeader->raw.target = j["Target"].get<uint32_t>();
			_mainBlockHeader->raw.nonce = j["Nonce"].get<uint32_t>();
			_mainBlockHeader->raw.totalTx = j["TotalTx"].get<uint32_t>();

			std::vector<std::string> hashArray = j["Hashes"];
			_mainBlockHeader->raw.hashesCount = hashArray.size();
			UInt256 hashes[_mainBlockHeader->raw.hashesCount];
			for (size_t i = 0; i < _mainBlockHeader->raw.hashesCount; ++i) {
				hashes[i] = Utils::UInt256FromString(hashArray[i]);
			}

			CMBlock flags = Utils::decodeHex(j["Flags"].get<std::string>());
			_mainBlockHeader->raw.flagsLen = flags.GetSize();

			BRMerkleBlockSetTxHashes(&_mainBlockHeader->raw, hashes, _mainBlockHeader->raw.hashesCount,
									 flags, _mainBlockHeader->raw.flagsLen);

			_mainBlockHeader->raw.height = j["Height"].get<uint32_t>();

			_mainBlockHeader->auxPow.fromJson(j["AuxPow"]);
		}

		nlohmann::json IdAuxPow::toJson() const {
			nlohmann::json j;

			size_t len = _idAuxMerkleBranch.size();
			std::vector<std::string> auxMerkleBranch(len);
			for (size_t i = 0; i < len; ++i) {
				auxMerkleBranch[i] = Utils::UInt256ToString(_idAuxMerkleBranch[i]);
			}
			j["IdAuxMerkleBranch"] = auxMerkleBranch;

			j["IdAuxMerkleIndex"] = _idAuxMerkleIndex;
			j["IdAuxBlockTx"] = _idAuxBlockTx.toJson();
			j["MainBlockHeader"] = mainBlockHeaderToJson();

			return j;
		}

		void IdAuxPow::fromJson(const nlohmann::json &j) {
			std::vector<std::string> idAuxMerkleBranch = j["IdAuxMerkleBranch"];
			_idAuxMerkleBranch.resize(idAuxMerkleBranch.size());
			for (size_t i = 0; i < _idAuxMerkleBranch.size(); ++i) {
				_idAuxMerkleBranch[i] = Utils::UInt256FromString(idAuxMerkleBranch[i]);
			}

			_idAuxMerkleIndex = j["IdAuxMerkleIndex"].get<uint32_t>();
			_idAuxBlockTx.fromJson(j["IdAuxBlockTx"]);
			mainBlockHeaderFromJson(j["MainBlockHeader"]);
		}

		void IdAuxPow::setIdAuxMerkleBranch(const std::vector<UInt256> &idAuxMerkleBranch) {
			_idAuxMerkleBranch = idAuxMerkleBranch;
		}

		void IdAuxPow::setIdAuxMerkleIndex(uint32_t index) {
			_idAuxMerkleIndex = index;
		}

		void IdAuxPow::setIdAuxBlockTx(const Transaction &tx) {
			_idAuxBlockTx = tx;
		}

		void IdAuxPow::setMainBlockHeader(ELAMerkleBlock *blockHeader) {
			if (_mainBlockHeader) {
				ELAMerkleBlockFree(_mainBlockHeader);
			}
			_mainBlockHeader = blockHeader;
		}

		const std::vector<UInt256> &IdAuxPow::getIdAuxMerkleBranch() const {
			return _idAuxMerkleBranch;
		}

		uint32_t IdAuxPow::getIdAuxMerkleIndex() const {
			return _idAuxMerkleIndex;
		}

		const Transaction &IdAuxPow::getIdAuxBlockTx() const {
			return _idAuxBlockTx;
		}

		ELAMerkleBlock *IdAuxPow::getMainBlockHeader() const {
			return _mainBlockHeader;
		}

	}
}
