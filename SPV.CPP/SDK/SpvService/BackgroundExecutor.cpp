// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BackgroundExecutor.h"
#include <SDK/Common/Log.h>

using namespace boost;
using namespace asio;

namespace Elastos {
	namespace ElaWallet {

		BackgroundExecutor::BackgroundExecutor(uint8_t threadCount) {
			initThread(threadCount);
		}

		BackgroundExecutor::~BackgroundExecutor() {
			stopThread();
		}

		void BackgroundExecutor::execute(const Runnable &runnable) {
			_workerService.post(runnable.Closure);
		}

		void BackgroundExecutor::initThread(uint8_t threadCount) {
			_workerService.restart();
			_workerLoop = boost::shared_ptr<io_service::work>(new io_service::work(_workerService));

			for (uint8_t i = 0; i < threadCount; ++i) {
				_workerThreadPool.create_thread(boost::bind(&io_service::run, &_workerService));
			}
		}

		void BackgroundExecutor::stopThread() {
			if (!_workerService.stopped()) {
				_workerService.stop();
				while (!_workerService.stopped());
			}
			_workerThreadPool.join_all();
		}

	}
}