/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Plugin/Transaction/Payload/CRCouncilMemberClaimNode.h>
#include <Common/Log.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/Key.h>

static uint8_t version = CRCouncilMemberClaimNodeVersion;

using namespace Elastos::ElaWallet;
static void initPayload(CRCouncilMemberClaimNode &payload) {
	std::string mnemonic = Mnemonic(boost::filesystem::path("Data")).Create("English", Mnemonic::WORDS_12);
	uint512 seed = BIP39::DeriveSeed(mnemonic, "");
	HDSeed hdseed(seed.bytes());
	HDKeychain rootkey(hdseed.getExtendedKey(true));
	HDKeychain masterKey = rootkey.getChild("44'/0'/0'");
	HDKeychain nodePublicKey = masterKey.getChild("0/0");
	HDKeychain councilMemberKey = masterKey.getChild("0/1");

	payload.SetNodePublicKey(nodePublicKey.pubkey());
	payload.SetCRCouncilMemberDID(Address(PrefixIDChain, councilMemberKey.pubkey(), true));

	Key key;
	const uint256 &digest = payload.DigestUnsigned(version);
	key = councilMemberKey;
	bytes_t signature = key.Sign(digest);
	payload.SetCRCouncilMemberSignature(signature);
}

TEST_CASE("RechargeToSideChain test", "[RechargeToSideChain]") {
	Log::registerMultiLogger();
	CRCouncilMemberClaimNode p1, p2;
	ByteStream stream;

	initPayload(p1);

	REQUIRE(p1.IsValid(version));
	p1.Serialize(stream, version);
	REQUIRE(p2.Deserialize(stream, version));
	REQUIRE(p1 == p2);

	initPayload(p1);
	REQUIRE(p1.IsValid(version));
	nlohmann::json j = p1.ToJson(version);
	REQUIRE_NOTHROW(p2.FromJson(j, version));
	REQUIRE(p1 == p2);
}