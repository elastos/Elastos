// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRegisterAsset.h"
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Asset.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterAsset::PayloadRegisterAsset() :
				_amount(0),
				_asset(new Asset()) {
		}

		PayloadRegisterAsset::PayloadRegisterAsset(const AssetPtr &asset, uint64_t amount, const uint168 &controller) :
			_amount(amount),
			_asset(asset),
			_controller(controller) {
		}

		PayloadRegisterAsset::PayloadRegisterAsset(const PayloadRegisterAsset &payload) {
			operator=(payload);
		}

		PayloadRegisterAsset::~PayloadRegisterAsset() {

		}

		bool PayloadRegisterAsset::IsValid() const {
			return (_asset->GetPrecision() <= Asset::MaxPrecision);
		}

		void PayloadRegisterAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			_asset->Serialize(ostream);
			ostream.WriteBytes(&_amount, sizeof(_amount));
			ostream.WriteBytes(_controller);
		}

		bool PayloadRegisterAsset::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!_asset->Deserialize(istream)) {
				Log::error("Payload register asset deserialize asset fail");
				return false;
			}

			if (!istream.ReadBytes(&_amount, sizeof(_amount))) {
				Log::error("Payload register asset deserialize amount fail");
				return false;
			}

			if (!istream.ReadBytes(_controller)) {
				Log::error("Payload register asset deserialize controller fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRegisterAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["Asset"] = _asset->ToJson();
			j["Amount"] = _amount;
			j["Controller"] = _controller.GetHex();

			return j;
		}

		void PayloadRegisterAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_asset->FromJson(j["Asset"]);
			_amount = j["Amount"].get<uint64_t>();
			_controller.SetHex(j["Controller"].get<std::string>());
		}

		IPayload &PayloadRegisterAsset::operator=(const IPayload &payload) {
			try {
				const PayloadRegisterAsset &payloadRegisterAsset = dynamic_cast<const PayloadRegisterAsset &>(payload);
				operator=(payloadRegisterAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadRegisterAsset");
			}

			return *this;
		}

		PayloadRegisterAsset &PayloadRegisterAsset::operator=(const PayloadRegisterAsset &payload) {
			_asset = payload._asset;
			_amount = payload._amount;
			_controller = payload._controller;

			return *this;
		}

	}
}