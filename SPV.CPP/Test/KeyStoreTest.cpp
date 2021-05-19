// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <Account/Account.h>
#include <Common/Log.h>
#include <WalletCore/KeyStore.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/Base58.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/CoinInfo.h>

#include <fstream>

using namespace Elastos::ElaWallet;

TEST_CASE("KeyStore test", "[KeyStore]") {
	Log::registerMultiLogger();

	SECTION("keystore from web wallet") {
		SECTION("with private key and without passphrase") {
			std::string passwd = "s12345678";
			const std::string mnemonic = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
			const std::string passphrase = "";
			nlohmann::json keystoreJson = nlohmann::json::parse("{\"iv\":\"neksof2hUCccAvdcMeYOqg==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"Sv8OvJi34Rs=\",\"ct\":\"qGWsLoHyWuZ/2ARas9G+qXfuIOj67oULVlIJOyK/n1HrhlVVUYukeh9Q3TWMdsKFFQdmLIrhd0dI9dic/VU/YulJrCQK7kvTeYTp9/Tp3INbU6XPygrt48addEYTQNmkcplHasuSGupYnXFGgfhda6F5ABRy41xHf4vbqDh9+7b+k/QyIyTqglVpIKJEiJ+J5/8lkVuIILm2OWNYdh/dMO+/s4gapLvGncBN8uZrpY2GlGDIVMhcH0B+hKsorDSSBssMn3E7JOXTlym2T+wC0Qy1FiUTmsCk7TQu4NpUa3OsPx1Lcp4Ow7AjkI7xmbR48mEBZbLD1GtkRwQiS5j4cp25iGHeeEM3jsdTnOSVa4sZpsnzq2jxEgHVeLmPwZdWwWfKrQo47gH2c95F1hPI7J4eGGLeN9l2YF0JBS9Zl6zQO/jge+PUZolibpKAU9OoGzZF+nCjoLZ9IJJrPIXxVycDquHd07WSGUoQgK+e+BybH5zlbIb3dtMjJFFXI6oXj1+G0ZgN5MKXjnyhcMHf1ojefK3A4BejoQ3dPav34PVJyWPUW3XdZLtfb/7KUk2jgx06kZgcIMFG2QIvK6uexH3KvIcPteVSSDVQ4oSj+HAKDX5qvfku7Yqb7FE04NjB4bRJIhvTngXbW6kuz2iowkc+V1Q2ssfV/js7/IIde1CGfy/fk0qbUNt5QYDgtmX4YmphcCdby67Uaba+YklqinHOJWfS+/pAJ2+FJ/WSS6bUvZljEATsbZdY25xexx1PZ0h5mhIC8tZHOQntcU37AH+DFj/fePqm+SNnXdiKC4ZX2Al8l6hU2yOYCswtWMd1uF2ZVZbZ1xoRkx9A/NOyQt3+/i/qrsa2or4hrg3Dob/sef8zPNlUoMBgvgQ/W0xczugmI0E+Ojh9k4ibzTQMzwicvgYYD2oLnFAlYIUUcRpgE1MQ5ETxeRJsg5tyEz7tEenaOBVbM/xkUDvcbzwKpa+7irBF+fKtyHZseJzTJxfNAfKdyy8smuFqwILrEBWJ4Ci0FYKG2CjlHPqsNK5U0v2YnyYs1nmuxnRhl7v/5QJcgtFZIDbGDuzILCde5NwAv2rsyb7FxFW/1Mic+YbZg3lMmSpa6yOQ89/5v+K3XyFzoad8LFxrjzxsChj1EFci6wc8MNoUlTbMYeO+jhbIRqYje6U4kUKMuglq1CAZEb0KQAK+FHF37Ap8Gd3RR2VE5bQ8K6G4+Tx71c+RLZYqPkDn8STCiGcrXLkrNZfSo1iLYqA0vIfpyZHmkTAjxdWMgBBBAKu503qwSamZYwN23SsAvg/qxj++NMsHKRek98j7jwrZWmNZDfe6W+RAEcrG2LWKcHVTyKWLKm5rE+bi1cKB5W9ZTPCLGkN/jw8tDa25BYd2rMA5mqeGShBqSbG/acw05rQFAgVKvfCZRyMjieGMoZ9Pk68xxABn5ZNFjaogFOylt3VOtL/Dz92wmin+6Q/1oRX8qCkavdVmJu230L2Z7JpVAeoRc2usm+bGRdbvdVC9j3NFW6Mu8+8AcN5ZPGm8yzG7yP2MaGPSGO/PiXc/CUtZNUGQW27BDaClj4EtKIRqp76lDam4e88pLgp67O5naUdcdB4ISmlkleDH4OMdtqSoSj/YDHZvg7rta9SSEDloNB0Y7X+JCaI7czLPZp4CfKHjzfmqflrxZ7YYZLhgkKHdU8xHhS4xg5punUGQHxYlS2xGAAdBSBJ8WaeTtdNXVXYa2b02zJNMSRaWyvnHh5IZezg2P5f4g8MTvInbDmO66vMOe12A/hpf/Zyolw==\"}");

			KeyStore ks;
			REQUIRE_NOTHROW(ks.Import(keystoreJson, passwd));

			const ElaNewWalletJson &walletData = ks.WalletJson();
			REQUIRE(!walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);
			REQUIRE(walletData.GetM() == 1);
			REQUIRE(walletData.GetN() == 1);

			uint512 seed = BIP39::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			HDKeychain firstKey = rootprv.getChild("44'/0'/0'/0/0");
			Address addr(PrefixStandard, firstKey.pubkey());

			bytes_t xpubkey;
			Base58::CheckDecode(walletData.xPubKey(), xpubkey);
			HDKeychain mpk(xpubkey);
			Address addr1(PrefixStandard, mpk.getChild("0/0").pubkey());
			REQUIRE(addr1.String() == addr.String());

			std::string ownerPubKey = rootprv.getChild("44'/0'/1'/0/0").pubkey().getHex();
			REQUIRE(walletData.OwnerPubKey() == ownerPubKey);
			REQUIRE(walletData.GetCoinInfoList().size() == 0);
			REQUIRE(!walletData.SingleAddress());
		}

		SECTION("with private key and with passphrase") {
			const std::string passwd = "s12345678";
			const std::string mnemonic = "锭 援 迁 姻 角 孤 钙 肠 遵 腾 盗 困";
			const std::string passphrase = "heroopan";
			nlohmann::json keystoreJson = nlohmann::json::parse("{\"iv\":\"kgn/MQV6TYL0HPMqm51FqA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"4+aks85XBtI=\",\"ct\":\"iDuHXNSZMOjeTm6nZDYKPYrTObuILSmRSL9niqwHjfhx7fFXALVm2feOG04qj2HbfFMYTdrfPRmqmMUYbbx9O8Ti+5XRgI8uRTAbrbz0mvpalDcZrzVy6QWsVGGACpt3nllQhYlZ5vTyaKM0hhrYk/+xAEDc7V7SVStnVu3DJxF07VWYh4sQxv8g7y3UM2Jw1jlnLZCp2Y1guUUsih6rRnzGCNbM7bvtxDVyEgBrVDL/8sWGhIy/P0va3d7N+4LDTO2smQ5Rx+AumRTwa1oAxcy4H+kACP5TzHtJqAc2hB2w+fQ70Jf+XKET5CGaNTWCzi+hjosgEl6blD1bWf0oFMu1sl/27OGY2kgssvkxELuQHQcQU5aiy3jFzkaVLh/sM4I3rTeqxamQNTU/440Pv6SmRxOg6O6udlfanft8QgRfSFkzBi1bTgE2XOFh00qgumPks3Lz5lSLXSGpPxiN647XiWgUxCG60BSTd/c2KAx/0VdF3Q99v9tR3Z7lT2nLhoOj9VdXveVTspNFCxWbe4HoERvT3x4Fv0P+ie9QiqeJn3CEGgi6Vv34ZL0yOyisS5v8M73tsmY2QWBCw+eZ/M1RFEVitF+wnB/6c7LaEfNJjJXgi485qp9epF+rKpV6OyWacVbSImHRGY+TtLcn7O/RP1Uy4sVevQ2q5xin7NkAs8HjtpBP7kfbcKEZNwYvRTgeaqIzuA2o13mUlYceLN9NZdKuHTKYrwZ1nCROMBhifleGVhc55VRLcUgbhCNUDUPsjJRlFW2F4flY/GSGuOrLRGJvdt/ga4S68qiF8Ep82aC9x8ZXVkAVfoXmrX3VRTw7aVT8f8qD6nMONpsXxSgOGyx6U6wFrZcJdO+Zzw3e4J9axEx8+h+qb7cfsdjE+y6fKwA1Ke8SMmwiHXYDbDsX+vvZvFU4S97AweqlP4BQvw1hhAbYTbi75mEGMgDTE6oaQVRbbyy90jOBlNvHRVcR/BbBjlXs6VUiWlamy6FuQcONFGFxZ2cBVd1jJ+V9CiQcmqRRM3NC8H/icFzHhKOP8yy+FKeFbwSLq1p3sUuTEaQnynYNkCeR768zZMGxtc2ffqKtETsjjZuyNseKtUG7xRmv+X12x0E5NktYnPeBXMf/SHIMAed3RtjKxQuEjMcXmgfAFH70E1W1U9Q2T0sG4Qi6eEKFkJFZsh69KxMy8UpWr+yP+aqYCU+46oPz3OysNtCM27cQC+RylsuqhUkpkoVS1HZwPvIl6v4pnf7maCH1VfcrwCHa3KRymPLFaztUIF4OM0Z6WpSLBg2n1x/2YLpp9aBYrCubwvhhAREu10ZU6rRrdBHpQj8760//Z0UCVy+/uuUAClxG2oDC682bhSKv3KhfJMNGmcI5YnRTUdRJ6k3kGLvSNYQTnuhn1s8TSfWjqSneMJtxaQCOs9mNTtZH3JMwVdceVeCrxWL6WRhRaK6GrNsKKw3mgPmE9s7FWKOOgMY6SJW2GnkfY7ucdlxaGDOCubWM2hx/+y3EBHF/oYdfflAAAZ3ery35oQmRwXrLrvye7JwCQCFQph7mdouoZ3lxvhbkF87dh2Xf1jt2Ros9shAAR+/RoSJMJRLz3jggorfbWmiHjVv2OAxGRIRDCGo4dRdE0se0AFX9Aw2v9ga3wQR/jCMIQGUh3HUKgREosPaPWkDBXsKgQUf+uRyUgasgWVUllYnIpiPBjV9qj2h2lsztImupNAKm/ha3p9rzDNtKUZjdhyrTUtNpcbN/+VZfhBgrr5oYzl5m5JFB7JAzrgaVnA==\"}");

			KeyStore ks;
			REQUIRE_NOTHROW(ks.Import(keystoreJson, passwd));
			const ElaNewWalletJson &walletData = ks.WalletJson();
			REQUIRE(walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);
			REQUIRE(walletData.GetM() == 1);
			REQUIRE(walletData.GetN() == 1);

			uint512 seed = BIP39::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			HDKeychain firstKey = rootprv.getChild("44'/0'/0'/0/0");
			Address addr(PrefixStandard, firstKey.pubkey());

			bytes_t xpubkey;
			Base58::CheckDecode(walletData.xPubKey(), xpubkey);
			HDKeychain mpk(xpubkey);
			Address addr1(PrefixStandard, mpk.getChild("0/0").pubkey());
			REQUIRE(addr1.String() == addr.String());

			std::string ownerPubKey = rootprv.getChild("44'/0'/1'/0/0").pubkey().getHex();
			REQUIRE(walletData.OwnerPubKey() == ownerPubKey);

			REQUIRE(walletData.GetCoinInfoList().size() == 0);
			REQUIRE(!walletData.SingleAddress());
		}

		SECTION("without private key") {
			const std::string passwd = "s12345678";
			nlohmann::json keystoreJson = nlohmann::json::parse("{\"iv\":\"1n/Rqe07AqCzE4l9I2bQ8g==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"4+aks85XBtI=\",\"ct\":\"LtkQ9J95DJagndh0OaGujBsoS8NQ+oNEVy9YMfjo+Bpzi4Uy/iQhAkr3AyNmXA+3SpqMKRxAYusStehLPPgk35Z3Z6gBvbXqRKVzi1uOZ1hC3/SIN8sYx0Q14VEdr7eK6ah20KYEqgNIOZmLp9la0KkGXwgYDA3xFpXOFLnLpQpBYZJc7oHEfRApCtz8Bf8/k4S0oMZzMVcz0tpkhz40dURtOBWIGM9pgmVu/IlpsLeSeH7KjjkCp5R3APOAqB0OFG2OUE1zLQZR2oAE1mjEKy+Ynh2cPgR7kNSQwduY/PRK8pEoGiSp1g2VGGUbKoN/iokK/Vxj+rabXhAek7YLoyvbEON95hatEFVMVmPYbl4m+BApsLKXX82DeRLoa7gMRpYNXNJSLNS7cpgB79kYqhT8LqsQ2ewzhBXlBkMxdOjpKqII3D+Ll9FrDHp2AMQ3SpcS8BcRo39gKL4zFOR+EmVOnRA7d2H9RfBLtt+K7OAb4twtZcy6Kc/KE4kcNCzNFrvQ0KGbYhk1vU1j8Xv5zZIwFXnrG32TKecqezwBp3scbQMyBi7Q5Khe2Fj2x/593WHy9H1MgugvtkxBuqP9qwUyEY9+KaXrIHPDyEY01lRtMiqurOtQ7h6mT9q2vFwmFoKeFniMl1sv7wCDV7WxGkE6v8odreGV0IK/8bkTdTtvxYmOZ//YP+MCtLkrXgm9uAKCrGY36nL1HUG/sWJCpb4H19/atEgy7bWFJzeDpnIDR3noek553FUwciczMBWXmuDHTukrDOMKx8MHp74Tlz9y2I17E8enDXTDkp5efoa2JmG3Mn+Dqo34F6g6G8KFKbFz7AtgdBZZlz/87L1jQRQcrvtejn6a3l9Sge7uKgP/XKxKTv7C7bcL+/sPLpyO1uA/lBXk3q2uArw4RI8yqgLIW9+m6lJtUhOm+tvlh6gKT/jGJNuhJ+XcpRvR+2G8npFOcKz6An3e4AQ3p+e1A25ydiyN083atabe/J+V2ChFq3u15urFgE6tcDUhzRAOFK9YHo1VT1HGkm/BvSCXemiszfueJO7VGoNxx29GogmTQ70pZ4tEH7c7KbHkcdqnejKUssRnQjdUd4fS37fmTp8jP5RgxEXygBBs/UVVKKzVgywtIU9VPnPomzeIA6jeQoG26a/Wqup/eXQWdgrS/qZ2+Ux3NnxnySzW3hJpXUVS9letr6WrBzeDpDVLNaamGEo9WD5iKHRiQLkJjqnDtlYzZTSMtDV06ehwLqkRLwTpqj68ARhvQvhRqZWMfyN6veVLiE3pnXgsYOt385wgFBHZjHSuv505lXZ98i42AIVEpVHcC+hwIp6IjmdnUdf+RvFBHBEg1cxG00OBSPoR1NvQ1bZ370oOOlH1cJ2RL0HyAkIbUA7reVC64JNxKKp5okdH9cq4VuI9lU+yC0+oNB7dPXC+GdPWj6suZbXGKsqMRKm1ZgtYYwGsnjH9KhWw/dokZZVb5fLxiUZOm4UFSalEZIvUeAOCrFSoznRMMfUYQJEBEBaqHQuLHtdL5HkDfg==\"}");

			KeyStore ks;
			REQUIRE_NOTHROW(ks.Import(keystoreJson, passwd));
			const ElaNewWalletJson &walletData = ks.WalletJson();
			REQUIRE(walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic().empty());
			REQUIRE(walletData.xPrivKey().empty());
			REQUIRE(walletData.GetM() == 1);
			REQUIRE(walletData.GetN() == 1);


			REQUIRE(walletData.RequestPubKey() == "02644bccfc0abc15fe9b800ba41d323665c3935aacf761900320c036879e4db1ab");
			REQUIRE(walletData.RequestPrivKey() == "c56db84b2907d6364437b3ac77a6bf56467e3c8d47327053189253f8700b4cec");
			REQUIRE(walletData.xPubKey() == "xpub6D9d1MdbeSWkBMh9eMXHvdLNnXPLSZysWZBD4WDeRFF4W59qimVy29MF2Nnm2rGLVGRguFw4QF1DF7BTbS5fUZ91y354bJcJjn4FxvKywrv");
			REQUIRE(walletData.GetPublicKeyRing().size() == 1);
			REQUIRE(walletData.RequestPubKey() == walletData.GetPublicKeyRing()[0].GetRequestPubKey());
			REQUIRE(walletData.xPubKey() == walletData.GetPublicKeyRing()[0].GetxPubKey());

			// keystore from web wallet and without private do not support owner public key
			REQUIRE(walletData.OwnerPubKey().empty());
			REQUIRE(walletData.GetCoinInfoList().size() == 0);
			REQUIRE(!walletData.SingleAddress());
		}

		SECTION("multi sign keystore with private key and without passphrase") {
			const std::string passwd = "s12345678";
			//const std::string addr = "8ZmSJ3Kz7nSmPBdXYmspYYHKqU11617ks5";
			const std::string &mnemonic = "令 厘 选 健 爱 轨 烯 距 握 译 控 礼";
			nlohmann::json keystoreJson = nlohmann::json::parse("{\"iv\":\"2ZKIcWrVmEPp3I2B/4WYcA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"4+aks85XBtI=\",\"ct\":\"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\"}");

			KeyStore ks;
			REQUIRE_NOTHROW(ks.Import(keystoreJson, passwd));
			const ElaNewWalletJson &walletData = ks.WalletJson();
			REQUIRE(!walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);

			REQUIRE(2 == walletData.GetM());
			REQUIRE(3 == walletData.GetN());

			uint512 seed = BIP39::DeriveSeed(mnemonic, "");
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			REQUIRE(Base58::CheckEncode(rootprv.extkey()) == walletData.xPrivKey());
			REQUIRE(Base58::CheckEncode(rootprv.getChild("44'/0'/0'").getPublic().extkey()) == walletData.xPubKey());

			REQUIRE(walletData.GetN() == walletData.GetPublicKeyRing().size());
			std::vector<bytes_t> pubkeys;
			for (size_t i = 0; i < walletData.GetPublicKeyRing().size(); ++i) {
				bytes_t pubkey(walletData.GetPublicKeyRing()[i].GetRequestPubKey());
				pubkeys.push_back(pubkey);
			}
			Address addr1(PrefixMultiSign, pubkeys, walletData.GetM());
			REQUIRE(!walletData.SingleAddress());
			//REQUIRE(addr1.String() == addr);
		}
	}

	SECTION("old keystore from spv wallet") {
		SECTION("with private key and without passphrase") {
			const std::string passwd = "Abcd1234";
			const std::string mnemonic = "卢 口 宴 点 睛 量 旦 奥 沫 洪 鲁 距";
			const std::string passphrase = "";
			nlohmann::json keystoreJson = nlohmann::json::parse(
				"{\"adata\":\"\",\"cipher\":\"aes\",\"ct\":\"ugZV9gZlLiUiYZ0wCkxjP+/OFhoRF/bYtwEpBdilpIkhiNSvOgxjRX8R6wyT84gR809Sd/Stc3moH+zAEw1qgOKeLatz33r+H2K5OnpR1MIkflpYI2XFPZBt0+TckN/OsTq1v+QiFRlSLtdSJWdWS9o943nBxLc81x7HbcdPpGTaPAXUj6HNp1V5HW+qLdc9lUkL0EYPRZ//9VLJY9AdhuNhsOZxBokAVYy4qobQ1p4tWaj4v8/qYUMcokdKeDYZmsdY8r5wVPmHehACkBMAfPcNbzwS32M8wHq2WeNDie/oPmrKZLn74vYURweSBAUqF4x/QDbitJkc7aonSebx0qOvLhrJap5MtP8R19quGNqmT2Vb7aPsPTzdExrRPXJyZZ4s91iJYWYkq9+tSmWFGUdbMnJv8hlVKNfdxM/DzaBvwFyGgIR58VvESFOQwuPLCUEgUPQjjDhE31+CqQUwLH2SSDNE9lNgGGRzCKdyXFKIYbk3cF7/E1UzHXnt0gOBxneGCpqE+Qo4ChQ+4YNxdRdAwnrbYJbhxRvbEoCguGgiWnBmrJUODGBntM+OEQexNqJ9hoAPBezm4hrlAz274owPKhRnsjpZrJ43ChvVTiMUji6RpignlpV5mYcjD6dpU7tf3yeibD5tdhzb7vsBe3BLqW5x4z66H2JbLz542ts0qaMwpX0/SE3r7vTBsH5ZxrHYzObXbUqqKAUfyXhVfoDCC5Zm6CgaKhbuxZvtxaYnatNkwYCyS+ihy0By6LvNmFyZsDUGq3tiRAkpNRIxJSuzFbnYGNgvoGj9W6MNmbgwL5kUiHOTHd+jloVlmKkmGdTaxBAzgQoX8NyM35pR7zHc6dccuOQBT7IcEHA0QF4WXEwQR3lztX/zHYsWBSlnYn782suDBKtF2pmxW7vbLlylyn804Ni1HjaeRQYztZe5t2ld3Bum4rSIW7+tQux2jqNAitayEvxb4j2owPLbZwKEOwDik4JJG65smQ1FfwNkiusrWRNyjJgBlgZ5HchrPSwGY48Z8+J6oRRZOqJf4QyTfT7xwq0gYvUXBSQjj+ZtmZprToohO9N8RV928AEwHzUlFamS2a/DtZ03qMnXahKE4K5EophFIE0FbUXrxxbZYj2v3H7s7xoVoDAVIYwFyEhapmD69aglei0a8lcfuryB3a5OMirnW3BJblD8t8MfTncoeoRhT2qYhrDfU6/A+tL3ArKaVI9IM070gVTy24Co/gk1svfKdsnKDRq4x80aydpcuw9rilS+gbAHRcnts/vc9oqelLzChKFpGFRbkBgccKNFlO4bhPnwGFjfVc3seAw31zFOu3ixO42o8U1s1vlSyIMmroMnV4hE609FJlyQXK4I9htl8zrxx5J5u7DMjy3w0nWIckpz04+oSXujgaK9p/EYdIQDGQq0sQzoazeXxTxxTPNLWEf/chepgBnd3L+sxKRtQKTEksFSJqlasy4qMWo+IxEayFlRWNarUA60xWywCdEffNAqxxLRR/WoExXUuz2lZnpk3MbJ7ZkEP+CuiObfiJNt5dGkUkaEjyhakDv9Ug2iDulu1eDemMJipDflndNux2xXGKU2K3hOe2GfMxNcbWdq4ll44qx/uf+kqr/KYq4bxJ8+3uW8T85ORFKVU/RypRU884q+vGYlF9KpPlIxWUCA7pMtV3ClZ9YbWlfexKacL2VxZbVxsxAt4HS88PhPW4reafL3KY23Au9V07XZp9XKdu4EzM14OUzA0j8yYH52/X5iXEGD+sxrZogBtl3l4xAesO981spamZhjLdeBWUJnA6X1JY5MY++Vh4I7stvi0R0LQ4f3LjMXzCdCLDbrNchPtV8m/VlRXBxfw4OJ6FOxvkKEFqGBygOYH613cqa6HW/AlqwhTN42azUYBXMbUHPv3j23FiRrSzjg0OzEnkEVZ20yNn8B8JaKg+9UkWKOy9Gyqrz/LfMsIOs=\",\"iter\":10000,\"iv\":\"aknqq5k9rY7GfrIzSXLt1Q==\",\"ks\":128,\"mode\":\"ccm\",\"salt\":\"HoI5PkRT1Wc=\",\"ts\":64,\"v\":1}");

			KeyStore ks;
			REQUIRE_THROWS(ks.Import(keystoreJson, passwd));
#if 0
			const ElaNewWalletJson &walletData = ks.WalletJson();
			REQUIRE(!walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);

			REQUIRE(1 == walletData.GetM());
			REQUIRE(1 == walletData.GetN());

			uint512 seed = BIP39::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			HDKeychain firstKey = rootprv.getChild("44'/0'/0'/0/0");
			Address addr(PrefixStandard, firstKey.pubkey());
			REQUIRE("EPmxXC5orLrXd1FJK8Skz5tmf2gDreh8fd" == addr.String());

			std::string ownerPubKey = rootprv.getChild("44'/0'/1'/0/0").pubkey().getHex();
			REQUIRE(walletData.OwnerPubKey() == ownerPubKey);

			const std::vector<CoinInfoPtr> &coinInfos = walletData.GetCoinInfoList();
			REQUIRE(2 == coinInfos.size());
			REQUIRE(coinInfos[0]->GetChainID() == "ELA");
			REQUIRE(coinInfos[0]->GetEarliestPeerTime() != 0);
			REQUIRE(coinInfos[0]->GetVisibleAssets().size() == 1);
			REQUIRE(coinInfos[0]->GetVisibleAssets()[0] == Asset::GetELAAssetID());
			REQUIRE(coinInfos[1]->GetChainID() == "IDChain");
			REQUIRE(coinInfos[1]->GetEarliestPeerTime() != 0);
			REQUIRE(coinInfos[1]->GetVisibleAssets().size() == 1);
			REQUIRE(coinInfos[1]->GetVisibleAssets()[0] == Asset::GetELAAssetID());
			REQUIRE(!walletData.SingleAddress());
#endif
		}

		SECTION("with private key and with passphrase") {
			const std::string passwd = "Abcd1234";
			const std::string mnemonic = "symptom glove trigger fossil stage valid cherry bid dolphin kit promote vital";
			const std::string passphrase = "Abcd1234";
			nlohmann::json keystoreJson  = nlohmann::json::parse("{\"adata\":\"\",\"cipher\":\"aes\",\"ct\":\"W21+P8TWNznXpQlJlSqrbWMbZZof7rmDkwK0oTYPjqhVsUNf+PF3EhNPsLV7OSJOF7SwjCl0wB+6xEUjxXNcuqMBjQQJ4Mduv3APesxupdh2H43a56ZlAZV9wIT5Tk1YaUIlBsIqGgJ+ZTyHYohWj6V9+IH6mJSxRSwFhr9IHyL9Z6KQqc5AngLzMpRSC4zAnpDxZx/b4N5hqS0sm42MhgZxnXDC31100PR2Mzy/PFoAgcN7whT4eCsF3v2nJ/RG3BTrw9zz4sCuVwTH8vRs7qSB9KWKoNUsfG2qtGtidc8KPg5pwHZnZlqK69Yp9vifFlQEo4e9miMSzKRwce4sUe/lO1A7HCZt9Vq8lDJ1PerX8KMeLyy9fsqkjDpRhENs1Vopa6Xr08ksZn8YhcRWYc1p/K6tgI7gLoZpD+hG22HDdNHKY+HVskQsL5LhR5wQE609gPO2zs4JTK4TB6hwqD8zcfkDNJqIFs+3iqAmfyDH3TfXGDyrGkTIHUM5HjcVk4bECBv84lI4lX8y49yapwQdigXis93uSR4EAxBqHsmy9mCTIjUDjTcltJVwgsqt6zBrfJy0qR7/qzY9nR8dnOo6i2Ep3kfsf/MSHAhovtfRcXW6lwvDs3lKMDAnQqx1Py0+7irNYvdy4GfwPIopJgcoZEC5x2JhTnXaN0zk7b8XyGy9MImQQnzws6t0OEJLHLXYwZyMS/7+kx5XwsmSrEgFUVtSlx1Gcfj8RVzKjv8oADrh1obEo8eTjiHL1p2dE6dRH0hkgTsm/0sJz9FeNJqmOa8kTuYg6+8gEfmjiHLd/qFAb5sJg0WLThAIWNXGFoLSld/9e5tO1xBMjKrt1CtAUPq++Lo8q1uAmdZXnWkeUs2DTMGdPNdcDY88BpbKyYOlVh1Wq2n9hSjrBySANx+UVxJfyZXOwcmw23eKsNA31V0yhGvSO4ZgLsIgmgtbSWCRItX5XT6YG2GuCN8R3Jx5lE6rzoQrWzSZgGTooRTsw7T8qmVy6E8DLW1+OiZS5n5vyl0pI+dxDx75Yt23O34dh1nvJLmGN0oQEabA6qCXBxdgf/O0y319HgnjEusmoyqG4hs6GGQ+u8qxUviXMFreQAersgsNoF8Jo+24aRNHTq+fEcjOh5DFvSlYFh763Eu7GSqS2fLnIRrBW/4Y7WMdhdEbFxLueDAWdt0Nu4GXZsGLCUaAjGlzfEwgHxoBTca6/4HDeg7XbrbemKLU+KQFdfxU4zlLxJyR5zHRnRVMmFHQJf3nYkjYAuKkdgv9+nvqEfeX0Xn5/hdNgTAncDPfNKIt5NoJrMVyYGFjpnvRZZ4YxRW56+TK0OdTvVsc6Sp1gsnp4y3MkHNxvMktjZKPl2TPDi1diUUCoKqkP0pjADZSwxvo4vOWKLPKCivcREi0ID8TIVIJ+SM0jO6fcQ==\",\"iter\":10000,\"iv\":\"oD/5rjOJ8wQpv1jS8QH6kw==\",\"ks\":128,\"mode\":\"ccm\",\"salt\":\"iIjm5NrNxR0=\",\"ts\":64,\"v\":1}");

			KeyStore ks;
			REQUIRE_THROWS(ks.Import(keystoreJson, passwd));

#if 0
			const ElaNewWalletJson &walletData = ks.WalletJson();

			REQUIRE(walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);

			REQUIRE(1 == walletData.GetM());
			REQUIRE(1 == walletData.GetN());

			uint512 seed = BIP39::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			HDKeychain firstKey = rootprv.getChild("44'/0'/0'/0/0");
			Address addr(PrefixStandard, firstKey.pubkey());
			REQUIRE("EWkMVjB48znLGXCNNpLzf2rLmnX1iU91FP" == addr.String());

			std::string ownerPubKey = rootprv.getChild("44'/0'/1'/0/0").pubkey().getHex();
			REQUIRE(walletData.OwnerPubKey() == ownerPubKey);

			const std::vector<CoinInfoPtr> &coinInfos = walletData.GetCoinInfoList();
			REQUIRE(1 == coinInfos.size());
			REQUIRE(coinInfos[0]->GetChainID() == "ELA");
			REQUIRE(coinInfos[0]->GetEarliestPeerTime() != 0);
			REQUIRE(coinInfos[0]->GetVisibleAssets().size() == 1);
			REQUIRE(coinInfos[0]->GetVisibleAssets()[0] == Asset::GetELAAssetID());
			REQUIRE(!walletData.SingleAddress());
#endif
		}

		SECTION("with private key and with passphrase and single address") {
			const std::string passwd = "Abcd1234";
			const std::string mnemonic = "ride chalk song document stem want vocal win birth hotel pottery kitchen";
			const std::string passphrase = "Abcd1234";
			nlohmann::json keystoreJson = nlohmann::json::parse("{\"adata\":\"\",\"cipher\":\"aes\",\"ct\":\"/17CPdZfIS+iT7SWtt0x7f4vsokkOad9tVoOxt9gt0IWf0347BNqTC02NnDEftlE6AzqLxMQ7TY3QSTK/iOKx7gQdYm+1qU1jDCFjrwFvs27KHWtfFrW3XHYkzk2XQiZ2pf/EoY4w3K0Z8L4fudr24btJYP6M+/IJews1atdWDmCd3/FonsRJEaQfeDMMr4nBif/0+fcMwbcRprWX+qxQrw3A/VSZp6vLHROG7VA4mKCxHUOryCofgbJfo9z/6bQQuXL6R4cYEUnYqn7fQAiv7SNv54AlcgiVhnfYDBl6mqUMrmvsX6pYSzQvst6p3a2PCWln5YkLDFEsHaxqtDeLSUZz7Z0VUCXnLXM103CoIXMZbHE5M1ILuF+tOomORy1po4Xk32cyH78h0Dnuz5qGXXPFc/NsZ9v1oXERUnVPhyYTQE+GMaehMSXEttdlZRg4vgSAwro3kgdt8o39ou81s4KPjv9PIowIuFDmT0VKFvM+QKj8MTUSVEQPCdJjsrg960AycVo00/YZpD2AJbNjSulouiizZ61B2TRIGa0afbAl5D8Mz9xRbbfje2Nfc81il+X2dSgT3DX4E/ZbVCFVHuWgbDYlcftq6YfGd231WkgZlCEYJQG6Ab2jiVHLe8yyz0jrVtz5BPueqKrtDbvVHDQ7g79J7UJvG33vdbtsSNczA+oBcxaHiJ1qoUmJyO2W4x3u8vv6mkP2/AWaFaNBHEheBJ3j9DU898766K0ElGZJky4pucZmyWsvJqteKUlohfGpTYlVUaRrXxIhIx2cWelYKrQTni6iDzfOP1N6WvYQlvCq8uAdsQHt6H7wbM5QTuJp22vdiIpFkhYLII5acxNATMHpHZMIrelTsaHr/dqEtSmvZOb0v/qlaPYsDovoLBeW4G5vx53Mz6KFH6cdwPTZL+NKsWim8wcARD2VY7U7TvNoZqjsn9E32sapXCVjRtM5zzdhIYzqU1/UNFHxCUZ0o5vvW0P48Y1QUOIssD3h5hw1GiVg0A/VRW/PxB96wQjVeYcuAquDcn2sqpA6NVSkXsg+T7p+uHiqwx85wZ1zfRuZHhq3ZpmNAIzRMm9uyngFFVoSBewCzRvsMP/kjj5mqLx8igjdmUXa2hidDuAXVO7KdFvx5KFgpLM2fmhxcl1GxSY7OGpmU4gLncLKD1C5oBnCW+QW/BUl6e1++K7YrtpGiZrbkV4WLqHdPF1SNciATdyAMvQ21b/6MBOKFc3wh8CjfOyRhK8QnAmFauOqqFx5F4NXtx89wXQUMhf5tVnLp0NVxzmsU7BQSBVZAyJY/27tmbTR0Nv3UDbWscGAX7uaUJyvAxZJ3YXSLkZ8ITYb/w2Il/WvcTMU6v5guO8Xm+jkLk21JsrzZAG+pgjquWBaeqJ2/yF33TVMsBZT5tzLQonN6YX\",\"iter\":10000,\"iv\":\"vwYqwOxaqkGA9JQetCHtcg==\",\"ks\":128,\"mode\":\"ccm\",\"salt\":\"DVFsO/mhTm0=\",\"ts\":64,\"v\":1}");

			KeyStore ks;
			REQUIRE_THROWS(ks.Import(keystoreJson, passwd));

#if 0
			const ElaNewWalletJson &walletData = ks.WalletJson();

			REQUIRE(walletData.HasPassPhrase());
			REQUIRE(walletData.Mnemonic() == mnemonic);

			REQUIRE(1 == walletData.GetM());
			REQUIRE(1 == walletData.GetN());

			uint512 seed = BIP39::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootprv(hdseed.getExtendedKey(true));

			HDKeychain requestKey = rootprv.getChild("1'/0");
			REQUIRE(walletData.RequestPubKey() == requestKey.pubkey().getHex());
			REQUIRE(walletData.RequestPrivKey() == requestKey.privkey().getHex());

			HDKeychain firstKey = rootprv.getChild("44'/0'/0'/0/0");
			Address addr(PrefixStandard, firstKey.pubkey());
			REQUIRE("ENjw5MoRDuKRsuZ6WhhbzxmJ1yuSvecLSf" == addr.String());

			std::string ownerPubKey = rootprv.getChild("44'/0'/1'/0/0").pubkey().getHex();
			REQUIRE(walletData.OwnerPubKey() == ownerPubKey);

			const std::vector<CoinInfoPtr> &coinInfos = walletData.GetCoinInfoList();
			REQUIRE(1 == coinInfos.size());
			REQUIRE(coinInfos[0]->GetChainID() == "ELA");
			REQUIRE(coinInfos[0]->GetEarliestPeerTime() != 0);
			REQUIRE(coinInfos[0]->GetVisibleAssets().size() == 1);
			REQUIRE(coinInfos[0]->GetVisibleAssets()[0] == Asset::GetELAAssetID());
			REQUIRE(walletData.SingleAddress());
#endif
		}

	}
}