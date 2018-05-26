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

        CMBlock bd_src = ptcca.getData();
        CMBlock bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.GetSize() == bd_rc.GetSize());

        if (bd_src && bd_rc)
            REQUIRE(0 == memcmp(bd_src, bd_rc, bd_src.GetSize()));
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

        CMBlock bd_src = ptcca.getData();
        CMBlock bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.GetSize() == bd_rc.GetSize());

        if (bd_src && bd_rc)
            REQUIRE(0 == memcmp(bd_src, bd_rc, bd_src.GetSize()));
    }

    SECTION("toJson fromJson test") {
        PayloadTransferCrossChainAsset ptcca, ptcca_re;

        std::map<std::string, uint64_t> am;
        am["addr"]  = 64;
        am["addr1"] = 128;
        ptcca.setAddressMap(am);

        nlohmann::json jsonData = ptcca.toJson();

        ptcca_re.fromJson(jsonData);

        CMBlock bd_src = ptcca.getData();
        CMBlock bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.GetSize() == bd_rc.GetSize());

        if (bd_src && bd_rc)
            REQUIRE(0 == memcmp(bd_src, bd_rc, bd_src.GetSize()));
    }
}