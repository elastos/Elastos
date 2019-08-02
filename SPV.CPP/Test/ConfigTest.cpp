// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <SDK/SpvService/Config.h>
#include <SDK/Common/Log.h>
#include <SDK/P2P/ChainParams.h>

#include <fstream>
#include <boost/filesystem.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Config test", "[Config]") {
	Log::registerMultiLogger();

	SECTION("ELA") {
		const std::string chainID = "ELA";
		const std::string genesisAddress = "";
		const std::string pluginType = "ELA";
		uint32_t index = 0;

		SECTION("MainNet") {
			const std::string netType = "MainNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 20338);
			REQUIRE(chainParams->MagicNumber() == 2017001);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-mainnet-002.elastos.org");
			REQUIRE(dnsSeed[1] == "node-mainnet-006.elastos.org");
			REQUIRE(dnsSeed[2] == "node-mainnet-014.elastos.org");
			REQUIRE(dnsSeed[3] == "node-mainnet-016.elastos.org");
			REQUIRE(dnsSeed[4] == "node-mainnet-021.elastos.org");
			REQUIRE(dnsSeed[5] == "node-mainnet-003.elastos.org");
			REQUIRE(dnsSeed[6] == "node-mainnet-007.elastos.org");
			REQUIRE(dnsSeed[7] == "node-mainnet-017.elastos.org");
			REQUIRE(dnsSeed[8] == "node-mainnet-022.elastos.org");
			REQUIRE(dnsSeed[9] == "node-mainnet-004.elastos.org");
			REQUIRE(dnsSeed[10] == "node-mainnet-023.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[1].Hash() == uint256("333d9a0e874cf1b165a998c061c2f8be8e03ce31712046d001823d528e1fec84"));
			REQUIRE(checkPoints[1].Height() == 2016);
			REQUIRE(checkPoints[2].Hash() == uint256("6347d62e227dbf74c8a4c478fa4f8d351f24a3db3f5a01aad32ea262b6e6f6aa"));
			REQUIRE(checkPoints[2].Height() == 12096);
		}

		SECTION("TestNet") {
			const std::string netType = "TestNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 21338);
			REQUIRE(chainParams->MagicNumber() == 2018101);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-testnet-005.elastos.org");
			REQUIRE(dnsSeed[1] == "node-testnet-006.elastos.org");
			REQUIRE(dnsSeed[2] == "node-testnet-007.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[4].Hash() == uint256("0fd0ecfabdd3f3405b9808e4f67749232d23404fd1c90a4e2c755ead8651e759"));
			REQUIRE(checkPoints[4].Height() == 20160);
			REQUIRE(checkPoints[5].Hash() == uint256("d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c"));
			REQUIRE(checkPoints[5].Height() == 26208);
		}

		SECTION("RegTest") {
			const std::string netType = "RegTest";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 22338);
			REQUIRE(chainParams->MagicNumber() == 2018211);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-regtest-105.eadd.co");
			REQUIRE(dnsSeed[1] == "node-regtest-106.eadd.co");
			REQUIRE(dnsSeed[2] == "node-regtest-107.eadd.co");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[4].Hash() == uint256("3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a"));
			REQUIRE(checkPoints[4].Height() == 26208);
			REQUIRE(checkPoints[5].Hash() == uint256("8dfe3ec6125c06c40bcd9a72963b6e60dc214b7a61ad2e805b138f60ac7e92de"));
			REQUIRE(checkPoints[5].Height() == 34272);
		}

	}

	SECTION("IDChain") {
		const std::string chainID = "IDChain";
		const std::string genesisAddress = "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ";
		const std::string pluginType = "SideStandard";
		uint32_t index = 1;

		SECTION("MainNet") {
			const std::string netType = "MainNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index()== index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 20608);
			REQUIRE(chainParams->MagicNumber() == 2017002);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-mainnet-026.elastos.org");
			REQUIRE(dnsSeed[1] == "node-mainnet-027.elastos.org");
			REQUIRE(dnsSeed[2] == "node-mainnet-028.elastos.org");
			REQUIRE(dnsSeed[3] == "node-mainnet-029.elastos.org");
			REQUIRE(dnsSeed[4] == "node-mainnet-030.elastos.org");
			REQUIRE(dnsSeed[5] == "node-mainnet-031.elastos.org");
			REQUIRE(dnsSeed[6] == "node-mainnet-032.elastos.org");
			REQUIRE(dnsSeed[7] == "node-mainnet-033.elastos.org");
			REQUIRE(dnsSeed[8] == "node-mainnet-034.elastos.org");
			REQUIRE(dnsSeed[9] == "node-mainnet-035.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[4].Hash() == uint256("d342ee0817e30a57a515ccfaaf836e7314aac1235d0659d5e6ffc46671ae4979"));
			REQUIRE(checkPoints[4].Height() == 14112);
			REQUIRE(checkPoints[5].Hash() == uint256("fc4e7ba460e38964faeb33873418c9fcd3ae2648a543cc1e8f5c47806b593ac8"));
			REQUIRE(checkPoints[5].Height() == 18144);
		}

		SECTION("TestNet") {
			const std::string netType = "TestNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 21608);
			REQUIRE(chainParams->MagicNumber() == 2018102);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-testnet-011.elastos.org");
			REQUIRE(dnsSeed[1] == "node-testnet-012.elastos.org");
			REQUIRE(dnsSeed[2] == "node-testnet-013.elastos.org");
			REQUIRE(dnsSeed[3] == "node-testnet-014.elastos.org");
			REQUIRE(dnsSeed[4] == "node-testnet-015.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[4].Hash() == uint256("46a9dd847b62f278a26a93e3f7a4cc135b30754e07f2754d6ac0baf6870c8060"));
			REQUIRE(checkPoints[4].Height() == 8064);
			REQUIRE(checkPoints[5].Hash() == uint256("09cdb3d720a7095c7023241361ca8a317c75273fbdd0e5dc268667683e360780"));
			REQUIRE(checkPoints[5].Height() == 10080);
		}

		SECTION("RegTest") {
			const std::string netType = "RegTest";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 22608);
			REQUIRE(chainParams->MagicNumber() == 2018202);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-regtest-116.eadd.co");
			REQUIRE(dnsSeed[1] == "node-regtest-117.eadd.co");
			REQUIRE(dnsSeed[2] == "node-regtest-118.eadd.co");
			REQUIRE(dnsSeed[3] == "node-regtest-119.eadd.co");
			REQUIRE(dnsSeed[4] == "node-regtest-120.eadd.co");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[0].Hash() == uint256("56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3"));
			REQUIRE(checkPoints[0].Height() == 0);
			REQUIRE(checkPoints[1].Hash() == uint256("fb29c3de03ab8dbdc9e50ee882efc9a8115697a7681593ec1c587dd8ddc0da70"));
			REQUIRE(checkPoints[1].Height() == 2016);
		}
	}

