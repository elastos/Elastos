/*
 * Copyright (c) 2022 Elastos Foundation LTD.
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
#ifndef __ELASTOS_SPVSDK_VOTING_H__
#define __ELASTOS_SPVSDK_VOTING_H__

#include "IPayload.h"

namespace Elastos {
    namespace ElaWallet {

#define VoteVersion 0
#define RenewalVoteVersion 1

        class VotesWithLockTime {
        public:
            VotesWithLockTime();

            ~VotesWithLockTime();

            size_t EstimateSize(uint8_t version) const;

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            bool Equal(const VotesWithLockTime &vwl, uint8_t version) const;

            friend void from_json(const nlohmann::json &j, VotesWithLockTime &v);

            friend void to_json(nlohmann::json &j, const VotesWithLockTime &v);

        private:
            bytes_t _candidate;
            uint64_t _votes;
            uint32_t _lockTime;
        };

        extern void from_json(const nlohmann::json &j, VotesWithLockTime &v);

        extern void to_json(nlohmann::json &j, const VotesWithLockTime &v);


        class VotesContent {
        public:
            VotesContent();

            ~VotesContent();

            size_t EstimateSize(uint8_t version) const;

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            bool Equal(const VotesContent &vc, uint8_t version) const;

            friend void from_json(const nlohmann::json &j, VotesContent &vc);

            friend void to_json(nlohmann::json &j, const VotesContent &vc);
        private:
            uint8_t _voteType;
            std::vector<VotesWithLockTime> _votesInfo;
        };

        extern void from_json(const nlohmann::json &j, VotesContent &vc);

        extern void to_json(nlohmann::json &j, const VotesContent &vc);


        class RenewalVotesContent {
        public:
            RenewalVotesContent();

            ~RenewalVotesContent();

            size_t EstimateSize(uint8_t version) const;

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            bool Equal(const RenewalVotesContent &rvc, uint8_t version) const;

            friend void from_json(const nlohmann::json &j, RenewalVotesContent &rvc);

            friend void to_json(nlohmann::json &j, const RenewalVotesContent &rvc);
        private:
            uint256 _referKey;
            VotesWithLockTime _voteInfo;
        };

        extern void from_json(const nlohmann::json &j, RenewalVotesContent &rvc);

        extern void to_json(nlohmann::json &j, const RenewalVotesContent &rvc);


        class Voting : public IPayload {
        public:
            Voting();

            ~Voting();

        public:
            virtual size_t EstimateSize(uint8_t version) const;

            virtual void Serialize(ByteStream &stream, uint8_t version) const;

            virtual bool Deserialize(const ByteStream &stream, uint8_t version);

            virtual nlohmann::json ToJson(uint8_t version) const;

            virtual void FromJson(const nlohmann::json &j, uint8_t version);

            virtual bool IsValid(uint8_t version) const;

            virtual IPayload &operator=(const IPayload &payload);

            Voting &operator=(const Voting &payload);

            virtual bool Equal(const IPayload &payload, uint8_t version) const;

        private:
            std::vector<VotesContent> _contents; // 投票
            std::vector<RenewalVotesContent> _renewalVotesContent; // 续期
        };

    }
}

#endif
