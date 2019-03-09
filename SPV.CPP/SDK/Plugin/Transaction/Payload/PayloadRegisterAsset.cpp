// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRegisterAsset.h"
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterAsset::PayloadRegisterAsset() :
				_amount(0),
				_controller(UINT168_ZERO) {
			_asset.SetName("ELA");
			_asset.SetPrecision(0x08);
			_asset.SetAssetType(Asset::AssetType::Token);
		}

		PayloadRegisterAsset::PayloadRegisterAsset(const PayloadRegisterAsset &payload) {
			operator=(payload);
		}

		PayloadRegisterAsset::~PayloadRegisterAsset() {

		}

		void PayloadRegisterAsset::SetAsset(const Asset &asset) {
			_asset = asset;
		}

		Asset PayloadRegisterAsset::GetAsset() const {
			return _asset;
		}

		void PayloadRegisterAsset::SetAmount(uint64_t amount) {
			_amount = amount;
		}

		uint64_t PayloadRegisterAsset::GetAmount() const {
			return _amount;
		}

		void PayloadRegisterAsset::SetController(const UInt168 &controller) {
			_controller = controller;
		}

		const UInt168 &PayloadRegisterAsset::GetController() const {
			return _controller;
		}

		bool PayloadRegisterAsset::IsValid() const {
			if(_asset.GetPrecision() < Asset::MinPrecidion || _asset.GetPrecision() > Asset::MinPrecidion) {
				return false;
			}
			return _amount % (uint64_t)pow(10, 8 - _asset.GetPrecision()) != 0;
		}

		void PayloadRegisterAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			_asset.Serialize(ostream);

			ostream.WriteBytes(&_amount, sizeof(_amount));
			ostream.WriteBytes(_controller.u8, sizeof(_controller));
		}

		bool PayloadRegisterAsset::Deserialize(ByteStream &istream, uint8_t version) {
			if (!_asset.Deserialize(istream)) {
				Log::error("Payload register asset deserialize asset fail");
				return false;
			}

			if (!istream.ReadBytes(&_amount, sizeof(_amount))) {
				Log::error("Payload register asset deserialize amount fail");
				return false;
			}

			if (!istream.ReadBytes(_controller.u8, sizeof(_controller))) {
				Log::error("Payload register asset deserialize controller fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRegisterAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["Asset"] = _asset.ToJson();
			j["Amount"] = _amount;
			j["Controller"] = Utils::UInt168ToString(_controller);

			return j;
		}

		void PayloadRegisterAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_asset.FromJson(j["Asset"]);
			_amount = j["Amount"].get<uint64_t>();
			_controller = Utils::UInt168FromString(j["Controller"].get<std::string>());
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