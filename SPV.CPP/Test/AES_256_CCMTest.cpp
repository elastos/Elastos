// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <SDK/Common/Log.h>

#include "AES_256_CCM.h"

using namespace Elastos::ElaWallet;

static unsigned char iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF, 0x21,
							 0xCA, 0x80,
							 0xCF};
static unsigned char salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};

TEST_CASE("Encrypt and decrypt", "[AES_256_CCM]") {

	SECTION("decrypt and encrypt") {
		nlohmann::json keystore = nlohmann::json::parse("{\"iv\":\"kha3Vctm00fA1hjOdP0YQA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"J4aFQZSWvFo=\",\"ct\":\"mDVCy7fnV3RnehbDlafq4ALab43ZkVkzm5x2sHVjQscmQt4jWXUQQJP/A/+SYixFvj/fqZ4u+alT5/x08OAcWqnr/ft19ItfMB+bBd7vT1T93DygjRj9nRKMSyPu9rIarCU1fRPbxmNKF7dE7E2njtPcEZ+FAUKorj0P5AGvzWQM8QIlVbxz6TQTu120Ar1ukzlHc+xXTA8pw6ZlJcZQ6i7khI/hnVQzkuXr7R7y09yvU4LIDob/KmzsP7hSMha3hVUFLtvnTl5ElHlll7nywERMNYVt2khNSDfPACsFyaVlPHopKGsdp66IjJBemTVna6dH/n0HEIGFzsQ/dr//vsk9/BUoq7eLMart0StzaNm4R43q/3h0f4+SwqG0hk2aBGCG0QEgSor/S02ts50P80pbuGOQ7zARfmxkkEpm5z4iT0krPyJEsccitDtYBHmUEaSnT1zffkchtJsaZJFFx/9s5UpLkD2JrtYKvAM/txuBmacPFyxzUIdCoAH1n2z1rBvdd2hUPOXqds9IvHIEhx0aXR7fjRivS1JBpjqFHRKOEkGjkb+lN+HGtjooQxdOcmN1lVR5/oHETKqelupSosdNOiw2z2Ytpd5ovzX14PyGqnY8um4d/CzG7q5h8KDsGlrxPPvVEtl+WuAQ6zO/4YxLP+llukuY4b+vyT+v6c40qP5+acnkjb3zJ301EAo/PMIMf8IX2NssPVcd+XuRIospbvKMpTh05VjoXFr6rNu+/zho527C8JDe4tG4Fjs1yWOwQVM1Qk2ear2Rnwbf/lnYmU2TcO7zd69iRkiY7rhuLUTerbCjqVgZUTixri0HvsU0g8y5YZ5Thor7ghr/vV/YOywlgTAxknnr6/mtrVE1YXRP071lDsQ0734FK1L6oUfzoMBVlecUV8DUMQO6U4A34Y9THqHbhMBem05US7D0AQ8HtRuhg7c28LUOugxd6LTmbo4RNnre0bMCGNI6ZGhtBHXn1lMWWLe1bFy6tj9Uo6oVE/vg7pykEA/zJ3BhE4TJv8J3HIsAUEgVe5f7StDieODk8Y4uYAp2XLTi8QhiDt4dvaLHVkLz1Vjn3GQ15882bI6/H3/ezUTfxgTrdfGm8T7hFrYQx7QibYQRYgs+9UWWbHVKbqA5T0DA87yUFIcVgVv+CrubxrIr61vF7nu3EpLKCstqcKTM3LKjPxJlCKqCJAPDSgvpwjCWmI6fftZQ1SlCgIWAT3yGOy+FBaHr25+ZhT2iKh6UTUD3Yxr7mzo1/wzQGgpMqCLSKCmJUuHDzjdzjAu13RL9i+YA4Pk0FjkGhRLjhg/XT5trsLA2Hl8ZvUtZK0BXPPUYCXlFjLB6By1CkH4CPQJU69yV+MWO7NsJ59fQy5wl+5NS4Ks7XyNyAPacK3BkHuFlulGKCJyeQ9mLy9qKaXsdgUwThvVfgnRSl5qFoUvdZIHCgPESVRudP3lptlrTX1+ryuSLztNj8DE4zXUJRAgmua/QwVXrlBN6Y+I6tctjrSXONWtNVCp5HYmeaGbDo1w0oUnOQkpgP/4RjEfvDa5InRjJ6NxhuhOXfqWiYUNiHL9AXJPlulSnda/nOylOneJqxCFP4/dFF/0fXBon+pVf4HMc0M7KrRxwGq2fvk2OhIU/lo6ouZQCIfQuV0JM/PHEm/PK0YMbyQx+FvGiZzBtTiVMQyKUOxAgNDJMyJCWvhBm/+ktzIQgtpvBq9NI1pkF+GFd2AgIoArhA4DidpLBM477HXAntj+PGeSc4FvS6gk=\"}");
		std::string plainResult = "{\"coin\":\"ela\",\"network\":\"livenet\",\"xPrivKey\":\"xprv9s21ZrQH143K2C3tdyYrBmtXj6z83YHanKhuMm3PCXoGidNEkfKQYMBtG1Z4GA65VGJMQvBw4y1SpqdeTT25pgNKBSARK5DRp1goQAXrrNp\",\"xPubKey\":\"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z\",\"requestPrivKey\":\"763e69ed2899f4302ed2655687f55e5a453dfe52c6bb2d13aea7002ff0cf3c3f\",\"requestPubKey\":\"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280\",\"copayerId\":\"03c3e253ca50aa7437560ef4fe9e71c2613297ab37331074028f7af5a6a789fc\",\"publicKeyRing\":[{\"xPubKey\":\"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z\",\"requestPubKey\":\"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280\"}],\"walletId\":\"79d91cc2-0002-4091-b752-d517c71724b5\",\"walletName\":\"个人钱包\",\"m\":1,\"n\":1,\"walletPrivKey\":\"77fd3b22162b6abf9896cc319ecb01e78946e0e431cf681e0be07386b3d089e1\",\"personalEncryptingKey\":\"9e8NKebjCQheiNkc4WWTMA==\",\"sharedEncryptingKey\":\"BIEMo98/cEXf+pY1MnmDHQ==\",\"copayerName\":\"我\",\"mnemonic\":\"闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉\",\"entropySource\":\"5480ac5511c2daaa99641ce3547d6a36439b5911bb769d6f79cbec927d81d771\",\"mnemonicHasPassphrase\":false,\"derivationStrategy\":\"BIP44\",\"account\":0,\"compliantDerivation\":true,\"addressType\":\"P2PKH\"}";

		std::string iv = keystore["iv"];
		int ks = keystore["ks"];
		std::string ct = keystore["ct"];
		std::string salt = keystore["salt"];
		std::string mode = keystore["mode"];
		std::string adata = keystore["adata"];

		std::string plaintext;
		std::string passwd = "heropoon";
		REQUIRE(AES_256_CCM::Decrypt(plaintext, ct, passwd, salt, iv, adata, ks == 128 ? true : false));
		REQUIRE(plaintext == plainResult);

		std::string cipherText;
		REQUIRE(AES_256_CCM::Encrypt(cipherText, plaintext, passwd, salt, iv, adata, ks == 128 ? true : false));
		REQUIRE(cipherText == ct);

		plaintext.clear();
		REQUIRE(AES_256_CCM::Decrypt(plaintext, cipherText, passwd, salt, iv, adata, ks == 128 ? true : false));
		REQUIRE(plaintext == plainResult);

		plaintext.clear();

		std::string wrongPasswd = "12345";
		REQUIRE(!AES_256_CCM::Decrypt(plaintext, ct, wrongPasswd, salt, iv, adata, ks == 128 ? true : false));
		REQUIRE(plaintext != plainResult);

		cipherText.clear();
		REQUIRE(AES_256_CCM::Encrypt(cipherText, "", passwd, salt, iv, adata, ks == 128 ? true : false));
		Log::getLogger()->info("ciphertext -> {}", cipherText);

		plaintext.clear();
		REQUIRE(AES_256_CCM::Decrypt(plaintext, cipherText, passwd, salt, iv, adata, ks == 128 ? true : false));
		REQUIRE(plaintext == "");
	}
}

