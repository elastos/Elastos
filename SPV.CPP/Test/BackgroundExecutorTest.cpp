// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <SpvService/BackgroundExecutor.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE( "BackgroundExecutor simple test", "[Normal]" ) {
	Log::registerMultiLogger();

	SECTION("Single thread test (default)") {
		BackgroundExecutor executor;

		const int expectValue = 3;
		int actualValue = 1;
		executor.Execute(Runnable([&actualValue, expectValue]() -> void {
			actualValue = expectValue;
		}));

		sleep(1);
		REQUIRE(actualValue == expectValue);
	}
	SECTION("Two thread test") {
		BackgroundExecutor executor(2);

		const int expectValue = 3;
		bool finished = false;

		std::vector<int> array(100, 1);
		for (int i = 0; i < array.size(); ++i) {
			executor.Execute(Runnable([&array, i, expectValue]() -> void {
				array[i] = expectValue;
			}));
		}

		executor.Execute(Runnable([&finished]() -> void {
			finished = true;
		}));

		sleep(1); //todo should not add this line
		while (!finished);

		for (int i = 0; i < array.size(); ++i) {
			REQUIRE(array[i] == expectValue);
		}
	}
}