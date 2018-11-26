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
			_asset.setName("ELA");
			_asset.setPrecision(0x08);
			_asset.setAssetType(Asset::AssetType::Token);
		}

		PayloadRegisterAsset::~PayloadRegisterAsset() {

		}

		void PayloadRegisterAsset::setAsset(const Asset &asset) {
			_asset = asset;
		}

		Asset PayloadRegisterAsset::getAsset() const {
			return _asset;
		}

		void PayloadRegisterAsset::setAmount(uint64_t amount) {
			_amount = amount;
		}

		uint64_t PayloadRegisterAsset::getAmount() const {
			return _amount;
		}

		void PayloadRegisterAsset::setController(const UInt168 &controller) {
			_controller = controller;
		}

		const UInt168 &PayloadRegisterAsset::getController() const {
			return _controller;
		}

		bool PayloadRegisterAsset::isValid() const {
			if(_asset.getPrecision() < Asset::MinPrecidion || _asset.getPrecision() > Asset::MinPrecidion) {
				return false;
			}
			return _amount % (uint64_t)pow(10, 8 - _asset.getPrecision()) != 0;
		}

		CMBlock PayloadRegisterAsset::getData() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer();
		}

		void PayloadRegisterAsset::Serialize(ByteStream &ostream) const {
			_asset.Serialize(ostream);

			ostream.writeBytes(&_amount, sizeof(_amount));
			ostream.writeBytes(_controller.u8, sizeof(_controller));
		}

		bool PayloadRegisterAsset::Deserialize(ByteStream &istream) {
			if (!_asset.Deserialize(istream)) {
				Log::error("Payload register asset deserialize asset fail");
				return false;
			}

			if (!istream.readBytes(&_amount, sizeof(_amount))) {
				Log::error("Payload register asset deserialize amount fail");
				return false;
			}

			if (!istream.readBytes(_controller.u8, sizeof(_controller))) {
				Log::error("Payload register asset deserialize controller fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRegisterAsset::toJson() const {
			nlohmann::json j;

			j["Asset"] = _asset.toJson();
			j["Amount"] = _amount;
			j["Controller"] = Utils::UInt168ToString(_controller);

			return j;
		}

		void PayloadRegisterAsset::fromJson(const nlohmann::json &j) {
			_asset.fromJson(j["Asset"]);
			_amount = j["Amount"].get<uint64_t>();
			_controller = Utils::UInt168FromString(j["Controller"].get<std::string>());
		}
	}
}