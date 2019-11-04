// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <nlohmann/json.hpp>
#include <WalletCore/SjclFile.h>
#include <Common/Log.h>

#include <fstream>
#include <boost/filesystem.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Json convert", "[SjclFile]") {
	Log::registerMultiLogger();

	SECTION("to json and from json") {
		SjclFile s1, s2;
		s1.SetIv("n2JUTJ0/yrLdCDPfIcqAzw==");
		s1.SetV(uint32_t(1));
		s1.SetIter(uint32_t(10000));
		s1.SetKs(uint32_t(256));
		s1.SetTs(uint32_t(64));
		s1.SetMode("ccm");
		s1.SetAdata("");
		s1.SetCipher("aes");
		s1.SetSalt("ZRVja4LFrFY=");
		s1.SetCt("FCuQWGYz3onE/lRt/7vCl5A=");

		nlohmann::json json = s1.ToJson();

		s2.FromJson(json);

		REQUIRE(s1.GetIv() == s2.GetIv());
		REQUIRE(s1.GetV() == s2.GetV());
		REQUIRE(s1.GetIter() == s2.GetIter());
		REQUIRE(s1.GetKs() == s2.GetKs());
		REQUIRE(s1.GetTs() == s2.GetTs());
		REQUIRE(s1.GetMode() == s2.GetMode());
		REQUIRE(s1.GetAdata() == s2.GetAdata());
		REQUIRE(s1.GetCipher() == s2.GetCipher());
		REQUIRE(s1.GetSalt() == s2.GetSalt());
		REQUIRE(s1.GetCt() == s2.GetCt());
	}

}
