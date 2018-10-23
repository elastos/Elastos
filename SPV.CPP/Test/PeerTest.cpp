// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRPeer.h>

#include "catch.hpp"
#include "SDK/P2P/Peer.h"

using namespace Elastos::ElaWallet;

TEST_CASE( "Peer construct test", "[PeerConstruct]" ) {

	SECTION("Construct with add, port, timestamp or peer") {

		UInt128 addr = *(UInt128 *)"\x0b\x3b\x20\xea\xf1\x69\x64\x62\xf5\x0d\x1a\x3b\xbd\xd3\x0c\xef";
		uint16_t port = 8080;
		uint64_t timestamp = time(NULL);
		Peer peer = Peer(addr, port, timestamp);

		SECTION("Construct with (add, port, timestamp)") {
			REQUIRE(peer.getRaw() != nullptr);
			REQUIRE(0 == memcmp(peer.getAddress().u8, addr.u8, sizeof(addr)));
			REQUIRE(port == peer.getPort());
			REQUIRE(timestamp == peer.getTimestamp());
		}

		SECTION("Construct with (peer)") {
			Peer p = Peer(peer);

			REQUIRE(p.getRaw() != nullptr);
			REQUIRE(0 == memcmp(p.getAddress().u8, addr.u8, sizeof(addr)));
			REQUIRE(port == p.getPort());
			REQUIRE(timestamp == p.getTimestamp());
		}

	}

	SECTION("Construct with magicNumber") {

		uint32_t magicNumber = 0x12345678;
		Peer peer = Peer(magicNumber);
		REQUIRE(peer.getRaw() != nullptr);

		UInt128 addr = *(UInt128 *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xbd\xd3\x0c\xef";
		peer.getRaw()->address = addr;
		REQUIRE(0 == memcmp(peer.getAddress().u8, addr.u8, sizeof(addr)));
		REQUIRE(0 == peer.getPort());
		REQUIRE(0 == peer.getTimestamp());
		REQUIRE("189.211.12.239" == peer.getHost());
		REQUIRE(Peer::Disconnected == peer.GetConnectStatus());
		REQUIRE(0 == peer.getVersion());
		REQUIRE("" == peer.getUserAgent());

	}

}
