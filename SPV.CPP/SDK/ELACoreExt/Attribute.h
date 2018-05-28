// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ATTRIBUTE_H__
#define __ELASTOS_SDK_ATTRIBUTE_H__

#include <boost/shared_ptr.hpp>

#include "ELAMessageSerializable.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class Attribute :
				public ELAMessageSerializable {
		public:
			enum Usage {
				Nonce = 0x00,
				Script = 0x20,
				DescriptionUrl = 0x81,
				Description = 0x90,
			};

		public:
			Attribute();

			Attribute(Usage usage, const CMBlock &data);

			~Attribute();

			virtual void Serialize(ByteStream &ostream) const;
			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &jsonData);

		private:
			Usage _usage;
			CMBlock _data;
		};

		typedef boost::shared_ptr<Attribute> AttributePtr;

	}
}

#endif //__ELASTOS_SDK_ATTRIBUTE_H__
