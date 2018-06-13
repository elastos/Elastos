// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BigIntFormat.h"

using namespace Elastos::SDK;

TEST_CASE("tesing Normal", "[BigIntFormatTest.cpp]") {
	uint8_t block[] = {0xBF, 0xCE, 0x24, 0x45, 0x67, 0xA6};
	CMemBlock<uint8_t> mb_src;
	mb_src.SetMemFixed(block, sizeof(block));

	CMemBlock<char> mb_dest = Hex2Str(mb_src);

	std::string str_dest = "BFCE244567A6";
	REQUIRE(str_dest == (const char *) mb_dest);

	uint8_t block1[] = {165, 226, 33};
	CMemBlock<uint8_t>mb_src1;
	mb_src1.SetMemFixed(block1, sizeof(block1));
	CMemBlock<char> mb_dest1 = Dec2Str(mb_src1);

	std::string str_dest1 = (const char *) mb_dest1;
	REQUIRE(str_dest1 == (const char *) mb_dest1);

	CMemBlock<uint8_t> mb_src1_recov = Str2Dec(mb_dest1);
	REQUIRE(0 == memcmp(mb_src1_recov, block1, sizeof(block1)));
}

TEST_CASE("tesing None Normal", "[BigIntFormatTest.cpp]") {
	CMemBlock<uint8_t> mb_src;

	CMemBlock<char> mb_dest = Hex2Str(mb_src);

	REQUIRE((const void *) mb_dest == (const void *) mb_src);

	CMemBlock<uint8_t> mb_src_recov = Str2Hex(mb_dest);
	REQUIRE((const void *) mb_src_recov == (const void *) mb_src);
}