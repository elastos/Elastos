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
        CMBlock bd_src, bd_re;

        pcb.Serialize(stream);
        stream.setPosition(0);
        pcb_re.Deserialize(stream);

        bd_src = pcb.getData();
        bd_re  = pcb_re.getData();

        REQUIRE(bd_src.GetSize()==bd_re.GetSize());
        if (bd_src && bd_re)
            REQUIRE(0==memcmp(bd_src, bd_re, bd_src.GetSize()));
    }

    SECTION("init test") {
        PayloadCoinBase pcb, pcb_re;
        ByteStream stream;
        CMBlock bd_src, bd_re;

        uint8_t buf[] = {'I', ' ', 'a', 'm', ' ', 'O', 'K', '\0'};
        CMBlock bd;
        bd.SetMemFixed(buf, sizeof(buf));
        pcb.setCoinBaseData(bd);
        pcb.Serialize(stream);
        stream.setPosition(0);

        pcb_re.Deserialize(stream);

        bd_src = pcb.getData();
        bd_re  = pcb_re.getData();

        REQUIRE(bd_src.GetSize()==bd_re.GetSize());
        if (bd_src && bd_re)
            REQUIRE(0==memcmp(bd_src, bd_re, bd_src.GetSize()));
    }

    SECTION("toJson fromJson test") {
        uint8_t buf[] = {'I', ' ', 'a', 'm', ' ', 'O', 'K', '\0'};
        CMBlock bd;
        bd.SetMemFixed(buf, sizeof(buf));

        PayloadCoinBase payloadCoinBase(bd);

        nlohmann::json jsonData = payloadCoinBase.toJson();

        PayloadCoinBase payloadCoinBase1;
        payloadCoinBase1.fromJson(jsonData);

        CMBlock bd_src, bd_re;
        bd_src = payloadCoinBase.getData();
        bd_re  = payloadCoinBase1.getData();

        REQUIRE(bd_src.GetSize()==bd_re.GetSize());
        if (bd_src && bd_re)
            REQUIRE(0==memcmp(bd_src, bd_re, bd_src.GetSize()));
    }
}