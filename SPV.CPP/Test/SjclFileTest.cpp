// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <nlohmann/json.hpp>
#include <SDK/KeyStore/SjclFile.h>

#include <fstream>
#include <boost/filesystem.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Json convert", "[SjclFile]") {

	SECTION("to json and from json") {
		SjclFile s1, s2;
		s1.setIv("n2JUTJ0/yrLdCDPfIcqAzw==");
		s1.setV(uint32_t(1));
		s1.setIter(uint32_t(10000));
		s1.setKs(uint32_t(256));
		s1.setTs(uint32_t(64));
		s1.setMode("ccm");
		s1.setAdata("");
		s1.setCipher("aes");
		s1.setSalt("ZRVja4LFrFY=");
		s1.setCt("FCuQWGYz3onE/lRt/7vCl5A=");

		nlohmann::json json;
		json << s1;

		json >> s2;

		REQUIRE(s1.getIv() == s2.getIv());
		REQUIRE(s1.getV() == s2.getV());
		REQUIRE(s1.getIter() == s2.getIter());
		REQUIRE(s1.getKs() == s2.getKs());
		REQUIRE(s1.getTs() == s2.getTs());
		REQUIRE(s1.getMode() == s2.getMode());
		REQUIRE(s1.getAdata() == s2.getAdata());
		REQUIRE(s1.getCipher() == s2.getCipher());
		REQUIRE(s1.getSalt() == s2.getSalt());
		REQUIRE(s1.getCt() == s2.getCt());
	}

}
