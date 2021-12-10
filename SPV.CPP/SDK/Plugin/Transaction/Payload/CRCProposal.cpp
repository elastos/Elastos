/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "CRCProposal.h"

#include <Common/hash.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Key.h>
#include <Common/Base64.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

#define DRAFT_DATA_MAX_SIZE (1024 * 1024)

		Budget::Budget() {

		}

		Budget::Budget(Budget::Type type, uint8_t stage, const BigInt &amount) :
			_type(type), _stage(stage), _amount(amount) {

		}

		Budget::~Budget() {

		}

		Budget::Type Budget::GetType() const {
			return _type;
		}

		uint8_t Budget::GetStage() const {
			return _stage;
		}

		BigInt Budget::GetAmount() const {
			return _amount;
		}

		void Budget::Serialize(ByteStream &ostream) const {
			ostream.WriteUint8(_type);
			ostream.WriteUint8(_stage);
			ostream.WriteUint64(_amount.getUint64());
		}

		bool Budget::Deserialize(const ByteStream &istream) {
			uint8_t type;
			if (!istream.ReadUint8(type)) {
				SPVLOG_ERROR("Budget::Deserialize: read type key");
				return false;
			}
			_type = Budget::Type(type);

			if (!istream.ReadUint8(_stage)) {
				SPVLOG_ERROR("Budget::Deserialize: read stage key");
				return false;
			}

			uint64_t amount;
			if (!istream.ReadUint64(amount)) {
				SPVLOG_ERROR("Budget::Deserialize: read amount key");
				return false;
			}
			_amount.setUint64(amount);

			return true;
		}

		bool Budget::IsValid() const {
			if (_type >= Budget::Type::maxType) {
				SPVLOG_ERROR("invalid budget type: {}", _type);
				return false;
			}

			if (_stage > 127) {
				SPVLOG_ERROR("invalid budget stage", _stage);
				return false;
			}

			return true;
		}

		nlohmann::json Budget::ToJson() const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyStage] = _stage;
			j[JsonKeyAmount] = _amount.getDec();
			return j;
		}

		void Budget::FromJson(const nlohmann::json &j) {
			_type = Budget::Type(j[JsonKeyType].get<uint8_t>());
			_stage = j[JsonKeyStage].get<uint8_t>();
			_amount.setDec(j[JsonKeyAmount].get<std::string>());
		}

		bool Budget::operator==(const Budget &budget) const {
			return _type == budget._type && _stage == budget._stage && _amount == budget._amount;
		}

        UpgradeCodeInfo::UpgradeCodeInfo() {

        }

        UpgradeCodeInfo::~UpgradeCodeInfo() {

        }

        void UpgradeCodeInfo::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteUint32(_workingHeight);
            stream.WriteVarString(_nodeVersion);
            stream.WriteVarString(_nodeDownloadUrl);
            stream.WriteBytes(_nodeBinHash);
            stream.WriteUint8(_force ? 0x01 : 0x00);
        }

        bool UpgradeCodeInfo::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadUint32(_workingHeight)) {
                SPVLOG_ERROR("deserialize workingHeight failed");
                return false;
            }

            if (!stream.ReadVarString(_nodeVersion)) {
                SPVLOG_ERROR("deserialize nodeVersion failed");
                return false;
            }

            if (!stream.ReadVarString(_nodeDownloadUrl)) {
                SPVLOG_ERROR("deserialize nodeDownloadUrl failed");
                return false;
            }

            if (!stream.ReadBytes(_nodeBinHash)) {
                SPVLOG_ERROR("deserialize nodeBinHash failed");
                return false;
            }

            uint8_t force = 0;
            if (!stream.ReadUint8(force)) {
                SPVLOG_ERROR("deserialize force failed");
                return false;
            }
            _force = force == 0 ? false : true;

            return true;
        }

        nlohmann::json UpgradeCodeInfo::ToJson(uint8_t version) const {
            nlohmann::json j;
            j["WorkingHeight"] = _workingHeight;
            j["NodeVersion"] = _nodeVersion;
            j["NodeDownloadUrl"] = _nodeDownloadUrl;
            j["NodeBinHash"] = _nodeBinHash.GetHex();
            j["Force"] = _force;
            return j;
		}

        void UpgradeCodeInfo::FromJson(const nlohmann::json &j, uint8_t version) {
            _workingHeight = j["WorkingHeight"].get<uint32_t>();
            _nodeVersion = j["NodeVersion"].get<std::string>();
            _nodeDownloadUrl = j["NodeDownloadUrl"].get<std::string>();
            _nodeBinHash.SetHex(j["NodeBinHash"].get<std::string>());
            _force = j["Force"].get<bool>();
		}

        bool UpgradeCodeInfo::IsValid(uint8_t version) const {
		    return true;
		}

        SideChainInfo::SideChainInfo() {

		}

        SideChainInfo::~SideChainInfo() {

		}

        void SideChainInfo::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteVarString(_sideChainName);
            stream.WriteUint32(_magicNumber);
            stream.WriteBytes(_genesisHash);
            stream.WriteUint64(_exchangeRate);
            stream.WriteUint32(_effectiveHeight);
            stream.WriteVarString(_resourcePath);
		}

        bool SideChainInfo::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_sideChainName)) {
                SPVLOG_ERROR("deserialize side-chain name failed");
                return false;
            }

            if (!stream.ReadUint32(_magicNumber)) {
                SPVLOG_ERROR("deserialize magic number failed");
                return false;
            }

            if (!stream.ReadBytes(_genesisHash)) {
                SPVLOG_ERROR("deserialize genesis hash failed");
                return false;
            }

            if (!stream.ReadUint64(_exchangeRate)) {
                SPVLOG_ERROR("deserialize exchange rate failed");
                return false;
            }

            if (!stream.ReadUint32(_effectiveHeight)) {
                SPVLOG_ERROR("deserialize effective height failed");
                return false;
            }

            if (!stream.ReadVarString(_resourcePath)) {
                SPVLOG_ERROR("deserialize resource path failed");
                return false;
            }

            return true;
		}

        nlohmann::json SideChainInfo::ToJson(uint8_t version) const {
            nlohmann::json j;
            j["SideChainName"] = _sideChainName;
            j["MagicNumber"] = _magicNumber;
            j["GenesisHash"] = _genesisHash.GetHex();
            j["ExchangeRate"] = _exchangeRate;
            j["EffectiveHeight"] = _effectiveHeight;
            j["ResourcePath"] = _resourcePath;
            return j;
		}

        void SideChainInfo::FromJson(const nlohmann::json &j, uint8_t version) {
            _sideChainName = j["SideChainName"].get<std::string>();
            _magicNumber = j["MagicNumber"].get<uint32_t>();
            _genesisHash.SetHex(j["GenesisHash"].get<std::string>());
            _exchangeRate = j["ExchangeRate"].get<uint64_t>();
            _effectiveHeight = j["EffectiveHeight"].get<uint32_t>();
            _resourcePath = j["ResourcePath"].get<std::string>();
		}

        bool SideChainInfo::IsValid(uint8_t version) const {
            return true;
		}

        bool SideChainInfo::operator==(const SideChainInfo &info) const {
            return _sideChainName == info._sideChainName &&
                   _magicNumber == info._magicNumber &&
                   _genesisHash == info._genesisHash &&
                   _exchangeRate == info._exchangeRate &&
                   _effectiveHeight == info._effectiveHeight &&
                   _resourcePath == info._resourcePath;
        }

        SideChainInfo &SideChainInfo::operator=(const SideChainInfo &info) {
		    _sideChainName = info._sideChainName;
		    _magicNumber = info._magicNumber;
		    _genesisHash = info._genesisHash;
		    _exchangeRate = info._exchangeRate;
		    _effectiveHeight = info._effectiveHeight;
		    _resourcePath = info._resourcePath;
            return *this;
		}

        CustomIDFeeRateInfo::CustomIDFeeRateInfo() {

        }

        CustomIDFeeRateInfo::~CustomIDFeeRateInfo() {

        }

        void CustomIDFeeRateInfo::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteUint64(_rateOfCustomIDFee);
            stream.WriteUint32(_eIDEffectiveHeight);
        }

        bool CustomIDFeeRateInfo::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadUint64(_rateOfCustomIDFee)) {
                SPVLOG_ERROR("deserialize rateOfCustomIDFee failed");
                return false;
            }

            if (!stream.ReadUint32(_eIDEffectiveHeight)) {
                SPVLOG_ERROR("deserialize eIDEffectiveHeight failed");
                return false;
            }

            return true;
        }

        nlohmann::json CustomIDFeeRateInfo::ToJson(uint8_t version) const {
            nlohmann::json j;
            j["RateOfCustomIDFee"] = _rateOfCustomIDFee;
            j["EIDEffectiveHeight"] = _eIDEffectiveHeight;
            return j;
        }

        void CustomIDFeeRateInfo::FromJson(const nlohmann::json &j, uint8_t version) {
            _rateOfCustomIDFee = j["RateOfCustomIDFee"].get<uint64_t>();
            _eIDEffectiveHeight = j["EIDEffectiveHeight"].get<uint32_t>();
        }

        bool CustomIDFeeRateInfo::operator==(const CustomIDFeeRateInfo &info) const {
            return this->_rateOfCustomIDFee == info._rateOfCustomIDFee && this->_eIDEffectiveHeight == info._eIDEffectiveHeight;
        }

        CustomIDFeeRateInfo &CustomIDFeeRateInfo::operator=(const CustomIDFeeRateInfo &info) {
            this->_rateOfCustomIDFee = info._rateOfCustomIDFee;
            this->_eIDEffectiveHeight = info._eIDEffectiveHeight;
            return *this;
        }

		CRCProposal::CRCProposal() {

		}

		CRCProposal::~CRCProposal() {

		}

		void CRCProposal::SetTpye(CRCProposal::Type type) {
			_type = type;
		}

		CRCProposal::Type CRCProposal::GetType() const {
			return _type;
		}

		void CRCProposal::SetCategoryData(const std::string &categoryData) {
			_categoryData = categoryData;
		}

		const std::string &CRCProposal::GetCategoryData() const {
			return _categoryData;
		}

		void CRCProposal::SetOwnerPublicKey(const bytes_t &publicKey) {
			_ownerPublicKey = publicKey;
		}

		const bytes_t &CRCProposal::GetOwnerPublicKey() const {
			return _ownerPublicKey;
		}

		void CRCProposal::SetCRCouncilMemberDID(const Address &crSponsorDID) {
			_crCouncilMemberDID = crSponsorDID;
		}

		const Address &CRCProposal::GetCRCouncilMemberDID() const {
			return _crCouncilMemberDID;
		}

		void CRCProposal::SetDraftHash(const uint256 &draftHash) {
			_draftHash = draftHash;
		}

		const uint256 &CRCProposal::GetDraftHash() const {
			return _draftHash;
		}

		void CRCProposal::SetDraftData(const bytes_t &draftData) {
			_draftData = draftData;
		}

		const bytes_t &CRCProposal::GetDraftData() const {
			return _draftData;
		}

		void CRCProposal::SetBudgets(const std::vector<Budget> &budgets) {
			_budgets = budgets;
		}

		const std::vector<Budget> &CRCProposal::GetBudgets() const {
			return _budgets;
		}

		void CRCProposal::SetRecipient(const Address &recipient) {
			_recipient = recipient;
		}

		const Address &CRCProposal::GetRecipient() const {
			return _recipient;
		}

		void CRCProposal::SetTargetProposalHash(const uint256 &hash) {
			_targetProposalHash = hash;
		}

		const uint256 &CRCProposal::GetTargetProposalHash() const {
			return _targetProposalHash;
		}

		void CRCProposal::SetNewRecipient(const Address &recipient) {
			_newRecipient = recipient;
		}

		const Address &CRCProposal::GetNewRecipient() const {
			return _newRecipient;
		}

		void CRCProposal::SetNewOwnerPublicKey(const bytes_t &pubkey) {
			_newOwnerPublicKey = pubkey;
		}

		const bytes_t &CRCProposal::GetNewOwnerPublicKey() const {
			return _newOwnerPublicKey;
		}

		void CRCProposal::SetSecretaryPublicKey(const bytes_t &pubkey) {
			_secretaryPublicKey = pubkey;
		}

		const bytes_t CRCProposal::GetSecretaryPublicKey() const {
			return _secretaryPublicKey;
		}

		void CRCProposal::SetSecretaryDID(const Address &did) {
			_secretaryDID = did;
		}

		const Address &CRCProposal::GetSecretaryDID() const {
			return _secretaryDID;
		}

		void CRCProposal::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposal::GetSignature() const {
			return _signature;
		}

		void CRCProposal::SetNewOwnerSignature(const bytes_t &sign) {
			_newOwnerSignature = sign;
		}

		const bytes_t &CRCProposal::GetNewOwnerSignature() const {
			return _newOwnerSignature;
		}

		void CRCProposal::SetSecretarySignature(const bytes_t &sign) {
			_secretarySignature = sign;
		}

		const bytes_t &CRCProposal::GetSecretarySignature() const {
			return _secretarySignature;
		}

		void CRCProposal::SetCRCouncilMemberSignature(const bytes_t &signature) {
			_crCouncilMemberSignature = signature;
		}

		const bytes_t &CRCProposal::GetCRCouncilMemberSignature() const {
			return _crCouncilMemberSignature;
		}

        uint256 CRCProposal::DigestNormalOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeOwnerUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestNormalCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

		size_t CRCProposal::EstimateSize(uint8_t version) const {
			ByteStream stream, byteStream;
			size_t size = 0;

			size += sizeof(uint16_t);
			size += stream.WriteVarUint(_categoryData.size());
			size += _categoryData.size();
			size += stream.WriteVarUint(_ownerPublicKey.size());
			size += _ownerPublicKey.size();
			if (version >= CRCProposalVersion01) {
				size += stream.WriteVarUint(_draftData.size());
				size += _draftData.size();
			}
			size += _draftHash.size();

			switch (_type) {
				case elip:
				case normal:
					size += stream.WriteVarUint(_budgets.size());

					for (size_t i = 0; i < _budgets.size(); ++i) {
						_budgets[i].Serialize(byteStream);
					}
					size += byteStream.GetBytes().size();
					size += _recipient.ProgramHash().size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					break;

				case secretaryGeneralElection:
					size += stream.WriteVarUint(_secretaryPublicKey.size());
					size += _secretaryPublicKey.size();
					size += _secretaryDID.ProgramHash().size();
					size += stream.WriteVarUint(_secretarySignature.size());
					size += _secretarySignature.size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					break;

				case changeProposalOwner:
					size += _targetProposalHash.size();
					size += _newRecipient.ProgramHash().size();
					size += stream.WriteVarUint(_newOwnerPublicKey.size());
					size += _newOwnerPublicKey.size();
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					size += stream.WriteVarUint(_newOwnerSignature.size());
					size += _newOwnerSignature.size();
					break;

				case terminateProposal:
					size += stream.WriteVarUint(_signature.size());
					size += _signature.size();
					size += _targetProposalHash.size();
					break;

				default:
					break;
			}

			size += _crCouncilMemberDID.ProgramHash().size();
			size += stream.WriteVarUint(_crCouncilMemberSignature.size());
			size += _crCouncilMemberSignature.size();

			return size;
		}

		// normal or elip
		void CRCProposal::SerializeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			stream.WriteUint16(_type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i)
				_budgets[i].Serialize(stream);
			stream.WriteBytes(_recipient.ProgramHash());
		}

		bool CRCProposal::DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draftdata");
					return false;
				}
			}

			uint64_t count = 0;
			if (!stream.ReadVarUint(count)) {
				SPVLOG_ERROR("deserialize budgets size");
				return false;
			}
			_budgets.resize(count);
			for (size_t i = 0; i < count; ++i) {
				if (!_budgets[i].Deserialize(stream)) {
					SPVLOG_ERROR("deserialize bugets");
					return false;
				}
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize recipient");
				return false;
			}
			_recipient = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeCRCouncilMemberUnsigned(ByteStream &ostream, uint8_t version) const {
			SerializeOwnerUnsigned(ostream, version);

			ostream.WriteVarBytes(_signature);

			ostream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeCRCouncilMemberUnsigned(const ByteStream &istream, uint8_t version) {
			if (!DeserializeOwnerUnsigned(istream, version)) {
				SPVLOG_ERROR("deserialize unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			uint168 programHash;
			if (!istream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeNormalOrELIP(ByteStream &stream, uint8_t version) const {
			SerializeCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeNormalOrELIP(const ByteStream &stream, uint8_t version) {
			if (!DeserializeCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("CRCProposal deserialize crc unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("CRCProposal deserialize crc signature");
				return false;
			}

			return true;
		}

		// change owner
		void CRCProposal::SerializeChangeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);

			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteBytes(_targetProposalHash);
			stream.WriteBytes(_newRecipient.ProgramHash());
			stream.WriteVarBytes(_newOwnerPublicKey);
		}

		bool CRCProposal::DeserializeChangeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draftData");
					return false;
				}
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize target proposal hash");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize new recipient");
				return false;
			}
			_newRecipient = Address(programHash);

			if (!stream.ReadVarBytes(_newOwnerPublicKey)) {
				SPVLOG_ERROR("deserialize new owner PublicKey");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeChangeOwnerCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteVarBytes(_newOwnerSignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeChangeOwnerCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize change owner signature");
				return false;
			}

			if (!stream.ReadVarBytes(_newOwnerSignature)) {
				SPVLOG_ERROR("deserialize change owner new owner signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeChangeOwner(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeChangeOwner(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = EncodeDraftData(_draftData);
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();
			j[JsonKeyNewRecipient] = _newRecipient.String();
			j[JsonKeyNewOwnerPublicKey] = _newOwnerPublicKey.getHex();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = CheckAndDecodeDraftData(draftData, _draftHash);
			}
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
			_newRecipient = Address(j[JsonKeyNewRecipient].get<std::string>());
			_newOwnerPublicKey.setHex(j[JsonKeyNewOwnerPublicKey].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonChangeOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyNewOwnerSignature] = _newOwnerSignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonChangeOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_newOwnerSignature.setHex(j[JsonKeyNewOwnerSignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidChangeOwnerUnsigned(uint8_t version) const {
			if (_type != changeProposalOwner) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
				Key key1(CTElastos, _newOwnerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			if (!_newRecipient.Valid()) {
				SPVLOG_ERROR("invalid new recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidChangeOwnerUnsigned(version)) {
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}

				if (!Key(CTElastos, _newOwnerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _newOwnerSignature)) {
					SPVLOG_ERROR("verify new owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

        uint256 CRCProposal::DigestChangeOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeChangeOwnerUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

		// terminate proposal
		void CRCProposal::SerializeTerminateProposalUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteBytes(_targetProposalHash);
		}

		bool CRCProposal::DeserializeTerminateProposalUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize terminate proposal category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize terminate proposal owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize terminate proposal draft hash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize terminate proposal draftData");
					return false;
				}
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize terminate proposal target proposal hash");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeTerminateProposalCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeTerminateProposalCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize terminate proposal signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeTerminateProposal(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeTerminateProposal(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = EncodeDraftData(_draftData);
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = CheckAndDecodeDraftData(draftData, _draftHash);
			}
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonTerminateProposalOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonTerminateProposalOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidTerminateProposalOwnerUnsigned(uint8_t version) const {
			if (_type != terminateProposal) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidTerminateProposalOwnerUnsigned(version)) {
				SPVLOG_ERROR("terminate proposal unsigned is not valid");
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestTerminateProposalOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

        uint256 CRCProposal::DigestTerminateProposalOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeTerminateProposalUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

		// change secretary general
		void CRCProposal::SerializeSecretaryElectionUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			if (version >= CRCProposalVersion01)
				stream.WriteVarBytes(_draftData);
			stream.WriteVarBytes(_secretaryPublicKey);
			stream.WriteBytes(_secretaryDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draft hash");
				return false;
			}

			if (version >= CRCProposalVersion01) {
				if (!stream.ReadVarBytes(_draftData)) {
					SPVLOG_ERROR("deserialize draft data");
					return false;
				}
			}

			if (!stream.ReadVarBytes(_secretaryPublicKey)) {
				SPVLOG_ERROR("deserialize secretary pubkey");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_secretaryDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElectionCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteVarBytes(_secretarySignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change secretary secretary unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			if (!stream.ReadVarBytes(_secretarySignature)) {
				SPVLOG_ERROR("deserialize secretary signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize cr council mem did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElection(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeSecretaryElection(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionCRCouncilMemberUnsigned(stream, version)) {
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change secretary cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = EncodeDraftData(_draftData);
			j[JsonKeySecretaryPublicKey] = _secretaryPublicKey.getHex();
			j[JsonKeySecretaryDID] = _secretaryDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = CheckAndDecodeDraftData(draftData, _draftHash);
			}
			_secretaryPublicKey.setHex(j[JsonKeySecretaryPublicKey].get<std::string>());
			_secretaryDID = Address(j[JsonKeySecretaryDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j;

			j = ToJsonSecretaryElectionUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeySecretarySignature] = _secretarySignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonSecretaryElectionUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_secretarySignature.setHex(j[JsonKeySecretarySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidSecretaryElectionUnsigned(uint8_t version) const {
			if (_type != secretaryGeneralElection) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
				Key key1(CTElastos, _secretaryPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid ...");
				return false;
			}

			if (!_secretaryDID.Valid()) {
				SPVLOG_ERROR("invalid secretary did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidSecretaryElectionUnsigned(version)) {
				SPVLOG_ERROR("secretary election secretary unsigned not valid");
				return false;
			}

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestSecretaryElectionUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
				if (!Key(CTElastos, _secretaryPublicKey).Verify(DigestSecretaryElectionUnsigned(version), _secretarySignature)) {
					SPVLOG_ERROR("verify secretary signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

        uint256 CRCProposal::DigestSecretaryElectionUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeSecretaryElectionUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        void CRCProposal::SerializeReserveCustomIDUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            stream.WriteVarUint((uint64_t)_reservedCustomIDList.size());
            for (const std::string &reservedCustomID : _reservedCustomIDList)
                stream.WriteVarString(reservedCustomID);
		}

        bool CRCProposal::DeserializeReserveCustomIDUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize reserved custom id category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize reserved custom id owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize reserved custom id draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize reserved custom id draft data");
                    return false;
                }
            }

            uint64_t size = 0;
            if (!stream.ReadVarUint(size)) {
                SPVLOG_ERROR("deserialize reserved custom id list size");
                return false;
            }
            for (size_t i = 0; i < size; ++i) {
                std::string reservedCustomID;
                if (!stream.ReadVarString(reservedCustomID)) {
                    SPVLOG_ERROR("deserialize reserved custom id list[{}]", i);
                    return false;
                }
                _reservedCustomIDList.push_back(reservedCustomID);
            }

            return true;
		}

        void CRCProposal::SerializeReserveCustomIDCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeReserveCustomIDUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReserveCustomIDCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReserveCustomIDUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize reserved custom id signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReserveCustomID(ByteStream &stream, uint8_t version) const {
            SerializeReserveCustomIDCRCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeReserveCustomID(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReserveCustomIDCRCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize reserved custom id council member sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonReserveCustomIDOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = EncodeDraftData(_draftData);
            j[JsonKeyReservedCustomIDList] = _reservedCustomIDList;

            return j;
		}

        void CRCProposal::FromJsonReserveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = CheckAndDecodeDraftData(draftData, _draftHash);
            }
            _reservedCustomIDList = j[JsonKeyReservedCustomIDList].get<std::vector<std::string>>();
		}

        nlohmann::json CRCProposal::ToJsonReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonReserveCustomIDOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonReserveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonReserveCustomIDOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidReserveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_type != reserveCustomID) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidReserveCustomIDOwnerUnsigned(version))
                return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestReserveCustomIDOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("reserve custom id verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        uint256 CRCProposal::DigestReserveCustomIDOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeReserveCustomIDUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeReserveCustomIDCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        // ReceiveCustomID
        void CRCProposal::SerializeReceiveCustomIDUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            stream.WriteVarUint((uint64_t)_receivedCustomIDList.size());
            for (const std::string &receivedCustomID : _receivedCustomIDList)
                stream.WriteVarString(receivedCustomID);
            stream.WriteBytes(_receiverDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReceiveCustomIDUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize receive custom category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize receive custom owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize receive custom draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize receive custom draft data");
                    return false;
                }
            }

            uint64_t size = 0;
            if (!stream.ReadVarUint(size)) {
                SPVLOG_ERROR("deserialize receive custom id list size");
                return false;
            }
            for (size_t i = 0; i < size; ++i) {
                std::string receivedCustomID;
                if (!stream.ReadVarString(receivedCustomID)) {
                    SPVLOG_ERROR("deserialize receive custom id list[{}]", i);
                    return false;
                }
                _receivedCustomIDList.push_back(receivedCustomID);
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize receiver did");
                return false;
            }
            _receiverDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReceiveCustomIDCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeReceiveCustomIDUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeReceiveCustomIDCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReceiveCustomIDUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize reserved custom id signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeReceiveCustomID(ByteStream &stream, uint8_t version) const {
            SerializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeReceiveCustomID(const ByteStream &stream, uint8_t version) {
            if (!DeserializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize receive custom id council member sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = EncodeDraftData(_draftData);
            j[JsonKeyReceivedCustomIDList] = _receivedCustomIDList;
            j[JsonKeyReceiverDID] = _receiverDID.String();

            return j;
		}

        void CRCProposal::FromJsonReceiveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = CheckAndDecodeDraftData(draftData, _draftHash);
            }
            _receivedCustomIDList = j[JsonKeyReservedCustomIDList].get<std::vector<std::string>>();
            _receiverDID = Address(j[JsonKeyReceiverDID].get<std::string>());
		}

        nlohmann::json CRCProposal::ToJsonReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonReceiveCustomIDOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonReceiveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonReceiveCustomIDOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            if (_type != receiveCustomID) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
		    if (!IsValidReceiveCustomIDOwnerUnsigned(version))
		        return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestReceiveCustomIDOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("receive custom id verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        uint256 CRCProposal::DigestReceiveCustomIDOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeReceiveCustomIDUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeReceiveCustomIDCRCCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        // ChangeCustomIDFee
        void CRCProposal::SerializeChangeCustomIDFeeUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            _customIDFeeRateInfo.Serialize(stream, version);
		}

        bool CRCProposal::DeserializeChangeCustomIDFeeUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize change custom id category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize change custom id owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize change custom id draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize change custom id draft data");
                    return false;
                }
            }

            if (!_customIDFeeRateInfo.Deserialize(stream, version)) {
                SPVLOG_ERROR("deserialize change custom id fee");
                return false;
            }

            return true;
		}

        void CRCProposal::SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeChangeCustomIDFeeUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeChangeCustomIDFeeCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeChangeCustomIDFeeUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize change custom id fee signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize change custom id fee cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeChangeCustomIDFee(ByteStream &stream, uint8_t version) const {
            SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeChangeCustomIDFee(const ByteStream &stream, uint8_t version) {
            if (!DeserializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize change custom id fee council mem sign");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = EncodeDraftData(_draftData);
            j[JsonKeyCustomIDFeeRateInfo] = _customIDFeeRateInfo.ToJson(version);

            return j;
		}

        void CRCProposal::FromJsonChangeCustomIDFeeOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
            _type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = CheckAndDecodeDraftData(draftData, _draftHash);
            }
            _customIDFeeRateInfo.FromJson(j[JsonKeyCustomIDFeeRateInfo], version);
		}

        nlohmann::json CRCProposal::ToJsonChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
		    nlohmann::json j = ToJsonChangeCustomIDFeeOwnerUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
		    return j;
		}

        void CRCProposal::FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonChangeCustomIDFeeOwnerUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            if (_type != changeCustomIDFee) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidChangeCustomIDFeeOwnerUnsigned(version))
                return false;

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestChangeCustomIDFeeOwnerUnsigned(version), _signature)) {
                    SPVLOG_ERROR("change custom id fee verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        uint256 CRCProposal::DigestChangeCustomIDFeeOwnerUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeChangeCustomIDFeeUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        uint256 CRCProposal::DigestChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
        }

        void CRCProposal::SerializeRegisterSidechainUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            if (version >= CRCProposalVersion01)
                stream.WriteVarBytes(_draftData);
            _sidechainInfo.Serialize(stream, version);
		}

        bool CRCProposal::DeserializeRegisterSidechainUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize change custom id category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize change custom id owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize change custom id draft hash");
                return false;
            }

            if (version >= CRCProposalVersion01) {
                if (!stream.ReadVarBytes(_draftData)) {
                    SPVLOG_ERROR("deserialize change custom id draft data");
                    return false;
                }
            }

            if (!_sidechainInfo.Deserialize(stream, version)) {
                SPVLOG_ERROR("deserialize change custom id fee");
                return false;
            }

            return true;
		}

        void CRCProposal::SerializeRegisterSidechainCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeRegisterSidechainUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeRegisterSidechainCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeRegisterSidechainUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize id signature");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize cr council mem did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);
            return true;
		}

        void CRCProposal::SerializeRegisterSidechain(ByteStream &stream, uint8_t version) const {
            SerializeRegisterSidechainCRCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeRegisterSidechain(const ByteStream &stream, uint8_t version) {
            if (!DeserializeRegisterSidechainCRCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize register side-chain council member sign");
                return false;
            }
            return true;
		}

        nlohmann::json CRCProposal::ToJsonRegisterSidechainUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            if (version >= CRCProposalVersion01)
                j[JsonKeyDraftData] = EncodeDraftData(_draftData);
            j[JsonKeySidechainInfo] = _sidechainInfo.ToJson(version);

            return j;
		}

        void CRCProposal::FromJsonRegisterSidechainUnsigned(const nlohmann::json &j, uint8_t version) {
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            if (version >= CRCProposalVersion01) {
                std::string draftData = j[JsonKeyDraftData].get<std::string>();
                _draftData = CheckAndDecodeDraftData(draftData, _draftHash);
            }
            _sidechainInfo.FromJson(j[JsonKeySidechainInfo], version);
		}

        nlohmann::json CRCProposal::ToJsonRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonRegisterSidechainUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonRegisterSidechainCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonRegisterSidechainUnsigned(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidRegisterSidechainUnsigned(uint8_t version) const {
            if (_type != registerSideChain) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid reserve custom id pubkey");
                return false;
            }

            if (!_sidechainInfo.IsValid(version)) {
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidRegisterSidechainUnsigned(version)) {
                return false;
            }

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestRegisterSidechainUnsigned(version), _signature)) {
                    SPVLOG_ERROR("change register side-chain verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        uint256 CRCProposal::DigestRegisterSidechainUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeRegisterSidechainUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
		}

        uint256 CRCProposal::DigestRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeRegisterSidechainCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
		}

        // upgrade code
        void CRCProposal::SerializeUpgradeCodeUnsigned(ByteStream &stream, uint8_t version) const {
            uint16_t type = _type;
            stream.WriteUint16(type);
            stream.WriteVarString(_categoryData);
            stream.WriteVarBytes(_ownerPublicKey);
            stream.WriteBytes(_draftHash);
            _upgradeCodeInfo.Serialize(stream, version);
		}

        bool CRCProposal::DeserializeUpgradeCodeUnsigned(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarString(_categoryData)) {
                SPVLOG_ERROR("deserialize upgrade code category data");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPublicKey)) {
                SPVLOG_ERROR("deserialize upgrade code owner pubkey");
                return false;
            }

            if (!stream.ReadBytes(_draftHash)) {
                SPVLOG_ERROR("deserialize upgrade code draft hash");
                return false;
            }

            if (!_upgradeCodeInfo.Deserialize(stream, version)) {
                SPVLOG_ERROR("deserialize upgrade code");
                return false;
            }

            return true;
		}

        void CRCProposal::SerializeUpgradeCodeCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
            SerializeUpgradeCodeUnsigned(stream, version);
            stream.WriteVarBytes(_signature);
            stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

        bool CRCProposal::DeserializeUpgradeCodeCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
            if (!DeserializeUpgradeCodeUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                SPVLOG_ERROR("deserialize upgrade code signature failed");
                return false;
            }

            uint168 programHash;
            if (!stream.ReadBytes(programHash)) {
                SPVLOG_ERROR("deserialize upgrade code cr council did");
                return false;
            }
            _crCouncilMemberDID = Address(programHash);

            return true;
		}

        void CRCProposal::SerializeUpgradeCode(ByteStream &stream, uint8_t version) const {
            SerializeUpgradeCodeCRCouncilMemberUnsigned(stream, version);
            stream.WriteVarBytes(_crCouncilMemberSignature);
		}

        bool CRCProposal::DeserializeUpgradeCode(const ByteStream &stream, uint8_t version) {
            if (!DeserializeUpgradeCodeCRCouncilMemberUnsigned(stream, version)) {
                return false;
            }

            if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
                SPVLOG_ERROR("deserialize cr council mem sign failed");
                return false;
            }

            return true;
		}

        nlohmann::json CRCProposal::ToJsonUpgradeCodeUnsigned(uint8_t version) const {
            nlohmann::json j;

            j[JsonKeyType] = _type;
            j[JsonKeyCategoryData] = _categoryData;
            j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
            j[JsonKeyDraftHash] = _draftHash.GetHex();
            j[JsonKeyUpgradeCodeInfo] = _upgradeCodeInfo.ToJson(version);

            return j;
		}

        void CRCProposal::FromJsonUpgradeCode(const nlohmann::json &j, uint8_t version) {
            uint16_t type = j[JsonKeyType].get<uint16_t>();
            _type = CRCProposal::Type(type);
            _categoryData = j[JsonKeyCategoryData].get<std::string>();
            _ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
            _draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
            _upgradeCodeInfo.FromJson(j[JsonKeyUpgradeCodeInfo], version);
		}

        nlohmann::json CRCProposal::ToJsonUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const {
            nlohmann::json j = ToJsonUpgradeCodeUnsigned(version);
            j[JsonKeySignature] = _signature.getHex();
            j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
            return j;
		}

        void CRCProposal::FromJsonUpgradeCodeCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
            FromJsonUpgradeCode(j, version);
            _signature.setHex(j[JsonKeySignature].get<std::string>());
            _crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

        bool CRCProposal::IsValidUpgradeCodeUnsigned(uint8_t version) const {
            if (_type != mainChainUpgradeCode &&
                _type != ethUpdateCode &&
                _type != didUpdateCode) {
                SPVLOG_ERROR("invalid type: {}", _type);
                return false;
            }

            if (_categoryData.size() > 4096) {
                SPVLOG_ERROR("category data exceed 4096 bytes");
                return false;
            }

            try {
                Key key(CTElastos, _ownerPublicKey);
            } catch (const std::exception &e) {
                SPVLOG_ERROR("invalid owner pubkey");
                return false;
            }

            if (!_upgradeCodeInfo.IsValid(version)) {
                return false;
            }

            return true;
		}

        bool CRCProposal::IsValidUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const {
            if (!IsValidUpgradeCodeUnsigned(version)) {
                return false;
            }

            try {
                if (!Key(CTElastos, _ownerPublicKey).Verify(DigestUpgradeCodeUnsigned(version), _signature)) {
                    SPVLOG_ERROR("change upgrade code verify owner signature fail");
                    return false;
                }
            } catch (const std::exception &e) {
                SPVLOG_ERROR("verify signature exception: {}", e.what());
                return false;
            }

            if (!_crCouncilMemberDID.Valid()) {
                SPVLOG_ERROR("invalid cr committee did");
                return false;
            }

            return true;
		}

        uint256 CRCProposal::DigestUpgradeCodeUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeUpgradeCodeUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
		}

        uint256 CRCProposal::DigestUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const {
            ByteStream stream;
            SerializeUpgradeCodeCRCouncilMemberUnsigned(stream, version);
            return uint256(sha256(stream.GetBytes()));
		}

		// top serialize or deserialize
		void CRCProposal::Serialize(ByteStream &stream, uint8_t version) const {
			switch (_type) {
				case changeProposalOwner:
					SerializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					SerializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					SerializeSecretaryElection(stream, version);
					break;

			    case reserveCustomID:
                    SerializeReserveCustomID(stream, version);
			        break;

			    case receiveCustomID:
                    SerializeReceiveCustomID(stream, version);
			        break;

			    case changeCustomIDFee:
                    SerializeChangeCustomIDFee(stream, version);
			        break;

			    case registerSideChain:
			        SerializeRegisterSidechain(stream, version);
			        break;

			    case normal:
			    case elip:
					SerializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("serialize cr proposal unknown type");
					break;
			}
		}

		bool CRCProposal::Deserialize(const ByteStream &stream, uint8_t version) {
			uint16_t type = 0;
			if (!stream.ReadUint16(type)) {
				SPVLOG_ERROR("deserialize type");
				return false;
			}
			_type = CRCProposal::Type(type);

			bool r = false;
			switch (_type) {
				case changeProposalOwner:
					r = DeserializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					r = DeserializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					r = DeserializeSecretaryElection(stream, version);
					break;

                case reserveCustomID:
                    r = DeserializeReserveCustomID(stream, version);
                    break;

                case receiveCustomID:
                    r = DeserializeReceiveCustomID(stream, version);
                    break;

                case changeCustomIDFee:
                    r = DeserializeChangeCustomIDFee(stream, version);
                    break;

			    case registerSideChain:
			        r = DeserializeRegisterSidechain(stream, version);
			        break;

			    case mainChainUpgradeCode:
			    case didUpdateCode:
			    case ethUpdateCode:
			        r = DeserializeUpgradeCode(stream, version);
			        break;

                case normal:
                case elip:
					r = DeserializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					r = false;
					break;
			}

			return r;
		}

		nlohmann::json CRCProposal::ToJsonNormalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			if (version >= CRCProposalVersion01)
				j[JsonKeyDraftData] = EncodeDraftData(_draftData);
			j[JsonKeyBudgets] = _budgets;
			j[JsonKeyRecipient] = _recipient.String();
			return j;
		}

		void CRCProposal::FromJsonNormalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			if (version >= CRCProposalVersion01) {
				std::string draftData = j[JsonKeyDraftData].get<std::string>();
				_draftData = CheckAndDecodeDraftData(draftData, _draftHash);
			}
			_budgets = j[JsonKeyBudgets].get<std::vector<Budget>>();
			_recipient = Address(j[JsonKeyRecipient].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonNormalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonNormalOwnerUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
			return j;
		}

		void CRCProposal::FromJsonNormalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonNormalOwnerUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j;
			switch (_type) {
				case normal:
				case elip: j = ToJsonNormalCRCouncilMemberUnsigned(version); break;
				case secretaryGeneralElection: j = ToJsonSecretaryElectionCRCouncilMemberUnsigned(version); break;
				case changeProposalOwner: j = ToJsonChangeOwnerCRCouncilMemberUnsigned(version); break;
				case terminateProposal: j = ToJsonTerminateProposalCRCouncilMemberUnsigned(version); break;
			    case reserveCustomID: j = ToJsonReserveCustomIDCRCouncilMemberUnsigned(version); break;
			    case receiveCustomID: j = ToJsonReceiveCustomIDCRCouncilMemberUnsigned(version); break;
			    case changeCustomIDFee: j = ToJsonChangeCustomIDFeeCRCouncilMemberUnsigned(version); break;
			    case registerSideChain: j = ToJsonRegisterSidechainCRCouncilMemberUnsigned(version); break;
				default: SPVLOG_ERROR("unknow type: {}", _type); return j;
			}
            j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
            return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			switch (_type) {
				case normal:
				case elip: FromJsonNormalCRCouncilMemberUnsigned(j, version); break;
				case secretaryGeneralElection: FromJsonSecretaryElectionCRCouncilMemberUnsigned(j, version); break;
				case changeProposalOwner: FromJsonChangeOwnerCRCouncilMemberUnsigned(j, version); break;
				case terminateProposal: FromJsonTerminateProposalCRCouncilMemberUnsigned(j, version); break;
			    case reserveCustomID: FromJsonReserveCustomIDCRCouncilMemberUnsigned(j, version); break;
			    case receiveCustomID: FromJsonReceiveCustomIDCRCouncilMemberUnsigned(j, version); break;
			    case changeCustomIDFee: FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(j, version); break;
			    case registerSideChain: FromJsonRegisterSidechainCRCouncilMemberUnsigned(j, version);
				default: SPVLOG_ERROR("unknow type: {}", _type); return;
			}
            _crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
		}

		bool CRCProposal::IsValidNormalOwnerUnsigned(uint8_t version) const {
			if (_type >= CRCProposal::maxType) {
				SPVLOG_ERROR("invalid proposal type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(CTElastos, _ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid proposal owner pubkey");
				return false;
			}

			for (const Budget &budget : _budgets) {
				if (!budget.IsValid()) {
					SPVLOG_ERROR("invalid budget");
					return false;
				}
			}

			if (!_recipient.Valid()) {
				SPVLOG_ERROR("invalid recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidNormalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidNormalOwnerUnsigned(version))
				return false;

			try {
				if (!Key(CTElastos, _ownerPublicKey).Verify(DigestNormalOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValid(uint8_t version) const {
			bool isValid = false;
			switch (_type) {
				case normal:
				case elip: isValid = IsValidNormalCRCouncilMemberUnsigned(version); break;
				case secretaryGeneralElection: isValid = IsValidSecretaryElectionCRCouncilMemberUnsigned(version); break;
				case changeProposalOwner: isValid = IsValidChangeOwnerCRCouncilMemberUnsigned(version); break;
				case terminateProposal: isValid = IsValidTerminateProposalCRCouncilMemberUnsigned(version); break;
			    case reserveCustomID: isValid = IsValidReserveCustomIDCRCouncilMemberUnsigned(version); break;
			    case receiveCustomID: isValid = IsValidReceiveCustomIDCRCouncilMemberUnsigned(version); break;
			    case changeCustomIDFee: isValid = IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(version); break;
			    case registerSideChain: isValid = IsValidRegisterSidechainCRCouncilMemberUnsigned(version); break;
				default: break;
			}
			if (_crCouncilMemberSignature.empty()) {
				SPVLOG_ERROR("cr committee not signed");
				isValid = false;
			}
			return isValid;
		}

		CRCProposal &CRCProposal::operator=(const CRCProposal &payload) {
			_type = payload._type;
			_categoryData = payload._categoryData;
			_ownerPublicKey = payload._ownerPublicKey;
			_draftHash = payload._draftHash;
			_draftData = payload._draftData;
			_budgets = payload._budgets;
			_recipient = payload._recipient;
			_targetProposalHash = payload._targetProposalHash;
            _reservedCustomIDList = payload._reservedCustomIDList;
            _receivedCustomIDList = payload._receivedCustomIDList;
            _receiverDID = payload._receiverDID;
            _customIDFeeRateInfo = payload._customIDFeeRateInfo;
			_newRecipient = payload._newRecipient;
			_newOwnerPublicKey = payload._newOwnerPublicKey;
			_secretaryPublicKey = payload._secretaryPublicKey;
			_secretaryDID = payload._secretaryDID;
			_signature = payload._signature;
			_newOwnerSignature = payload._newOwnerSignature;
			_secretarySignature = payload._secretarySignature;
			_upgradeCodeInfo = payload._upgradeCodeInfo;
			_sidechainInfo = payload._sidechainInfo;

			_crCouncilMemberDID = payload._crCouncilMemberDID;
			_crCouncilMemberSignature = payload._crCouncilMemberSignature;
			return *this;
		}

		bool CRCProposal::Equal(const IPayload &payload, uint8_t version) const {
			bool equal = false;
            const CRCProposal &p = dynamic_cast<const CRCProposal &>(payload);
			try {
				switch (_type) {
					case normal:
					case elip:
						equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _budgets == p._budgets &&
                                _recipient == p._recipient &&
                                _signature == p._signature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
						break;
					case secretaryGeneralElection:
						equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _secretaryPublicKey == p._secretaryPublicKey &&
                                _secretaryDID == p._secretaryDID &&
                                _signature == p._signature &&
                                _secretarySignature == p._secretarySignature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
						break;
					case changeProposalOwner:
						equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _targetProposalHash == p._targetProposalHash &&
                                _newRecipient == p._newRecipient &&
                                _newOwnerPublicKey == p._newOwnerPublicKey &&
                                _signature == p._signature &&
                                _newOwnerSignature == p._newOwnerSignature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
						break;
					case terminateProposal:
						equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _targetProposalHash == p._targetProposalHash &&
                                _signature == p._signature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
						break;
				    case reserveCustomID:
				        equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _reservedCustomIDList == p._reservedCustomIDList &&
                                _signature == p._signature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
				        break;
				    case receiveCustomID:
                        equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _receivedCustomIDList == p._receivedCustomIDList &&
                                _receiverDID == p._receiverDID &&
                                _signature == p._signature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
				        break;
				    case changeCustomIDFee:
                        equal = _type == p._type &&
                                _categoryData == p._categoryData &&
                                _ownerPublicKey == p._ownerPublicKey &&
                                _draftHash == p._draftHash &&
                                _customIDFeeRateInfo == p._customIDFeeRateInfo &&
                                _signature == p._signature &&
                                _crCouncilMemberDID == p._crCouncilMemberDID &&
                                _crCouncilMemberSignature == p._crCouncilMemberSignature;
				        break;
				    case registerSideChain:
				        equal = _type == p._type &&
				                _categoryData == p._categoryData &&
				                _ownerPublicKey == p._ownerPublicKey &&
				                _draftHash == p._draftHash &&
				                _sidechainInfo == p._sidechainInfo &&
				                _signature == p._signature &&
				                _crCouncilMemberDID == p._crCouncilMemberDID &&
				                _crCouncilMemberSignature == p._crCouncilMemberSignature;
				        break;
					default:
						equal = false;
						break;
				}
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposal");
				equal = false;
			}
            if (version >= CRCProposalVersion01)
                equal = equal && _draftData == p._draftData;
			return equal;
		}

		IPayload &CRCProposal::operator=(const IPayload &payload) {
			try {
				const CRCProposal &crcProposal = dynamic_cast<const CRCProposal &>(payload);
				operator=(crcProposal);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposal");
			}
			return *this;
		}

//#define DraftData_Base64
#define DraftData_Hexstring
        std::string CRCProposal::EncodeDraftData(const bytes_t &draftData) const {
#ifdef DraftData_Hexstring
		    return draftData.getHex();
#else
            return Base64::Encode(draftData);
#endif
		}

        bytes_t CRCProposal::CheckAndDecodeDraftData(const std::string &draftData, const uint256 &draftHash) const {
#ifdef DraftData_Hexstring
            bytes_t draftDataDecoded(draftData);
#else
            bytes_t draftDataDecoded = Base64::Decode(draftData);
#endif
            ErrorChecker::CheckParam(draftDataDecoded.size() > DRAFT_DATA_MAX_SIZE, Error::ProposalContentTooLarge, "proposal origin content too large");
            uint256 draftHashDecoded(sha256_2(draftDataDecoded));
            ErrorChecker::CheckParam(draftHash != draftHashDecoded, Error::ProposalHashNotMatch, "proposal hash not match");
            return draftDataDecoded;
        }

	}
}