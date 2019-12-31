// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinBase.h"
#include <Common/Utils.h>
#include <Common/Log.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {
		CoinBase::CoinBase() {

		}

		CoinBase::CoinBase(const bytes_t &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		CoinBase::CoinBase(const CoinBase &payload) {
			operator=(payload);
		}

		CoinBase::~CoinBase() {
		}

		void CoinBase::SetCoinBaseData(const bytes_t &coinBaseData) {
			_coinBaseData = coinBaseData;
		}

		const bytes_t &CoinBase::GetCoinBaseData() const {
			return _coinBaseData;
		}

		size_t CoinBase::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_coinBaseData.size());
			size += _coinBaseData.size();

			return size;
		}

		void CoinBase::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_coinBaseData);
		}

		bool CoinBase::Deserialize(const ByteStream &istream, uint8_t version) {
			return istream.ReadVarBytes(_coinBaseData);
		}

		nlohmann::json CoinBase::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["CoinBaseData"] = _coinBaseData.getHex();
			return j;
		}

		void CoinBase::FromJson(const nlohmann::json &j, uint8_t version) {
			_coinBaseData.setHex(j["CoinBaseData"].get<std::string>());

		}

		IPayload &CoinBase::operator=(const IPayload &payload) {
			try {
				const CoinBase &payloadCoinBase = dynamic_cast<const CoinBase &>(payload);
				operator=(payloadCoinBase);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CoinBase");
			}

			return *this;
		}

		CoinBase &CoinBase::operator=(const CoinBase &payload) {
			_coinBaseData = payload._coinBaseData;
			return *this;
		}

	}
}