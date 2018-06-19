// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadTransferCrossChainAsset.h"

using namespace Elastos::ElaWallet;

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

        std::vector<std::string> crossChainAddress;
        std::vector<uint64_t> crossChainIndex;
        std::vector<uint64_t> crossChainAmount;
        for (int i = 0; i < 10; ++i) {
            std::string address = "test Address " + std::to_string(i + 1);
            crossChainAddress.push_back(address);
            crossChainIndex.push_back(i + 1);
            crossChainAmount.push_back(i + 1);
        }
        ptcca.setCrossChainData(crossChainAddress, crossChainIndex, crossChainAmount);

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

        std::vector<std::string> crossChainAddress;
        std::vector<uint64_t> crossChainIndex;
        std::vector<uint64_t> crossChainAmount;
        for (int i = 0; i < 10; ++i) {
            std::string address = "test Address " + std::to_string(i + 1);
            crossChainAddress.push_back(address);
            crossChainIndex.push_back(i + 1);
            crossChainAmount.push_back(i + 1);
        }
        ptcca.setCrossChainData(crossChainAddress, crossChainIndex, crossChainAmount);

        nlohmann::json jsonData = ptcca.toJson();

        ptcca_re.fromJson(jsonData);

        CMBlock bd_src = ptcca.getData();
        CMBlock bd_rc = ptcca_re.getData();

        REQUIRE(bd_src.GetSize() == bd_rc.GetSize());

        if (bd_src && bd_rc)
            REQUIRE(0 == memcmp(bd_src, bd_rc, bd_src.GetSize()));
    }
}