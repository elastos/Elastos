// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADVOTEPRODUCER_H__
#define __ELASTOS_SDK_PAYLOADVOTEPRODUCER_H__

#include <SDK/Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadVoteProducer : public IPayload {
		public:
			PayloadVoteProducer();

			~PayloadVoteProducer();

			const std::string &GetVoter() const;

			void SetVoter(const std::string &voter);

			uint64_t GetStake() const;

			void SetStake(uint64_t stake);

			const std::vector<std::string> &GetPublicKeys() const;

			void SetPublicKeys(const std::vector<std::string> &keys);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

		private:
			std::string _voter;
			uint64_t _stake;
			std::vector<std::string> _publicKeys;
		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADVOTEPRODUCER_H__
