// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRegisterAsset.h"

namespace Elastos {
	namespace SDK {

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

		ByteData PayloadRegisterAsset::getData() const {
			ByteStream byteStream;
			Serialize(byteStream);
			byteStream.setPosition(0);
			uint8_t *data = byteStream.getBuf();
			ssize_t len = byteStream.length();
			return ByteData(data, len);
		}

		void PayloadRegisterAsset::Serialize(ByteStream &ostream) const {
			_asset.Serialize(ostream);

			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, _amount);
			ostream.putBytes(amountData, sizeof(amountData));

			uint8_t controllerData[168 / 8];
			UInt168Set(controllerData, _controller);
			ostream.putBytes(controllerData, sizeof(controllerData));
		}

		void PayloadRegisterAsset::Deserialize(ByteStream &istream) {
			_asset.Deserialize(istream);

			uint8_t amountData[64 / 8];
			istream.getBytes(amountData, sizeof(amountData));
			_amount = UInt64GetLE(amountData);

			uint8_t controllerData[168 / 8];
			istream.getBytes(controllerData, sizeof(controllerData));
			UInt168Get(&_controller, controllerData);
		}
	}
}