// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadTransferCrossChainAsset.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadTransferCrossChainAsset Test", "[PayloadTransferCrossChainAsset]") {

    SECTION("none init test") {
        PayloadTransferCrossChainAsset ptcca, ptcca_re;

        ByteStream stream;
        ptcca.Serialize(stream);


        stream.setPosition(0);

        ptcca_re.Deserialize(stream);

        ByteData bd_src = ptcca.getData();
        ByteData bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.length == bd_rc.length);

        if (bd_src.data && bd_rc.data)
            REQUIRE(0 == memcmp(bd_src.data, bd_rc.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_rc.data) delete[] bd_rc.data;
    }

    SECTION("init test") {
        PayloadTransferCrossChainAsset ptcca, ptcca_re;

        std::map<std::string, uint64_t> am;
        am["addr"]  = 64;
        am["addr1"] = 128;
        ptcca.setAddressMap(am);

        ByteStream stream;
        ptcca.Serialize(stream);


        stream.setPosition(0);

        ptcca_re.Deserialize(stream);

        ByteData bd_src = ptcca.getData();
        ByteData bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.length == bd_rc.length);

        if (bd_src.data && bd_rc.data)
            REQUIRE(0 == memcmp(bd_src.data, bd_rc.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_rc.data) delete[] bd_rc.data;
    }
}