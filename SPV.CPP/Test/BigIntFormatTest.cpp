// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BigIntFormat.h"

using namespace Elastos::ElaWallet;

TEST_CASE("tesing Normal", "[BigIntFormatTest.cpp]") {
	uint8_t block[] = {0xBF, 0xCE, 0x24, 0x45, 0x67, 0xA6};
	CMemBlock<uint8_t> mbSrc;
	mbSrc.SetMemFixed(block, sizeof(block));

	CMemBlock<char> mbDest = Hex2Str(mbSrc);

	std::string strDest = "BFCE244567A6";
	REQUIRE(strDest == (const char *) mbDest);

	uint8_t block1[] = {165, 226, 33};
	CMemBlock<uint8_t>mbSrc1;
	mbSrc1.SetMemFixed(block1, sizeof(block1));
	CMemBlock<char> mbDest1 = Dec2Str(mbSrc1);

	std::string strDest1 = (const char *) mbDest1;
	REQUIRE(strDest1 == (const char *) mbDest1);

	CMemBlock<uint8_t> mbSrc1recov = Str2Dec(mbDest1);
	REQUIRE(0 == memcmp(mbSrc1recov, block1, sizeof(block1)));
}

TEST_CASE("tesing None Normal", "[BigIntFormatTest.cpp]") {
	CMemBlock<uint8_t> mbSrc;

	CMemBlock<char> mbDest = Hex2Str(mbSrc);

	REQUIRE((const void *) mbDest == (const void *) mbSrc);

	CMemBlock<uint8_t> mbSrcrecov = Str2Hex(mbDest);
	REQUIRE((const void *) mbSrcrecov == (const void *) mbSrc);
}