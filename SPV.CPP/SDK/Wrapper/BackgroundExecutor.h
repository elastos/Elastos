// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BACKGROUNDEXECUTOR_H__
#define __ELASTOS_SDK_BACKGROUNDEXECUTOR_H__

#include <queue>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "Executor.h"

namespace Elastos {
	namespace SDK {

		class BackgroundExecutor :
			public Executor {
		public:
			BackgroundExecutor(uint8_t threadCount = 1);

			virtual ~BackgroundExecutor();

			virtual void execute(const Runnable &runnable);

		protected:

			void initThread(uint8_t threadCount);

			void stopThread();

		protected:
			boost::asio::io_service _workerService;
			boost::thread_group _workerThreadPool;
			boost::shared_ptr<boost::asio::io_service::work> _workerLoop;
		};

	}
}

#endif //__ELASTOS_SDK_SINGLETHREADEXECUTOR_H__
