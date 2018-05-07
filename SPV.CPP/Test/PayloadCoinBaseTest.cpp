// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Log.h"
#include "Payload/PayloadCoinBase.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadCoinBase Test", "[PayloadCoinBase]") {

    SECTION("none init test") {
        PayloadCoinBase pcb, pcb_re;
        ByteStream stream;
        ByteData bd_src, bd_re;

        pcb.Serialize(stream);
        stream.setPosition(0);
        pcb_re.Deserialize(stream);

        bd_src = pcb.getData();
        bd_re  = pcb_re.getData();

        REQUIRE(bd_src.length==bd_re.length);
        if (bd_src.data && bd_re.data)
            REQUIRE(0==memcmp(bd_src.data, bd_re.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_re.data)  delete[] bd_re.data;
    }

    SECTION("init test") {
        PayloadCoinBase pcb, pcb_re;
        ByteStream stream;
        ByteData bd_src, bd_re;

        uint8_t buf[] = {'I', ' ', 'a', 'm', ' ', 'O', 'K', '\0'};
        ByteData bd(buf, sizeof(buf));
        pcb.setCoinBaseData(bd);
        pcb.Serialize(stream);
        stream.setPosition(0);
        pcb_re.Deserialize(stream);

        bd_src = pcb.getData();
        bd_re  = pcb_re.getData();

        REQUIRE(bd_src.length==bd_re.length);
        if (bd_src.data && bd_re.data)
            REQUIRE(0==memcmp(bd_src.data, bd_re.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_re.data)  delete[] bd_re.data;
    }
}