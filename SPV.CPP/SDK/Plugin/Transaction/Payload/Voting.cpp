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
#include <Common/Log.h>
#include <Common/JsonSerializer.h>
#include "Voting.h"

namespace Elastos {
    namespace ElaWallet {

        // VotesWithLockTime
        VotesWithLockTime::VotesWithLockTime() {
        }

        VotesWithLockTime::~VotesWithLockTime() {
        }

        VotesWithLockTime::VotesWithLockTime(const bytes_t &candidate, uint64_t votes, uint32_t lockTime) :
            _candidate(candidate), _votes(votes), _lockTime(lockTime) {
        }

        size_t VotesWithLockTime::EstimateSize(uint8_t version) const {
            size_t size = 0;
            ByteStream stream;

            size += stream.WriteVarUint(_candidate.size());
            size += _candidate.size();
            size += sizeof(_votes);
            size += sizeof(_lockTime);

            return size;
        }

        void VotesWithLockTime::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteVarBytes(_candidate);
            stream.WriteUint64(_votes);
            stream.WriteUint32(_lockTime);
        }

        bool VotesWithLockTime::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadVarBytes(_candidate)) {
                Log::error("VotesWithLockTime deserialize candidate");
                return false;
            }

            if (!stream.ReadUint64(_votes)) {
                Log::error("VotesWithLockTime deserialize votes");
                return false;
            }

            if (!stream.ReadUint32(_lockTime)) {
                Log::error("VotesWithLockTime deserialize locktime");
                return false;
            }

            return true;
        }

        bool VotesWithLockTime::Equal(const VotesWithLockTime &vwl, uint8_t version) const {
            return _candidate == vwl._candidate && _votes == vwl._votes && _lockTime == vwl._lockTime;
        }

        void from_json(const nlohmann::json &j, VotesWithLockTime &v) {
            v._candidate.setHex(j.at("Candidate").get<std::string>());
            j.at("Votes").get_to(v._votes);
            j.at("Locktime").get_to(v._lockTime);
        }

        void to_json(nlohmann::json &j, const VotesWithLockTime &v) {
            j = nlohmann::json {
                    {"Candidate", v._candidate.getHex()},
                    {"Votes", v._votes},
                    {"Locktime", v._lockTime}
            };
        }

        // VotesContent
        VotesContent::VotesContent() {

        }

        VotesContent::~VotesContent() {

        }

        VotesContent::VotesContent(uint8_t voteType, const std::vector<VotesWithLockTime> &votesInfo) :
            _voteType(voteType),
            _votesInfo(votesInfo) {
        }

        size_t VotesContent::EstimateSize(uint8_t version) const {
            size_t size = 0;
            ByteStream stream;

            size += sizeof(_voteType);
            size += stream.WriteVarUint(_votesInfo.size());
            for (size_t i = 0; i < _votesInfo.size(); ++i) {
                size += _votesInfo[i].EstimateSize(version);
            }

            return size;
        }

        void VotesContent::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteByte(_voteType);
            stream.WriteVarUint(_votesInfo.size());
            for (const VotesWithLockTime &info : _votesInfo) {
                info.Serialize(stream, version);
            }
        }

        bool VotesContent::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadByte(_voteType)) {
                Log::error("deserialize VotesContent vote type");
                return false;
            }

            uint64_t size = 0;
            if (!stream.ReadVarUint(size)) {
                Log::error("deserialize vote info size");
                return false;
            }

            for (size_t i = 0; i < size; ++i) {
                VotesWithLockTime vinfo;
                if (!vinfo.Deserialize(stream, version)) {
                    Log::error("deserialize voteinfo[{}]", i);
                    return false;
                }
                _votesInfo.push_back(vinfo);
            }

            return true;
        }

        bool VotesContent::Equal(const VotesContent &vc, uint8_t version) const {
            if (_voteType != vc._voteType)
                return false;

            if (_votesInfo.size() != vc._votesInfo.size())
                return false;

            for (size_t i = 0; i < _votesInfo.size(); ++i) {
                if (!_votesInfo[i].Equal(vc._votesInfo[i], version))
                    return false;
            }

            return true;
        }

        void from_json(const nlohmann::json &j, VotesContent &vc) {
            j.at("VoteType").get_to(vc._voteType);
            j.at("VotesInfo").get_to(vc._votesInfo);
        }

        void to_json(nlohmann::json &j, const VotesContent &vc) {
            j = nlohmann::json {
                    {"VoteType", vc._voteType},
                    {"VotesInfo", vc._votesInfo}
            };
        }

        // RenewalVotesContent
        RenewalVotesContent::RenewalVotesContent() {
        }

        RenewalVotesContent::~RenewalVotesContent() {
        }

        RenewalVotesContent::RenewalVotesContent(const uint256 &referKey, const VotesWithLockTime &voteInfo) :
            _referKey(referKey), _voteInfo(voteInfo) {

        }

        size_t RenewalVotesContent::EstimateSize(uint8_t version) const {
            size_t size = 0;

            size += _referKey.size();
            size += _voteInfo.EstimateSize(version);

            return size;
        }

        void RenewalVotesContent::Serialize(ByteStream &stream, uint8_t version) const {
            stream.WriteBytes(_referKey);
            _voteInfo.Serialize(stream, version);
        }

        bool RenewalVotesContent::Deserialize(const ByteStream &stream, uint8_t version) {
            if (!stream.ReadBytes(_referKey)) {
                Log::error("RenewalVotesContent deserialize refer key");
                return false;
            }

            if (!_voteInfo.Deserialize(stream, version)) {
                Log::error("RenewalVotesContent deserialize vote info");
                return false;
            }

            return true;
        }

        bool RenewalVotesContent::Equal(const RenewalVotesContent &rvc, uint8_t version) const {
            return _referKey == rvc._referKey && _voteInfo.Equal(rvc._voteInfo, version);
        }

        void from_json(const nlohmann::json &j, RenewalVotesContent &rvc) {
            j.at("ReferKey").get_to(rvc._referKey);
            j.at("VoteInfo").get_to(rvc._voteInfo);
        }

        void to_json(nlohmann::json &j, const RenewalVotesContent &rvc) {
            j = nlohmann::json{
                    {"ReferKey", rvc._referKey},
                    {"VoteInfo", rvc._voteInfo}
            };
        }

        // Voting
        Voting::Voting() {

        }

        Voting::~Voting() {

        }

        Voting::Voting(const std::vector<VotesContent> &contents,
                       const std::vector<RenewalVotesContent> &renewalVotesContent) :
                       _contents(contents), _renewalVotesContent(renewalVotesContent) {

        }

        size_t Voting::EstimateSize(uint8_t version) const {
            size_t size = 0;
            ByteStream stream;

            if (version == VoteVersion) {
                size += stream.WriteVarUint(_contents.size());
                for (size_t i = 0; i < _contents.size(); ++i) {
                    size += _contents[i].EstimateSize(version);
                }
            } else if (version == RenewalVoteVersion) {
                size += stream.WriteVarUint(_renewalVotesContent.size());
                for (size_t i = 0; i < _renewalVotesContent.size(); ++i) {
                    size += _renewalVotesContent[i].EstimateSize(version);
                }
            }

            return size;
        }

        void Voting::Serialize(ByteStream &stream, uint8_t version) const {
            if (version == VoteVersion) {
                stream.WriteVarUint(_contents.size());
                for (const VotesContent &vc : _contents)
                    vc.Serialize(stream, version);
            } else if (version == RenewalVoteVersion) {
                stream.WriteVarUint(_renewalVotesContent.size());
                for (const RenewalVotesContent &rvc : _renewalVotesContent)
                    rvc.Serialize(stream, version);
            }
        }

        bool Voting::Deserialize(const ByteStream &stream, uint8_t version) {
            uint64_t size = 0;
            if (version == VoteVersion) {
                if (!stream.ReadVarUint(size)) {
                    Log::error("voting deserialize contents size");
                    return false;
                }
                for (size_t i = 0; i < size; ++i) {
                    VotesContent vc;
                    if (!vc.Deserialize(stream, version)) {
                        Log::error("voting deserialize VotesContent[{}]", i);
                        return false;
                    }
                    _contents.push_back(vc);
                }
            } else if (version == RenewalVoteVersion) {
                if (!stream.ReadVarUint(size)) {
                    Log::error("voting deserialize RenewalVotesContent size");
                    return false;
                }
                for (size_t i = 0; i < size; ++i) {
                    RenewalVotesContent rvc;
                    if (!rvc.Deserialize(stream, version)) {
                        Log::error("voting deserialize RenewalVotesContent[{}]", i);
                        return false;
                    }
                    _renewalVotesContent.push_back(rvc);
                }
            }

            return true;
        }

        nlohmann::json Voting::ToJson(uint8_t version) const {
            if (version == VoteVersion) {
                return nlohmann::json {
                        {"Contents", _contents}
                };
            } else if (version == RenewalVoteVersion) {
                return nlohmann::json {
                        {"RenewalVotesContent", _renewalVotesContent}
                };
            } else {
                return nlohmann::json();
            }
        }

        void Voting::FromJson(const nlohmann::json &j, uint8_t version) {
            if (version == VoteVersion) {
                j.at("Contents").get_to(_contents);
            } else if (version == RenewalVoteVersion) {
                j.at("RenewalVotesContent").get_to(_renewalVotesContent);
            }
        }

        bool Voting::IsValid(uint8_t version) const {
            return true;
        }

        IPayload &Voting::operator=(const IPayload &payload) {
            try {
                const Voting &p= dynamic_cast<const Voting&>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Voting");
            }

            return *this;
        }

        Voting &Voting::operator=(const Voting &payload) {
            _contents = payload._contents;
            _renewalVotesContent = payload._renewalVotesContent;
            return *this;
        }

        bool Voting::Equal(const IPayload &payload, uint8_t version) const {
            try {
                const Voting &p = dynamic_cast<const Voting&>(payload);
                if (version == VoteVersion) {
                    if (_contents.size() != p._contents.size())
                        return false;

                    for (size_t i = 0; i < _contents.size(); ++i) {
                        if (!_contents[i].Equal(p._contents[i], version))
                            return false;
                    }
                } else if (version == RenewalVoteVersion) {
                    if (_renewalVotesContent.size() != p._renewalVotesContent.size())
                        return false;

                    for (size_t i = 0; i < _renewalVotesContent.size(); ++i) {
                        if (!_renewalVotesContent[i].Equal(p._renewalVotesContent[i], version))
                            return false;
                    }
                }
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of Voting");
                return false;
            }

            return true;
        }

    }
}