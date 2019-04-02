// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_EXECUTOR_H__
#define __ELASTOS_SDK_EXECUTOR_H__

#include <boost/function.hpp>

namespace Elastos {
	namespace ElaWallet {

		struct Runnable {
			Runnable(const boost::function<void()> &closure) :
				Closure(closure) {
			}

			boost::function<void()> Closure;
		};

		class Executor {
		public:
			virtual void Execute(const Runnable &runnable) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_EXECUTOR_H__
