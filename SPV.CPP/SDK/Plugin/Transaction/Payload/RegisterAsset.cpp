// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RegisterAsset.h"
#include <Common/Log.h>
#include <Common/Utils.h>
#include <Plugin/Transaction/Asset.h>

namespace Elastos {
	namespace ElaWallet {

		RegisterAsset::RegisterAsset() :
				_amount(0),
				_asset(new Asset()) {
		}

		RegisterAsset::RegisterAsset(const AssetPtr &asset, uint64_t amount, const uint168 &controller) :
			_amount(amount),
			_asset(asset),
			_controller(controller) {
		}

		RegisterAsset::RegisterAsset(const RegisterAsset &payload) {
			operator=(payload);
		}

		RegisterAsset::~RegisterAsset() {

		}

		bool RegisterAsset::IsValid(uint8_t version) const {
			return (_asset->GetPrecision() <= Asset::MaxPrecision);
		}

		size_t RegisterAsset::EstimateSize(uint8_t version) const {
			size_t size = 0;

			size += _asset->EstimateSize();
			size += sizeof(_amount);
			size += _controller.size();

			return size;
		}

		void RegisterAsset::Serialize(ByteStream &ostream, uint8_t version) const {
			_asset->Serialize(ostream);
			ostream.WriteBytes(&_amount, sizeof(_amount));
			ostream.WriteBytes(_controller);
		}

		bool RegisterAsset::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json RegisterAsset::ToJson(uint8_t version) const {
			nlohmann::json j;

			j["Asset"] = _asset->ToJson();
			j["Amount"] = _amount;
			j["Controller"] = _controller.GetHex();

			return j;
		}

		void RegisterAsset::FromJson(const nlohmann::json &j, uint8_t version) {
			_asset->FromJson(j["Asset"]);
			_amount = j["Amount"].get<uint64_t>();
			_controller.SetHex(j["Controller"].get<std::string>());
		}

		IPayload &RegisterAsset::operator=(const IPayload &payload) {
			try {
				const RegisterAsset &payloadRegisterAsset = dynamic_cast<const RegisterAsset &>(payload);
				operator=(payloadRegisterAsset);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of RegisterAsset");
			}

			return *this;
		}

		RegisterAsset &RegisterAsset::operator=(const RegisterAsset &payload) {
			_asset = payload._asset;
			_amount = payload._amount;
			_controller = payload._controller;

			return *this;
		}

	}
}