// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDAuxPow.h"

#include <Common/Utils.h>
#include <Common/ByteStream.h>
#include <Common/Log.h>

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
				stream.WriteBytes(_idAuxMerkleBranch[i]);
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
			stream.WriteUint8(1);
		}

		bool IDAuxPow::Deserialize(const ByteStream &stream) {
			if (!_idAuxBlockTx.Deserialize(stream)) {
				return false;
			}

			uint32_t size = 0;

			if (!stream.ReadUint32(size)) {
				return false;
			}

			_idAuxMerkleBranch.resize(size);
			for (size_t i = 0; i < size; ++i) {
				if (!stream.ReadBytes(_idAuxMerkleBranch[i])) {
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

			if (!_mainBlockHeader->auxPow.Deserialize(stream)) {
				return false;
			}

			uint8_t skipByte = 0;
			return stream.ReadUint8(skipByte);
		}

		void IDAuxPow::SetIdAuxMerkleBranch(const std::vector<uint256> &idAuxMerkleBranch) {
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

		const std::vector<uint256> &IDAuxPow::GetIdAuxMerkleBranch() const {
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
