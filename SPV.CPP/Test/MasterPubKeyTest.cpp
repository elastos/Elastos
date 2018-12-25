// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN


#include "catch.hpp"
#include <SDK/Crypto/MasterPubKey.h>

using namespace Elastos::ElaWallet;

TEST_CASE("MasterPubKey test", "[MasterPubKey]") {
	SECTION("Serialize and deserialize test") {
		MasterPubKey masterPubKey1("闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉", "");
		ByteStream stream;
		masterPubKey1.Serialize(stream);

		stream.setPosition(0);
		MasterPubKey masterPubKey2;
		REQUIRE(masterPubKey2.Deserialize(stream));

		REQUIRE(masterPubKey1.getFingerPrint() == masterPubKey2.getFingerPrint());
		REQUIRE((masterPubKey1.getPubKey() == masterPubKey2.getPubKey()));
		REQUIRE(1 == UInt256Eq(&masterPubKey1.getChainCode(), &masterPubKey2.getChainCode()));
	}
}
