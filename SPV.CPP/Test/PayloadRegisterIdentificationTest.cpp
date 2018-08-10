// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRInt.h"
#include "Payload/PayloadRegisterIdentification.h"

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadRegisterIdentification fromJson test", "[fromJson]") {

    nlohmann::json rawJson = "{\n"
                             "  \"Id\": \"ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6\",\n"
                             "  \"Contents\": [{\n"
                             "    \"Path\": \"kyc/person/identityCard\",\n"
                             "    \"Proof\": \"{\\\"signature\\\":\\\"30450220499a5de3f84e7e919c26b6a8543fd24129634c65ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6\\\",\\\"notary\\\":\\\"COOIX\\\"}\",\n"
                             "    \"DataHash\": \"bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd793fc3a9413c0\"\n"
                             "  }, {\n"
                             "    \"Path\": \"kyc/person/phone\",\n"
                             "    \"Proof\": \"{\\\"signature\\\":\\\"3046022100e888040388d0f569183eea8e6f608e00f38a0ed38e1d2f227a8638ef3efd8f6c022100ca1c2ec20a9a933c072f6e4e6b9660a5aa1bcace8af384abaa30941010d4a9cf\\\",\\\"notary\\\":\\\"COOIX\\\"}\",\n"
                             "    \"DataHash\": \"cf985a76d1eef80aa7aa4ce144edad2c042c217ecabb0f86cd91bd4dae3b7215\"\n"
                             "  }]\n"
                             "}"_json;

    PayloadRegisterIdentification payload;
    REQUIRE_NOTHROW(payload.fromJson(rawJson));
}