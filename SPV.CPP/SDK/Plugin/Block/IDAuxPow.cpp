// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDAuxPow.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>

#include <Core/BRMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		IDAuxPow::IDAuxPow() {
			_idAuxMerkleIndex = 0;
			_mainBlockHeader = ELAMerkleBlockNew();
		}

		IDAuxPow::IDAuxPow(const IDAuxPow &idAuxPow) {
			_idAuxMerkleBranch = idAuxPow._idAuxMerkleBranch;
			_idAuxMerkleIndex = idAuxPow._idAuxMerkleIndex;
			_idAuxBlockTx = idAuxPow._idAuxBlockTx;
			_mainBlockHeader = ELAMerkleBlockCopy(idAuxPow._mainBlockHeader);
		}

		IDAuxPow &IDAuxPow::operator=(const IDAuxPow &idAuxPow) {
			_idAuxMerkleBranch = idAuxPow._idAuxMerkleBranch;
			_idAuxMerkleIndex = idAuxPow._idAuxMerkleIndex;
			_idAuxBlockTx = idAuxPow._idAuxBlockTx;
			_mainBlockHeader = ELAMerkleBlockCopy(idAuxPow._mainBlockHeader);

			return *this;
		}

		IDAuxPow::~IDAuxPow() {
			ELAMerkleBlockFree(_mainBlockHeader);
		}

		void IDAuxPow::Serialize(ByteStream &stream) const {
			_idAuxBlockTx.Serialize(stream);

			stream.WriteUint32((uint32_t) _idAuxMerkleBranch.size());

			for (size_t i = 0; i < _idAuxMerkleBranch.size(); ++i) {
				stream.WriteBytes(_idAuxMerkleBranch[i].u8, sizeof(_idAuxMerkleBranch[i]));
			}

			stream.WriteUint32(_idAuxMerkleIndex);

			stream.WriteUint32(_mainBlockHeader->raw.version);
			stream.WriteBytes(_mainBlockHeader->raw.prevBlock.u8, sizeof(_mainBlockHeader->raw.prevBlock.u8));
			stream.WriteBytes(_mainBlockHeader->raw.merkleRoot.u8, sizeof(_mainBlockHeader->raw.merkleRoot.u8));
			stream.WriteUint32(_mainBlockHeader->raw.timestamp);
			stream.WriteUint32(_mainBlockHeader->raw.target);
			stream.WriteUint32(_mainBlockHeader->raw.nonce);
			stream.WriteUint32(_mainBlockHeader->raw.height);

			_mainBlockHeader->auxPow.Serialize(stream);
		}

		bool IDAuxPow::Deserialize(ByteStream &stream) {
			if (!_idAuxBlockTx.Deserialize(stream)) {
				return false;
			}

			uint32_t size = 0;

			if (!stream.ReadUint32(size)) {
				return false;
			}

			_idAuxMerkleBranch.resize(size);
			for (size_t i = 0; i < size; ++i) {
				if (!stream.ReadBytes(_idAuxMerkleBranch[i].u8, sizeof(_idAuxMerkleBranch[i]))) {
					return false;
				}
			}

			if (!stream.ReadUint32(_idAuxMerkleIndex)) {
				return false;
			}

			if (!stream.ReadUint32(_mainBlockHeader->raw.version)) {
				return false;
			}
			if (!stream.ReadBytes(_mainBlockHeader->raw.prevBlock.u8, sizeof(_mainBlockHeader->raw.prevBlock.u8))) {
				return false;
			}
			if (!stream.ReadBytes(_mainBlockHeader->raw.merkleRoot.u8, sizeof(_mainBlockHeader->raw.merkleRoot.u8))) {
				return false;
			}
			if (!stream.ReadUint32(_mainBlockHeader->raw.timestamp)) {
				return false;
			}
			if (!stream.ReadUint32(_mainBlockHeader->raw.target)) {
				return false;
			}
			if (!stream.ReadUint32(_mainBlockHeader->raw.nonce)) {
				return false;
			}
			if (!stream.ReadUint32(_mainBlockHeader->raw.height)) {
				return false;
			}

			return _mainBlockHeader->auxPow.Deserialize(stream);
		}

		nlohmann::json IDAuxPow::MainBlockHeaderToJson() const {
			nlohmann::json j;

			j["BlockHash"] = Utils::UInt256ToString(_mainBlockHeader->raw.blockHash, true);
			j["Version"] = _mainBlockHeader->raw.version;
			j["PrevBlock"] = Utils::UInt256ToString(_mainBlockHeader->raw.prevBlock, true);
			j["MerkleRoot"] = Utils::UInt256ToString(_mainBlockHeader->raw.merkleRoot, true);
			j["Timestamp"] = _mainBlockHeader->raw.timestamp;
			j["Target"] = _mainBlockHeader->raw.target;
			j["Nonce"] = _mainBlockHeader->raw.nonce;
			j["TotalTx"] = _mainBlockHeader->raw.totalTx;

			std::vector<std::string> hashes(_mainBlockHeader->raw.hashesCount);
			for (size_t i = 0; i < _mainBlockHeader->raw.hashesCount; ++i) {
				hashes[i] = Utils::UInt256ToString(_mainBlockHeader->raw.hashes[i], true);
			}
			j["Hashes"] = hashes;

			j["Flags"] = Utils::EncodeHex(_mainBlockHeader->raw.flags, _mainBlockHeader->raw.flagsLen);;
			j["Height"] = _mainBlockHeader->raw.height;

			j["AuxPow"] = _mainBlockHeader->auxPow.ToJson();

			return j;
		}

		void IDAuxPow::MainBlockHeaderFromJson(const nlohmann::json &j) {

			std::string blockHash = j["BlockHash"].get<std::string>();
			_mainBlockHeader->raw.blockHash = Utils::UInt256FromString(blockHash, true);
			_mainBlockHeader->raw.version = j["Version"].get<uint32_t>();
			std::string prevBlock = j["PrevBlock"].get<std::string>();
			_mainBlockHeader->raw.prevBlock = Utils::UInt256FromString(prevBlock, true);
			std::string merkleRoot = j["MerkleRoot"].get<std::string>();
			_mainBlockHeader->raw.merkleRoot = Utils::UInt256FromString(merkleRoot, true);
			_mainBlockHeader->raw.timestamp = j["Timestamp"].get<uint32_t>();
			_mainBlockHeader->raw.target = j["Target"].get<uint32_t>();
			_mainBlockHeader->raw.nonce = j["Nonce"].get<uint32_t>();
			_mainBlockHeader->raw.totalTx = j["TotalTx"].get<uint32_t>();

			std::vector<std::string> hashArray = j["Hashes"];
			_mainBlockHeader->raw.hashesCount = hashArray.size();
			UInt256 hashes[_mainBlockHeader->raw.hashesCount];
			for (size_t i = 0; i < _mainBlockHeader->raw.hashesCount; ++i) {
				hashes[i] = Utils::UInt256FromString(hashArray[i], true);
			}

			CMBlock flags = Utils::DecodeHex(j["Flags"].get<std::string>());
			_mainBlockHeader->raw.flagsLen = flags.GetSize();

			BRMerkleBlockSetTxHashes(&_mainBlockHeader->raw, hashes, _mainBlockHeader->raw.hashesCount,
									 flags, _mainBlockHeader->raw.flagsLen);

			_mainBlockHeader->raw.height = j["Height"].get<uint32_t>();

			_mainBlockHeader->auxPow.FromJson(j["AuxPow"]);
		}

		nlohmann::json IDAuxPow::ToJson() const {
			nlohmann::json j;

			size_t len = _idAuxMerkleBranch.size();
			std::vector<std::string> auxMerkleBranch(len);
			for (size_t i = 0; i < len; ++i) {
				auxMerkleBranch[i] = Utils::UInt256ToString(_idAuxMerkleBranch[i], true);
			}
			j["IdAuxMerkleBranch"] = auxMerkleBranch;

			j["IdAuxMerkleIndex"] = _idAuxMerkleIndex;
			j["IdAuxBlockTx"] = _idAuxBlockTx.ToJson();
			j["MainBlockHeader"] = MainBlockHeaderToJson();

			return j;
		}

		void IDAuxPow::FromJson(const nlohmann::json &j) {
			std::vector<std::string> idAuxMerkleBranch = j["IdAuxMerkleBranch"];
			_idAuxMerkleBranch.resize(idAuxMerkleBranch.size());
			for (size_t i = 0; i < _idAuxMerkleBranch.size(); ++i) {
				_idAuxMerkleBranch[i] = Utils::UInt256FromString(idAuxMerkleBranch[i], true);
			}

			_idAuxMerkleIndex = j["IdAuxMerkleIndex"].get<uint32_t>();
			_idAuxBlockTx.FromJson(j["IdAuxBlockTx"]);
			MainBlockHeaderFromJson(j["MainBlockHeader"]);
		}

		void IDAuxPow::SetIdAuxMerkleBranch(const std::vector<UInt256> &idAuxMerkleBranch) {
			_idAuxMerkleBranch = idAuxMerkleBranch;
		}

		void IDAuxPow::SetIdAuxMerkleIndex(uint32_t index) {
			_idAuxMerkleIndex = index;
		}

		void IDAuxPow::SetIdAuxBlockTx(const Transaction &tx) {
			_idAuxBlockTx = tx;
		}

		void IDAuxPow::SetMainBlockHeader(ELAMerkleBlock *blockHeader) {
			if (_mainBlockHeader) {
				ELAMerkleBlockFree(_mainBlockHeader);
			}
			_mainBlockHeader = blockHeader;
		}

		const std::vector<UInt256> &IDAuxPow::GetIdAuxMerkleBranch() const {
			return _idAuxMerkleBranch;
		}

		uint32_t IDAuxPow::GetIdAuxMerkleIndex() const {
			return _idAuxMerkleIndex;
		}

		const Transaction &IDAuxPow::GetIdAuxBlockTx() const {
			return _idAuxBlockTx;
		}

		ELAMerkleBlock *IDAuxPow::GetMainBlockHeader() const {
			return _mainBlockHeader;
		}

	}
}
