// Copyright (c) 2017-2022 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELASTOS_SPVSDK_PAYLOADSTAKE_H
#define ELASTOS_SPVSDK_PAYLOADSTAKE_H

#include "IOutputPayload.h"

namespace Elastos {
    namespace ElaWallet {

        class PayloadStake : public IOutputPayload {
        public:
            PayloadStake();

            ~PayloadStake();

        public:
            virtual size_t EstimateSize() const;

            virtual void Serialize(ByteStream &stream) const;

            virtual bool Deserialize(const ByteStream &stream);

            virtual nlohmann::json ToJson() const;

            virtual void FromJson(const nlohmann::json &j);

            virtual IOutputPayload &operator=(const IOutputPayload &payload);

            PayloadStake &operator=(const PayloadStake &payload);

            virtual bool operator==(const IOutputPayload &payload) const;

        private:
            uint8_t _version;
            uint168 _stakeAddress; // address of stake owner，begin with 'S'
        };

    }
}

#endif
