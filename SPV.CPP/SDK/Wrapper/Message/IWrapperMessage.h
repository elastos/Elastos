// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IWRAPPERMESSAGE_H__
#define __ELASTOS_SDK_IWRAPPERMESSAGE_H__

#include "IMessage.h"

namespace Elastos {
	namespace SDK {

		class IWrapperMessage :
			public IMessage {
		public:
			IWrapperMessage();

			virtual ~IWrapperMessage();


		};

	}
}

#endif //__ELASTOS_SDK_IWRAPPERMESSAGE_H__
