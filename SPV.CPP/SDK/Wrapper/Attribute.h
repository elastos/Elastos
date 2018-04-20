// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ATTRIBUTE_H__
#define __ELASTOS_SDK_ATTRIBUTE_H__

#include <boost/shared_ptr.hpp>

#include "ByteData.h"
#include "ELAMessageSerializable.h"

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

			Attribute(Usage usage, const ByteData &data);

			~Attribute();

			virtual void Serialize(std::istream &istream) const;
			virtual void Deserialize(std::ostream &ostream);

		private:
			Usage _usage;
			ByteData _data;
		};

		typedef boost::shared_ptr<Attribute> AttributePtr;

	}
}

#endif //__ELASTOS_SDK_ATTRIBUTE_H__
