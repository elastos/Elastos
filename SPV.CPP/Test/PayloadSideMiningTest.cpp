// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadSideMining.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadSideMining Test", "[PayloadSideMining]") {

    SECTION("init test") {
        PayloadSideMining psm, psm_re;
        psm.setSideBlockHash(UINT256_ZERO);
        psm.setSideGenesisHash(UINT256_ZERO);

        ByteStream stream;
        psm.Serialize(stream);
        stream.setPosition(0);

        psm_re.Deserialize(stream);

        CMBlock bd_src = psm.getData();
        CMBlock bd_rc = psm_re.getData();

        size_t mem_len = sizeof(UInt256) * 2;
        REQUIRE(mem_len == bd_src.GetSize());
        if (bd_src && bd_rc)
            REQUIRE(0 == memcmp(bd_src, bd_rc, mem_len));

    }

}