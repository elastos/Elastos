/*
 * Copyright (c) 2020 Elastos Foundation
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

#include "NextTurnDPoSInfo.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		NextTurnDPoSInfo::NextTurnDPoSInfo() :
			_workingHeight(0) {

		}

		NextTurnDPoSInfo::NextTurnDPoSInfo(uint32_t blockHeight,
										   const std::vector<bytes_t> &crPubkeys,
										   const std::vector<bytes_t> &dposPubkeys) :
			_workingHeight(blockHeight),
			_crPublicKeys(crPubkeys),
			_dposPublicKeys(dposPubkeys) {

		}

		NextTurnDPoSInfo::NextTurnDPoSInfo(const NextTurnDPoSInfo &payload) {
			operator=(payload);
		}

		NextTurnDPoSInfo::~NextTurnDPoSInfo() {

		}

		void NextTurnDPoSInfo::SetWorkingHeight(uint32_t height) {
			_workingHeight = height;
		}

		uint32_t NextTurnDPoSInfo::GetWorkingHeight() const {
			return _workingHeight;
		}

		void NextTurnDPoSInfo::SetCRPublicKeys(const std::vector<bytes_t> pubkeys) {
			_crPublicKeys = pubkeys;
		}

		std::vector<bytes_t> NextTurnDPoSInfo::GetCRPublicKeys() const {
			return _crPublicKeys;
		}

		void NextTurnDPoSInfo::SetDPoSPublicKeys(const std::vector<bytes_t> pubkeys) {
			_dposPublicKeys = pubkeys;
		}

		std::vector<bytes_t> NextTurnDPoSInfo::GetDPoSPublicKeys() const {
			return _dposPublicKeys;
		}

		size_t NextTurnDPoSInfo::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += sizeof(_workingHeight);
			size += stream.WriteVarUint(_crPublicKeys.size());
			for (size_t i = 0; i < _crPublicKeys.size(); ++i) {
				size += stream.WriteVarUint(_crPublicKeys[i].size());
				size += _crPublicKeys[i].size();
			}

			size += stream.WriteVarUint(_dposPublicKeys.size());
			for (size_t i = 0; i < _dposPublicKeys.size(); ++i) {
				size += stream.WriteVarUint(_dposPublicKeys[i].size());
				size += _dposPublicKeys[i].size();
			}

			return size;
		}

		void NextTurnDPoSInfo::Serialize(ByteStream &stream, uint8_t version) const {
			stream.WriteUint32(_workingHeight);
			stream.WriteVarUint(_crPublicKeys.size());
			for (size_t i = 0; i < _crPublicKeys.size(); ++i)
				stream.WriteVarBytes(_crPublicKeys[i]);
			stream.WriteVarUint(_dposPublicKeys.size());
			for (size_t i = 0; i < _dposPublicKeys.size(); ++i)
				stream.WriteVarBytes(_dposPublicKeys[i]);
		}

		bool NextTurnDPoSInfo::Deserialize(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadUint32(_workingHeight)) {
				Log::error("deserialize working height");
				return false;
			}

			uint64_t len = 0;
			if (!stream.ReadVarUint(len)) {
				Log::error("deserialize crPubKey length");
				return false;
			}
			for (size_t i = 0; i < len; ++i) {
				bytes_t pubkey;
				if (!stream.ReadVarBytes(pubkey)) {
					Log::error("deserialize crPubKeys");
					return false;
				}
				_crPublicKeys.push_back(pubkey);
			}

			len = 0;
			if (!stream.ReadVarUint(len)) {
				Log::error("deserialize dpos pubkey length");
				return false;
			}
			for (size_t i = 0; i < len; ++i) {
				bytes_t pubkey;
				if (!stream.ReadVarBytes(pubkey)) {
					Log::error("deserialize dpos pubkey");
					return false;
				}
				_dposPublicKeys.push_back(pubkey);
			}
			return true;
		}

		nlohmann::json NextTurnDPoSInfo::ToJson(uint8_t version) const {
			nlohmann::json j, crPubKeys = nlohmann::json::array(), dposPubKeys = nlohmann::json::array();

			for (size_t i = 0; i < _crPublicKeys.size(); ++i)
				crPubKeys.push_back(_crPublicKeys[i].getHex());

			for (size_t i = 0; i < _dposPublicKeys.size(); ++i)
				dposPubKeys.push_back(_dposPublicKeys[i].getHex());

			j["WorkingHeight"] = _workingHeight;
			j["CRPublicKeys"] = crPubKeys;
			j["DPoSPublicKeys"] = dposPubKeys;

			return j;
		}

		void NextTurnDPoSInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_workingHeight = j["WorkingHeight"].get<uint32_t>();

			nlohmann::json crPubKeys = j["CRPublicKeys"];
			nlohmann::json dposPubKeys = j["DPoSPublicKeys"];
			_crPublicKeys.clear();
			for (nlohmann::json::iterator it = crPubKeys.begin(); it != crPubKeys.end(); ++it) {
				bytes_t pubkey;
				pubkey.setHex((*it).get<std::string>());
				_crPublicKeys.push_back(pubkey);
			}

			_dposPublicKeys.clear();
			for (nlohmann::json::iterator it = dposPubKeys.begin(); it != dposPubKeys.end(); ++it) {
				bytes_t pubkey;
				pubkey.setHex((*it).get<std::string>());
				_dposPublicKeys.push_back(pubkey);
			}
		}

		IPayload &NextTurnDPoSInfo::operator=(const IPayload &payload) {
			try {
				const NextTurnDPoSInfo &p= dynamic_cast<const NextTurnDPoSInfo &>(payload);
				operator=(p);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of NextTurnDPoSInfo");
			}

			return *this;
		}

		NextTurnDPoSInfo &NextTurnDPoSInfo::operator=(const NextTurnDPoSInfo &payload) {
			_workingHeight = payload._workingHeight;
			_crPublicKeys = payload._crPublicKeys;
			_dposPublicKeys = payload._dposPublicKeys;
			return *this;
		}

		bool NextTurnDPoSInfo::Equal(const IPayload &payload, uint8_t version) const {
			try {
				const NextTurnDPoSInfo &p = dynamic_cast<const NextTurnDPoSInfo &>(payload);
				return _workingHeight == p._workingHeight &&
					   _crPublicKeys == p._crPublicKeys &&
					   _dposPublicKeys == p._dposPublicKeys;
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of NextTurnDPoSInfo");
			}

			return false;
		}
	}
}