// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CRInfo.h"
#include <Common/Log.h>
#include <WalletCore/Key.h>
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {
		CRInfo::CRInfo() {

		}

		CRInfo::~CRInfo() {

		}

		const bytes_t &CRInfo::GetCode() const {
			return _code;
		}

		void CRInfo::SetCode(const bytes_t &code) {
			_code = code;
		}

		const uint168 &CRInfo::GetCID() const {
			return _cid;
		}

		void CRInfo::SetCID(const uint168 &cid) {
			_cid = cid;
		}

		const uint168 &CRInfo::GetDID() const {
			return _did;
		}

		void CRInfo::SetDID(const uint168 &did) {
			_did = did;
		}

		const std::string &CRInfo::GetNickName() const {
			return _nickName;
		}

		void CRInfo::SetNickName(const std::string &nickName) {
			_nickName = nickName;
		}

		const std::string &CRInfo::GetUrl() const {
			return _url;
		}

		void CRInfo::SetUrl(const std::string &url) {
			_url = url;
		}

		uint64_t CRInfo::GetLocation() const {
			return _location;
		}

		void CRInfo::SetLocation(uint64_t location) {
			_location = location;
		}

		const bytes_t &CRInfo::GetSignature() const {
			return _signature;
		}

		void CRInfo::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		size_t CRInfo::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_code.size());
			size += _code.size();
			size += _cid.size();
			if (version > CRInfoVersion)
				size += _did.size();
			size += stream.WriteVarUint(_nickName.size());
			size += _nickName.size();
			size += stream.WriteVarUint(_url.size());
			size += _url.size();
			size += sizeof(_location);
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void CRInfo::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool CRInfo::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				Log::error("CRInfo Deserialize: payload unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("CRInfo Deserialize: read signature");
				return false;
			}

			return true;
		}

		void CRInfo::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_code);
			ostream.WriteBytes(_cid);
			if (version > CRInfoVersion)
				ostream.WriteBytes(_did);
			ostream.WriteVarString(_nickName);
			ostream.WriteVarString(_url);
			ostream.WriteUint64(_location);
		}

		bool CRInfo::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarBytes(_code)) {
				Log::error("CRInfo Deserialize: read _code");
				return false;
			}
			if (!istream.ReadBytes(_cid)) {
				Log::error("CRInfo Deserialize: read _cid");
				return false;
			}
			if (version > CRInfoVersion) {
				if (!istream.ReadBytes(_did)) {
					Log::error("CRInfo Deserialize: read _did");
					return false;
				}
			}
			if (!istream.ReadVarString(_nickName)) {
				Log::error("CRInfoDeserialize: read nick name");
				return false;
			}
			if (!istream.ReadVarString(_url)) {
				Log::error("CRInfo Deserialize: read url");
				return false;
			}
			if (!istream.ReadUint64(_location)) {
				Log::error("CRInfo Deserialize: read location");
				return false;
			}

			return true;
		}

		nlohmann::json CRInfo::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Code"] = _code.getHex();
			j["CID"] = Address(_cid).String();
			j["DID"] = Address(_did).String();
			j["NickName"] = _nickName;
			j["Url"] = _url;
			j["Location"] = _location;
			j["Signature"] = _signature.getHex();
			return j;
		}

		void CRInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string code = j["Code"].get<std::string>();
			_code.setHex(code);

			std::string cid = j["CID"].get<std::string>();
			_cid = Address(cid).ProgramHash();

			std::string did = j["DID"].get<std::string>();
			_did = Address(did).ProgramHash();

			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<uint64_t>();

			std::string signature = j["Signature"].get<std::string>();
			_signature.setHex(signature);
		}

		bool CRInfo::IsValid(uint8_t version) const {
			ByteStream stream(_code);
			bytes_t pubKey;
			stream.ReadVarBytes(pubKey);

			Key key;
			key.SetPubKey(pubKey);

			ByteStream ostream;
			SerializeUnsigned(ostream, version);
			uint256 digest(sha256(ostream.GetBytes()));

			return key.Verify(digest, _signature);
		}

		IPayload &CRInfo::operator=(const IPayload &payload) {
			try {
				const CRInfo &crinfo = dynamic_cast<const CRInfo &>(payload);
				operator=(crinfo);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CRInfo");
			}

			return *this;
		}

		CRInfo &CRInfo::operator=(const CRInfo &payload) {
			_code = payload._code;
			_cid = payload._cid;
			_did = payload._did;
			_nickName = payload._nickName;
			_url = payload._url;
			_location = payload._location;
			_signature = payload._signature;
			return *this;
		}
	}
}
