// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_LOCKABLE_H__
#define __ELASTOS_SDK_LOCKABLE_H__

#include <boost/thread/mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Lockable {
		public:
			void Lock() const { lock.lock(); }

			void Unlock() const { lock.unlock(); }

		protected:
			mutable boost::mutex lock;
		};

	}
}

#endif //__ELASTOS_SDK_LOCKABLE_H__
