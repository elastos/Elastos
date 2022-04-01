// Copyright (c) 2017-2022 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Common/Log.h>
#include <WalletCore/Address.h>

#include "PayloadStake.h"

namespace Elastos {
    namespace ElaWallet {
        PayloadStake::PayloadStake() {

        }

        PayloadStake::~PayloadStake() {

        }

        PayloadStake::PayloadStake(uint8_t version, const uint168 &stakeAddress) :
            _version(version), _stakeAddress(stakeAddress) {

        }

        size_t PayloadStake::EstimateSize() const {
            size_t size = 0;

            size += sizeof(_version);
            size += _stakeAddress.size();

            return size;
        }

        void PayloadStake::Serialize(ByteStream &stream) const {
            stream.WriteUint8(_version);
            stream.WriteBytes(_stakeAddress);
        }

        bool PayloadStake::Deserialize(const ByteStream &stream) {
            if (!stream.ReadUint8(_version)) {
                Log::error("output payload stake deserialize version");
                return false;
            }

            if (!stream.ReadBytes(_stakeAddress)) {
                Log::error("output payload stake deserialize stake address");
                return false;
            }

            return true;
        }

        nlohmann::json PayloadStake::ToJson() const {
            nlohmann::json j;

            j["Version"] = _version;
            j["StakeAddress"] = Address(_stakeAddress).String();

            return j;
        }

        void PayloadStake::FromJson(const nlohmann::json &j) {
            _version = j["Version"].get<uint8_t>();

            std::string addr = j["StakeAddress"].get<std::string>();
            _stakeAddress = Address(addr).ProgramHash();
        }

        IOutputPayload &PayloadStake::operator=(const IOutputPayload &payload) {
            try {
                const PayloadStake &p = dynamic_cast<const PayloadStake &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadStake");
            }

            return *this;
        }

        PayloadStake &PayloadStake::operator=(const PayloadStake &payload) {
            _version = payload._version;
            _stakeAddress = payload._stakeAddress;

            return *this;
        }

        bool PayloadStake::operator==(const IOutputPayload &payload) const {
            try {
                const PayloadStake &p = dynamic_cast<const PayloadStake &>(payload);
                return _version == p._version && _stakeAddress == p._stakeAddress;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadStake");
            }

            return false;
        }

    }
}
