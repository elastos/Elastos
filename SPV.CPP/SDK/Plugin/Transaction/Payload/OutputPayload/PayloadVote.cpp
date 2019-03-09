// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadVote.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadVote::PayloadVote() :
			_version(0) {
		}

		PayloadVote::PayloadVote(const std::vector<VoteContent> &voteContents) :
			_content(voteContents),
			_version(0) {
		}

		PayloadVote::PayloadVote(const PayloadVote &payload){
			operator=(payload);
		}

		PayloadVote::~PayloadVote() {

		}

		void PayloadVote::SetVoteContent(const std::vector<VoteContent> &voteContent){
			_content = voteContent;
		}

		const std::vector<PayloadVote::VoteContent> &PayloadVote::GetVoteContent() const {
			return _content;
		}

		void PayloadVote::Serialize(ByteStream &ostream) const {
			ostream.WriteUint8(_version);

			ostream.writeVarUint(_content.size());
			for (size_t i = 0; i < _content.size(); ++i) {
				ostream.WriteUint8(_content[i].type);
				const std::vector<CMBlock> &candidates = _content[i].candidates;
				ostream.writeVarUint(candidates.size());
				for (size_t j = 0; j < candidates.size(); ++j) {
					ostream.WriteVarBytes(candidates[j]);
				}
			}
		}

		bool PayloadVote::Deserialize(ByteStream &istream) {
			if (!istream.ReadUint8(_version)) {
				Log::error("payload vote deserialize version error");
				return false;
			}

			size_t contentCount = 0;
			if (!istream.readVarUint(contentCount)) {
				Log::error("payload vote deserialize content count error");
				return false;
			}

			_content.clear();
			for (size_t i = 0; i < contentCount; ++i) {
				uint8_t type;
				if (!istream.ReadUint8(type)) {
					Log::error("payload vote deserialize type error");
					return false;
				}
				size_t candidateCount = 0;
				if (!istream.readVarUint(candidateCount)) {
					Log::error("payload vote deserialize candidate count error");
					return false;
				}

				std::vector<CMBlock> candidates;
				for (size_t j = 0; j < candidateCount; ++j) {
					CMBlock candidate;
					if (!istream.ReadVarBytes(candidate)) {
						Log::error("payload vote deserialize candidate error");
						return false;
					}
					candidates.push_back(candidate);
				}

				_content.emplace_back(Type(type), candidates);
			}

			return true;
		}

		nlohmann::json PayloadVote::ToJson() const {
			nlohmann::json j;
			j["Version"] = _version;
			std::vector<nlohmann::json> voteContent;
			for (size_t i = 0; i < _content.size(); ++i) {
				nlohmann::json content;
				content["Type"] = _content[i].type;
				std::vector<std::string> candidates;
				for (size_t j = 0; j < _content[i].candidates.size(); ++j) {
					candidates.push_back(Utils::EncodeHex(_content[i].candidates[j]));
				}
				content["Candidates"] = candidates;
				voteContent.push_back(content);
			}
			j["VoteContent"] = voteContent;

			return j;
		}

		void PayloadVote::FromJson(const nlohmann::json &j) {
			_version = j["Version"];
			_content.clear();
			nlohmann::json voteContent = j["VoteContent"];
			for (nlohmann::json::iterator it = voteContent.begin(); it != voteContent.end(); ++it) {
				Type type = (*it)["Type"];
				std::vector<CMBlock> candidates;
				nlohmann::json candidatesJson = (*it)["Candidates"];
				for (nlohmann::json::iterator cit = candidatesJson.begin(); cit != candidatesJson.end(); ++cit) {
					candidates.push_back(Utils::DecodeHex(*cit));
				}
				_content.emplace_back(type, candidates);
			}
		}

		IOutputPayload &PayloadVote::operator=(const IOutputPayload &payload) {
			try {
				const PayloadVote &payloadVote = dynamic_cast<const PayloadVote &>(payload);
				operator=(payloadVote);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadVote");
			}

			return *this;
		}

		PayloadVote& PayloadVote::operator=(const PayloadVote &payload) {
			_version = payload._version;
			_content = payload._content;

			return *this;
		}

	}
}
