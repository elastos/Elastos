// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <Common/JsonGenerator.h>

TEST_CASE("JsonGenerator test", "[JsonGenerator]") {
	SECTION("make a json object") {
		JsonGenerator generator, *pGenerator;
		pGenerator = JsonGenerator_Initialize(&generator);

		JsonGenerator_WriteStartObject(pGenerator);

		// subject
		JsonGenerator_WriteFieldName(pGenerator, "id");
		JsonGenerator_WriteString(pGenerator, "hello");

		// publicKey
		JsonGenerator_WriteFieldName(pGenerator, "publicKey");
		JsonGenerator_WriteStartArray(pGenerator);
		JsonGenerator_WriteString(pGenerator, "pk1");
		JsonGenerator_WriteString(pGenerator, "pk2");
		JsonGenerator_WriteString(pGenerator, "pk3");
		JsonGenerator_WriteEndArray(pGenerator);

		JsonGenerator_WriteEndObject(pGenerator);
		const char *presult = JsonGenerator_Finish(pGenerator);
		std::string jsonString = presult;
		free((void *)presult);

		REQUIRE(jsonString == "{\"id\":\"hello\",\"publicKey\":[\"pk1\",\"pk2\",\"pk3\"]}");
	}
}