#if 0
	SECTION("TokenChain") {
		const std::string chainID = "TokenChain";
		const std::string genesisAddress = "XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4";
		const std::string pluginType = "SideStandard";
		uint32_t index = 2;

		SECTION("MainNet") {
			const std::string netType = "MainNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index()== index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 20618);
			REQUIRE(chainParams->MagicNumber() == 2019004);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-mainnet-002.elastos.org");
			REQUIRE(dnsSeed[1] == "node-mainnet-003.elastos.org");
			REQUIRE(dnsSeed[2] == "node-mainnet-004.elastos.org");
			REQUIRE(dnsSeed[3] == "node-mainnet-005.elastos.org");
			REQUIRE(dnsSeed[4] == "node-mainnet-006.elastos.org");
			REQUIRE(dnsSeed[5] == "node-mainnet-007.elastos.org");
			REQUIRE(dnsSeed[6] == "node-mainnet-008.elastos.org");
			REQUIRE(dnsSeed[7] == "node-mainnet-009.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[4].Hash() == uint256("ca8df7f2004dc5ff3b500967a24fc1f1d709ff804119ea86f9fc05d85de9dec6"));
			REQUIRE(checkPoints[4].Height() == 8064);
			REQUIRE(checkPoints[5].Hash() == uint256("58b25e8f4aed9346b8e23b224242ddb2f42d8d6c7d00077beaf97684ca02474e"));
			REQUIRE(checkPoints[5].Height() == 10080);
		}


		SECTION("TestNet") {
			const std::string netType = "TestNet";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 21618);
			REQUIRE(chainParams->MagicNumber() == 2019104);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-testnet-002.elastos.org");
			REQUIRE(dnsSeed[1] == "node-testnet-003.elastos.org");
			REQUIRE(dnsSeed[2] == "node-testnet-004.elastos.org");
			REQUIRE(dnsSeed[3] == "node-testnet-005.elastos.org");
			REQUIRE(dnsSeed[4] == "node-testnet-006.elastos.org");
			REQUIRE(dnsSeed[5] == "node-testnet-007.elastos.org");
			REQUIRE(dnsSeed[6] == "node-testnet-011.elastos.org");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[0].Hash() == uint256("b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e"));
			REQUIRE(checkPoints[0].Height() == 0);
		}

		SECTION("RegTest") {
			const std::string netType = "RegTest";
			Config config("Data", netType);

			ChainConfigPtr chainConfig = config.GetChainConfig(chainID);
			REQUIRE(chainConfig->ID() == chainID);
			REQUIRE(chainConfig->Index() == index);
			REQUIRE(chainConfig->NetType() == "MainNet");
			REQUIRE(chainConfig->MinFee() == 10000);
			REQUIRE(chainConfig->FeePerKB() == 10000);
			REQUIRE(chainConfig->DisconnectionTime() == 300);
			REQUIRE(chainConfig->PluginType() == pluginType);
			REQUIRE(chainConfig->GenesisAddress() == genesisAddress);

			const ChainParamsPtr &chainParams = chainConfig->ChainParameters();
			REQUIRE(chainParams->StandardPort() == 22618);
			REQUIRE(chainParams->MagicNumber() == 2019204);
			REQUIRE(chainParams->Services() == 0);
			REQUIRE(chainParams->TargetTimeSpan() == 86400);
			REQUIRE(chainParams->TargetTimePerBlock() == 120);

			const std::vector<std::string> &dnsSeed = chainParams->DNSSeeds();
			REQUIRE(dnsSeed[0] == "node-regtest-102.eadd.co");
			REQUIRE(dnsSeed[1] == "node-regtest-103.eadd.co");
			REQUIRE(dnsSeed[2] == "node-regtest-104.eadd.co");
			REQUIRE(dnsSeed[3] == "node-regtest-105.eadd.co");
			REQUIRE(dnsSeed[4] == "node-regtest-106.eadd.co");
			REQUIRE(dnsSeed[5] == "node-regtest-107.eadd.co");

			const std::vector<CheckPoint> &checkPoints = chainParams->Checkpoints();
			REQUIRE(checkPoints[0].Hash() == uint256("b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e"));
			REQUIRE(checkPoints[0].Height() == 0);
		}
	}
#endif

}