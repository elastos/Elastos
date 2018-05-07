// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadWithDrawAsset.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadWithDrawAsset Test", "[PayloadWithDrawAsset]") {

    SECTION("none init test") {
        PayloadWithDrawAsset pwda, pwda_re;
        ByteData bd_src, bd_re;
        ByteStream stream;

        pwda.Serialize(stream);
        stream.setPosition(0);
        pwda_re.Deserialize(stream);

        bd_src = pwda.getData();
        bd_re = pwda_re.getData();

        REQUIRE(bd_src.length==bd_re.length);

        if (bd_src.data && bd_re.data)
            REQUIRE(0==memcmp(bd_src.data, bd_re.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_re.data)  delete[] bd_re.data;
    }

    SECTION("init test") {
        PayloadWithDrawAsset pwda, pwda_re;
        ByteData bd_src, bd_re;
        ByteStream stream;

        pwda.setBlockHeight(100);
        pwda.setGenesisBlockAddress("address");
        pwda.setSideChainTransacitonHash("hash");
        pwda.Serialize(stream);
        stream.setPosition(0);
        pwda_re.Deserialize(stream);

        bd_src = pwda.getData();
        bd_re = pwda_re.getData();

        REQUIRE(bd_src.length==bd_re.length);

        if (bd_src.data && bd_re.data)
            REQUIRE(0==memcmp(bd_src.data, bd_re.data, bd_src.length));

        if (bd_src.data) delete[] bd_src.data;
        if (bd_re.data)  delete[] bd_re.data;
    }

}