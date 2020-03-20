// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ATTRIBUTE_H__
#define __ELASTOS_SDK_ATTRIBUTE_H__

#include <Plugin/Interface/ELAMessageSerializable.h>

#include <boost/shared_ptr.hpp>
#include <Common/JsonSerializer.h>

namespace Elastos {
	namespace ElaWallet {

		class Attribute :
				public ELAMessageSerializable, public JsonSerializer {
		public:
			enum Usage {
				Nonce = 0x00,
				Script = 0x20,
				DescriptionUrl = 0x91,
				Description = 0x90,
				Memo = 0x81,
				Confirmations = 0x92
			};

		public:
			Attribute();

			Attribute(const Attribute &attr);

			Attribute(Usage usage, const bytes_t &data);

			~Attribute();

			Attribute&operator=(const Attribute &attr);

			Usage GetUsage() const;

			const bytes_t &GetData() const;

			bool IsValid() const;

			virtual size_t EstimateSize() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		private:
			Usage _usage;
			bytes_t _data;
		};

		typedef boost::shared_ptr<Attribute> AttributePtr;
		typedef std::vector<AttributePtr> AttributeArray;

	}
}

#endif //__ELASTOS_SDK_ATTRIBUTE_H__
