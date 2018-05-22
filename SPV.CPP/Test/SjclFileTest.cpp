// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>
#include <boost/filesystem.hpp>

#include "KeyStore/SjclFile.h"

using namespace Elastos::SDK;

TEST_CASE("Json convert", "[SjclFile]") {

	SECTION("Convert to json") {
		SjclFile sf, sf_re;
		sf.setIv(std::string("n2JUTJ0/yrLdCDPfIcqAzw=="));
		sf.setV(uint32_t(1));
		sf.setIter(uint32_t(10000));
		sf.setKs(uint32_t(256));
		sf.setTs(uint32_t(64));
		sf.setMode(std::string("ccm"));
		sf.setAdata(std::string(""));
		sf.setCipher(std::string("aes"));
		sf.setSalt(std::string("ZRVja4LFrFY="));
		sf.setCt(std::string("FCuQWGYz3onE/lRt/7vCl5A="));

		nlohmann::json json;
		json << sf;

		json >> sf_re;

		int pause = 0;
	}

	SECTION("Convert from json file") {
		const boost::filesystem::path path("conf.json");
		std::string str_path = path.string();
		if (str_path != "") {
			std::ifstream i(str_path);
			nlohmann::json j;
			i >> j;

			SjclFile sf;
			j >> sf;

			int pause = 0;
		}
	}

}
