// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <WalletCore/AES.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("AES test", "[AES]") {
	Log::registerMultiLogger();
	SECTION("Decrypt and encrypt web keystore") {
		nlohmann::json keystore = nlohmann::json::parse(
			"{\"iv\":\"kha3Vctm00fA1hjOdP0YQA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"J4aFQZSWvFo=\",\"ct\":\"mDVCy7fnV3RnehbDlafq4ALab43ZkVkzm5x2sHVjQscmQt4jWXUQQJP/A/+SYixFvj/fqZ4u+alT5/x08OAcWqnr/ft19ItfMB+bBd7vT1T93DygjRj9nRKMSyPu9rIarCU1fRPbxmNKF7dE7E2njtPcEZ+FAUKorj0P5AGvzWQM8QIlVbxz6TQTu120Ar1ukzlHc+xXTA8pw6ZlJcZQ6i7khI/hnVQzkuXr7R7y09yvU4LIDob/KmzsP7hSMha3hVUFLtvnTl5ElHlll7nywERMNYVt2khNSDfPACsFyaVlPHopKGsdp66IjJBemTVna6dH/n0HEIGFzsQ/dr//vsk9/BUoq7eLMart0StzaNm4R43q/3h0f4+SwqG0hk2aBGCG0QEgSor/S02ts50P80pbuGOQ7zARfmxkkEpm5z4iT0krPyJEsccitDtYBHmUEaSnT1zffkchtJsaZJFFx/9s5UpLkD2JrtYKvAM/txuBmacPFyxzUIdCoAH1n2z1rBvdd2hUPOXqds9IvHIEhx0aXR7fjRivS1JBpjqFHRKOEkGjkb+lN+HGtjooQxdOcmN1lVR5/oHETKqelupSosdNOiw2z2Ytpd5ovzX14PyGqnY8um4d/CzG7q5h8KDsGlrxPPvVEtl+WuAQ6zO/4YxLP+llukuY4b+vyT+v6c40qP5+acnkjb3zJ301EAo/PMIMf8IX2NssPVcd+XuRIospbvKMpTh05VjoXFr6rNu+/zho527C8JDe4tG4Fjs1yWOwQVM1Qk2ear2Rnwbf/lnYmU2TcO7zd69iRkiY7rhuLUTerbCjqVgZUTixri0HvsU0g8y5YZ5Thor7ghr/vV/YOywlgTAxknnr6/mtrVE1YXRP071lDsQ0734FK1L6oUfzoMBVlecUV8DUMQO6U4A34Y9THqHbhMBem05US7D0AQ8HtRuhg7c28LUOugxd6LTmbo4RNnre0bMCGNI6ZGhtBHXn1lMWWLe1bFy6tj9Uo6oVE/vg7pykEA/zJ3BhE4TJv8J3HIsAUEgVe5f7StDieODk8Y4uYAp2XLTi8QhiDt4dvaLHVkLz1Vjn3GQ15882bI6/H3/ezUTfxgTrdfGm8T7hFrYQx7QibYQRYgs+9UWWbHVKbqA5T0DA87yUFIcVgVv+CrubxrIr61vF7nu3EpLKCstqcKTM3LKjPxJlCKqCJAPDSgvpwjCWmI6fftZQ1SlCgIWAT3yGOy+FBaHr25+ZhT2iKh6UTUD3Yxr7mzo1/wzQGgpMqCLSKCmJUuHDzjdzjAu13RL9i+YA4Pk0FjkGhRLjhg/XT5trsLA2Hl8ZvUtZK0BXPPUYCXlFjLB6By1CkH4CPQJU69yV+MWO7NsJ59fQy5wl+5NS4Ks7XyNyAPacK3BkHuFlulGKCJyeQ9mLy9qKaXsdgUwThvVfgnRSl5qFoUvdZIHCgPESVRudP3lptlrTX1+ryuSLztNj8DE4zXUJRAgmua/QwVXrlBN6Y+I6tctjrSXONWtNVCp5HYmeaGbDo1w0oUnOQkpgP/4RjEfvDa5InRjJ6NxhuhOXfqWiYUNiHL9AXJPlulSnda/nOylOneJqxCFP4/dFF/0fXBon+pVf4HMc0M7KrRxwGq2fvk2OhIU/lo6ouZQCIfQuV0JM/PHEm/PK0YMbyQx+FvGiZzBtTiVMQyKUOxAgNDJMyJCWvhBm/+ktzIQgtpvBq9NI1pkF+GFd2AgIoArhA4DidpLBM477HXAntj+PGeSc4FvS6gk=\"}");
		std::string plaintext = "{\"coin\":\"ela\",\"network\":\"livenet\",\"xPrivKey\":\"xprv9s21ZrQH143K2C3tdyYrBmtXj6z83YHanKhuMm3PCXoGidNEkfKQYMBtG1Z4GA65VGJMQvBw4y1SpqdeTT25pgNKBSARK5DRp1goQAXrrNp\",\"xPubKey\":\"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z\",\"requestPrivKey\":\"763e69ed2899f4302ed2655687f55e5a453dfe52c6bb2d13aea7002ff0cf3c3f\",\"requestPubKey\":\"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280\",\"copayerId\":\"03c3e253ca50aa7437560ef4fe9e71c2613297ab37331074028f7af5a6a789fc\",\"publicKeyRing\":[{\"xPubKey\":\"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z\",\"requestPubKey\":\"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280\"}],\"walletId\":\"79d91cc2-0002-4091-b752-d517c71724b5\",\"walletName\":\"个人钱包\",\"m\":1,\"n\":1,\"walletPrivKey\":\"77fd3b22162b6abf9896cc319ecb01e78946e0e431cf681e0be07386b3d089e1\",\"personalEncryptingKey\":\"9e8NKebjCQheiNkc4WWTMA==\",\"sharedEncryptingKey\":\"BIEMo98/cEXf+pY1MnmDHQ==\",\"copayerName\":\"我\",\"mnemonic\":\"闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉\",\"entropySource\":\"5480ac5511c2daaa99641ce3547d6a36439b5911bb769d6f79cbec927d81d771\",\"mnemonicHasPassphrase\":false,\"derivationStrategy\":\"BIP44\",\"account\":0,\"compliantDerivation\":true,\"addressType\":\"P2PKH\"}";
		std::string passwd = "heropoon";
		std::string wrongPasswd = "1234566";

		std::string iv = keystore["iv"];
		std::string ct = keystore["ct"];
		std::string salt = keystore["salt"];
		std::string mode = keystore["mode"];
		std::string aad = keystore["adata"];
		int ks = keystore["ks"];

		bytes_t plain;
		REQUIRE_THROWS(plain = AES::DecryptCCM(ct, wrongPasswd, salt, iv, aad, ks));
		REQUIRE_NOTHROW(plain = AES::DecryptCCM(ct, passwd, salt, iv, aad, ks));
		REQUIRE(plain == bytes_t(plaintext.data(), plaintext.size()));

		std::string randSalt = AES::RandomSalt().getBase64();
		std::string randIV = AES::RandomIV().getBase64();
		std::string cipher;
		REQUIRE_NOTHROW(
			cipher = AES::EncryptCCM(bytes_t(plaintext.data(), plaintext.size()), passwd, randSalt, randIV, aad, 256));
		REQUIRE_THROWS(plain = AES::DecryptCCM(cipher, wrongPasswd, randSalt, randIV, aad, 256));
		REQUIRE_THROWS(plain = AES::DecryptCCM(cipher, passwd, randSalt, randIV, aad, 128));
		REQUIRE_THROWS(plain = AES::DecryptCCM(cipher, passwd, randSalt, iv, aad, 256));
		REQUIRE_THROWS(plain = AES::DecryptCCM(cipher, passwd, salt, iv, aad, 256));
		REQUIRE_NOTHROW(plain = AES::DecryptCCM(cipher, passwd, randSalt, randIV, aad, 256));

		REQUIRE(plain == bytes_t(plaintext.data(), plaintext.size()));

		REQUIRE(AES::EncryptCCM(bytes_t(), passwd).empty());
		REQUIRE(AES::DecryptCCM(std::string(), passwd).empty());
	}

	SECTION("Decrypt and encrypt spvsdk keystore") {
		nlohmann::json encryptedKeystore = nlohmann::json::parse("{\"adata\":\"\",\"cipher\":\"aes\",\"ct\":\"XNfrjrFTYyzDZnZxjdjUNGfrJGrOxQ9hBU3j1+jRDf3knFTC+pDSrg0st+IHrxfvYEn5SBSUSbh6+PtHl3HKfeiBbJyuqjVNLZmy0ZlOTFI02yYlqVXC5HyOFM5uY4jGaGpY/gr+XPtJuaz39TpxwuUjckdNP1rvbFLuABvw6HJOTYMfKp+a+n5UzzITHMuRPoOJ1ZGjvws4f82Vft/Qdtp5/90TDDxeJFJlbZQTLleM/ckycZoUzCkuDxXYhBMcqu54h5HnQvupb9nDqWCUlxbr3olsok+YXuL+BA3b0qa/07b3u0KTlyGeYByg1PSMhaYjB6K6aalxoSyVVcS1lkbuuT3Z2AD2p9fuQtQIcG3/hw0fwzG3dXs6oMkkUPNPIQURniKrD5G84nVVvAfAUF8noB0kF1zQWRKZHUwxP21Qzgge1Fr883SOD9Z2lyVffiC7j2aeHOp3wKRoVn/0L115b+vtWv1L2mdMBqaqS/hMh+sCBJL3FnbAN0BqimN7s1o1idgHyfcq327JXHQl3vn+/yqYuAGdVPW7pGLQwa2nP41IcoqpbtlqVpa3iW/jsSgMueVAmTdjQmRjitqk83C/t62FQ8F2HwKV2iUQiutmLKPzYEDSJSvh3TL/gmIgGIVuYIUAlLJlpAicUBhNWiVkegj8y08UdgtpxogZaD9pbBgMQ2zYQufbbc32ZRIRv+mCKKU9ZrP9m34eR6Wzl7M5hHrevKKzpgODhNB5qSPYFC9nV6HDNUgYOdC9nd2TTTV5j3m9aLhV4wG3EPtK5gV3a5UUwjGgIITYYLrCogjijWRY/sCnVVRz2sUi2baZRQU7qIhHwdZAAyEC94zV3UPyQjK+ug2X2Ps8z5NbixcadtkyOHpzjurinDD6yW5vFmdU5U9QizoR92fw+HNnNyr5pz4Oe9QkWqZCi0ea9dMFTe0Z2EAoFBUujjvzhy6YANfyXNSUxHX3SNo2IV1MGo4YxBAaMpaK0JCcMoQWMVTVfXwcAJdhlY4eDCNJn/XnkKfe5TaIu1C4aQ7KQDezshCf4bmZ4GxA/GgSaZw/pslEyZr/gY7Xmwmwb0qZyAtLDyj9vNwsG9CYOugYBaDRqz43InFReQQvZJuAAgXpL0SOxXwEfLFHtTSpFxQEDITvCXQ8Dm++YhqOG+wFvMQNxvNcWJ+pDY2f7t8w81Y9VOee2OcRlMJsikKYCsEx0Yp8BFYhMet9IlKeFfKWbYkrVfyI/8EJc5NoxFUfLp359qtYqmJ6QZrSPUoQ1lGirYbFSBcg+5jlLDaGWG0Em/qgkC4RdO2n+I7W5lfoImTSG/x2aUfCloZEI8W7hV430+sUFbi4yjLzbpXNBDGNU9tWzRrssuQKP/pchHayhM3slrFjzWNqKDbL8JkKz7IbddJvYpPAaE88GwjZp3EZtxlCoEBZ+mdbzGSEMZdIEFqf3Ftu9invyhZYPcXWaq+f2+CfBnyTQ+0ze97NdP7URdXE7F9un1Fh+eovfCEJaixBSxgwDxbMjD8ec48xrh8A43JGt+Mq80jn/ostXTjiXN6w//3wgu/FwsENlCXsYEsi31Q6V2U2uHSnRL0vTWpjgyydbRhAErqaOIV78WdOOAruLZCs0L6e/xprcraXQkWPO+zCu0xJjKPxJPmLKZV0kb+JN41JnGKtFhwxq5sr1DlbAmeM/3HntLdF5mjiMR4ow/jIhewUvgl2jk57L6FmFdmIxTR4sNUPH8BHH7PDs4VBqMrBI7EQJJ1WOyY4MqfsenHFsa3rqg4mfUWzk0W3cdgcH/3Hz3hEtbXwMFw6ktwM+dUhvwRLhG6aZ3I4aTTsElej1VFArWh52qIDLxDyTBS/KdNL67DKqe6NGrDlamzLTpgx53ZAGygp3abeu7ZEdlTASTLLhsqH8wj3MUzNLCXJ1zVjeSmiAdCAQOs6xkdCaCp/u4z1U7KyUQhqQEwnaZjHE+37jtvC0CW9\",\"iter\":10000,\"iv\":\"S+Woj2wU047BHQ//5AVEdg==\",\"ks\":256,\"mode\":\"ccm\",\"salt\":\"bWOqnhKCAVs=\",\"ts\":64,\"v\":1}");
		std::string plaintext = "{\"CoSigners\":[],\"CoinInfoList\":[{\"ChainCode\":\"cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce\",\"ChainID\":\"ELA\",\"EarliestPeerTime\":1547695777,\"EnableP2P\":true,\"FeePerKB\":10000,\"Index\":0,\"MinFee\":10000,\"PublicKey\":\"0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f\",\"ReconnectSeconds\":300,\"SingleAddress\":false,\"UsedMaxAddressIndex\":0,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"],\"WalletType\":1},{\"ChainCode\":\"cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce\",\"ChainID\":\"IdChain\",\"EarliestPeerTime\":1550222329,\"EnableP2P\":true,\"FeePerKB\":10000,\"Index\":0,\"MinFee\":10000,\"PublicKey\":\"0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f\",\"ReconnectSeconds\":300,\"SingleAddress\":false,\"UsedMaxAddressIndex\":0,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"],\"WalletType\":3}],\"IsSingleAddress\":false,\"Language\":\"chinese\",\"PhrasePassword\":\"\",\"PrivateKey\":\"\",\"RequiredSignCount\":0,\"Type\":\"Standard\",\"account\":0,\"addressType\":\"\",\"coin\":\"\",\"compliantDerivation\":false,\"copayerId\":\"\",\"copayerName\":\"\",\"derivationStrategy\":\"\",\"entropySource\":\"\",\"m\":0,\"mnemonic\":\"闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉\",\"mnemonicHasPassphrase\":false,\"n\":0,\"network\":\"\",\"personalEncryptingKey\":\"\",\"publicKeyRing\":[],\"requestPrivKey\":\"\",\"requestPubKey\":\"\",\"sharedEncryptingKey\":\"\",\"walletId\":\"\",\"walletName\":\"\",\"walletPrivKey\":\"\",\"xPrivKey\":\"\",\"xPubKey\":\"\"}";
		std::string passwd = "qqqqqqqq";

		std::string iv   = encryptedKeystore["iv"];
		std::string ct   = encryptedKeystore["ct"];
		std::string salt = encryptedKeystore["salt"];
		std::string mode = encryptedKeystore["mode"];
		std::string aad  = encryptedKeystore["adata"];
		int ks = encryptedKeystore["ks"];

		bytes_t plain;
		REQUIRE_NOTHROW(plain = AES::DecryptCCM(ct, passwd, salt, iv, aad, ks));

		REQUIRE(plaintext == std::string((char *)plain.data(), plain.size()));
	}
}