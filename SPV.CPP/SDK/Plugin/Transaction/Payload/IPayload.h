// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IPAYLOAD_H__
#define __ELASTOS_SDK_IPAYLOAD_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Common/CMemBlock.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IPayload :
				public ELAMessageSerializable {
		public:
			virtual ~IPayload();

			virtual CMBlock getData() const;

			virtual bool isValid() const;
		};

		typedef boost::shared_ptr<IPayload> PayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IPAYLOAD_H__
