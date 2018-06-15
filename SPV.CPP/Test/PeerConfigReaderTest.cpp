// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "nlohmann/json.hpp"

#include "Core/BRPeer.h"

#include "PeerConfigReader.h"

using namespace Elastos::SDK;

TEST_CASE("Simple json sample test", "[PeerConfigReader]") {
	nlohmann::json j = "{\n"
					   "    \"MagicNumber\": 7630401,\n"
					   "    \"KnowingPeers\": [\n"
					   "        {\n"
					   "            \"Address\": \"127.0.0.1\",\n"
					   "            \"Port\": 20866,\n"
					   "            \"Timestamp\": 0,\n"
					   "            \"Services\": 1,\n"
					   "            \"Flags\": 0\n"
					   "        }\n"
					   "    ]\n"
					   "}"_json;
	PeerConfigReader reader;
	SharedWrapperList<Peer, BRPeer *> peers = reader.readPeersFromJson(j);
	REQUIRE(peers.size() == 1);
	REQUIRE(peers[0]->getPort() == 20866);
	UInt128 address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 127, 0, 0, 1});
	REQUIRE(UInt128Eq(&peers[0]->getRaw()->address, &address));
	REQUIRE(peers[0]->getTimestamp() != 0);
	REQUIRE(peers[0]->getRaw()->services == 1);
	REQUIRE(peers[0]->getRaw()->flags == 0);
}