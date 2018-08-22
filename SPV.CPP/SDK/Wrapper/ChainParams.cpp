// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRChainParams.h>
#include <BRMerkleBlock.h>
#include <Core/BRChainParams.h>
#include <SDK/Common/Log.h>

#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace ElaWallet {

		// MainChain Reg Net
		const BRCheckPoint MainChainRegNetCheckpoints[] = {
			{ 0,      uint256("6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3"), 1513936800, 486801407 },
			{ 2016,   uint256("99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767"), 1517940844, 503906048 },
			{ 4032,   uint256("37c594c2502c8efca228e32dd3c60dd867e449114afb55d4d4b19d6e39eda447"), 1518102448, 503371082 },
			{ 6048,   uint256("0e5c7c2df1f3357cd4355b19d9a33def5d754c4c5f19b2f40c5b2edf6b887df6"), 1522420376, 503363796 },
			{ 8064,   uint256("fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158"), 1522645593, 494006531 },
			{ 16128,  uint256("3751c065602216af876df0d9be26392ef38ba31604e8834beedee6536f5077ad"), 1523623153, 494569905 },
			{ 32256,  uint256("9ba5fec5acaf23302110e333b022a074de88b4e36b9264e98297cbaec09f591f"), 1526225370, 503370323 },
			{ 64512,  uint256("d31da57024096eed3d63671b19bcfde74e22936bd5f13b20d88addf8b5d79932"), 1531309241, 503350802 },
			{ 70560,  uint256("7abc2062a9bea209aa638adae6076461ca989fa1d24d4719cc98ba70e99d1a15"), 1532183322, 503380026 },
			{ 80640,  uint256("bcadac789eec2262f099cc0f57ef53937520ddbc298cb2341d00bce1cbabd7ec"), 1533423380, 503380311 },
			{ 84672,  uint256("69d6c8cbb97ee80f11a7deb0903f20dcd7750766ffaae572a310f814e4b89ac7"), 1533906003, 503380020 }
		};

		const char *MainChainRegNetDNSSeeds[] = {
			"node-regtest-002.elastos.org",
			"node-regtest-003.elastos.org",
			"node-regtest-004.elastos.org",
			NULL
		};
		const ELAChainParams MainChainRegNetParams = {
			.Raw = {
				MainChainRegNetDNSSeeds,
				22866,        // standardPort
				20180627,    // magicNumber
				0,            // services
				nullptr,
				MainChainRegNetCheckpoints,
				sizeof(MainChainRegNetCheckpoints) / sizeof(*MainChainRegNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "RegNet"
		};


		// MainChain Test Net
		const BRCheckPoint MainChainTestNetCheckpoints[] = {
			{ 0,       uint256("6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3"), 1513936800, 486801407 },
			{ 2016,    uint256("99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767"), 1517940844, 503906048 },
			{ 4032,    uint256("37c594c2502c8efca228e32dd3c60dd867e449114afb55d4d4b19d6e39eda447"), 1518102448, 503371082 },
			{ 6048,    uint256("0e5c7c2df1f3357cd4355b19d9a33def5d754c4c5f19b2f40c5b2edf6b887df6"), 1522420376, 503363796 },
			{ 8064,    uint256("fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158"), 1522645593, 494006531 },
			{ 10080,   uint256("4dbf87bdf287319958c480fbc9249cbc93847a54f05ccb13f8283de1cf365158"), 1522878764, 493221476 },
			{ 12096,   uint256("b8c86da76802ba3b767a30541a1a32db561847b67a93bdc11fd9e06ca30dd2d4"), 1523128962, 492960382 },
			{ 14112,   uint256("20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e"), 1523380720, 494547269 },
			{ 16128,   uint256("3751c065602216af876df0d9be26392ef38ba31604e8834beedee6536f5077ad"), 1523623153, 494569905 },
			{ 18144,   uint256("a299a6fde95bde058996253fb090658ca7b19b6366c48c053ac43302728d380d"), 1523866303, 494684787 },
			{ 20160,   uint256("0fd0ecfabdd3f3405b9808e4f67749232d23404fd1c90a4e2c755ead8651e759"), 1524792298, 503351573 },
			{ 22176,   uint256("a8baf0a258a9c1a760d09a2bf038bcfcceec4d2e652690bfc45c95c885487b62"), 1525053347, 503359531 },
			{ 24192,   uint256("bda8c13b6d57cecf02e31d5cadf09ded4a9525698b4958a2ae069e1a085a8240"), 1525299195, 503357722 },
			{ 26208,   uint256("d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c"), 1525542345, 503358411 },
			{ 28224,   uint256("360984231dac82bd6dbd6b23437c789427888c8a1eb815f75e0181e7a4cf39db"), 1525788808, 503361924 },
			{ 30240,   uint256("5a9f2456d12b5f335a4d1ecfad28cadd600b7a38957a72d0c087573f4aa8280e"), 1526024685, 503359636 },
			{ 32256,   uint256("0c03acf34f4cd5df02062e658f91dfe884788831b940b18a136f7d33adae00b0"), 1526270878, 503361032 },
			{ 34272,   uint256("d0a25ed7d01706dc11e66737e13e1eef9f2cfbab369cf6e1f507af6809d75d3f"), 1526516129, 503360832 },
			{ 36288,   uint256("2c72b7b2e069e399613ee13ad202f8453ea621162e86387a73773a764ad2b80e"), 1526754193, 503360391 },
			{ 38304,   uint256("ea313df1089b8b001c40565aa5e90d17da51885a29459b966d0019ac7dd90002"), 1526997348, 503361573 },
			{ 40320,   uint256("a61ec5bb0cfe02ac77dc4f38bf975c2d59e437941d91f4bab649a8256e70d755"), 1527224546, 503353415 },
			{ 42336,   uint256("5889b5b496311147aecf78caf9a6e6715175e8d8465c7efc1476d78e722e421e"), 1527464757, 503351150 },
			{ 70560,   uint256("551499f42d2dc3f8dc7875720590d437ccd91a60c309e38be972131a2bca65c1"), 1530853521, 494674545 },
			{ 84672,   uint256("bb0a0e077f031b68b4d300b462828c40f87014d39295b85b5674dee5e3f850aa"), 1532629387, 503383112 },
			{ 86688,   uint256("a25b7207c98cbbede93db67ca41a0ac308d198c556eea955ff63f52b0b70131a"), 1532870279, 503379835 },
			{ 88704,   uint256("15a9b8905e63891f7f26bb9a3b336c2ec1c4d19effe49f87d0a905273a3a0380"), 1533116422, 503382206 },
			{ 90720,   uint256("801a188478b77b8f0b07320ef6ff3ccc7d6931804fa328f57e91176d65ee9326"), 1533356482, 503378206 },
			{ 92736,   uint256("c5134b4d7b275a6109cf2f1e7c4f31ad96a4c6c19d2bfd98e7b655d9d49d5723"), 1533598732, 503377199 },
			{ 94752,   uint256("372df4fe8ebb396f8298d3fa596ab6c825fb9d16a7d93ba15ed5372ed9382dfb"), 1533848032, 503381789 },
			{ 96768,   uint256("95977fa25966b0c4f6351bca811deca8baf90e765b72f091e21084e1c41558cc"), 1534085167, 503380421 },
			{ 98784,   uint256("552732e51a44b3cf99992447b9b3f79226fa90f0f9f1b47e15836abc2c334322"), 1534328247, 503380456 },
			{ 100800,  uint256("3d03a76f1f76bd91673a32d670d4b9dfb18996295b8823202534fa92693fdd9c"), 1534564416, 503375063 },
			{ 102816,  uint256("b5e07af958149f5332336be914dbeba25da11dbd81cc0be9421352cb6fb02379"), 1534812379, 503379290 },
		};
		const char *MainChainTestNetDNSSeeds[] = {
			"node-testnet-002.elastos.org",
			"node-testnet-003.elastos.org",
			"node-testnet-004.elastos.org",
			NULL
		};
		const ELAChainParams MainChainTestNetParams = {
			.Raw = {
				MainChainTestNetDNSSeeds,
				21866,        // standardPort
				2018001,    // magicNumber
				0,            // services
				nullptr,
				MainChainTestNetCheckpoints,
				sizeof(MainChainTestNetCheckpoints) / sizeof(*MainChainTestNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "TestNet"
		};


		// IdChain Reg Net
		const BRCheckPoint IdChainRegNetCheckpoints[] = {
			{ 0,      uint256("56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3"), 1513936800, 486801407 },
//			{ 2016,   uint256("f1c1d10a1987b3e6c0b1abe2f30116fac878e98f948afb26891123949edf9fe8"), 1531068028, 506089933 },
//			{ 4032,   uint256("abdd6bde1af7c09926fda29cbfe93aa87d9f5ff84e5ffde38a407963aa202e60"), 1534530162, 520199430 },
//			{ 6048,   uint256("0c015b956574c79007e67c517cdf46e44894c028b16055a28c0ca0e06fc0d2ea"), 1534596583, 503869751 }
		};
		const char *IdChainRegNetDNSSeeds[] = {
			"did-regtest-001.elastos.org",
			"did-regtest-002.elastos.org",
			"did-regtest-003.elastos.org",
			"did-regtest-004.elastos.org",
			"did-regtest-005.elastos.org",
			NULL
		};
		const ELAChainParams IdChainRegNetParams = {
			.Raw = {
				IdChainRegNetDNSSeeds,
				22608,        // standardPort
				201806271,    // magicNumber
				0,            // services
				nullptr,
				IdChainRegNetCheckpoints,
				sizeof(IdChainRegNetCheckpoints) / sizeof(*IdChainRegNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "RegNet"
		};

		// IdChain Test Net
		const BRCheckPoint IdChainTestNetCheckpoints[] = {
			{ 0,      uint256("56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3"), 1513936800, 486801407 },
			{ 2016,   uint256("9b3069a05478988d4c9d2d4b941af048680dbe92353bc8f0cf282766aa935edb"), 1532276131, 505977014 },
			{ 4032,   uint256("6e8f5e21ddd736bb62dcd3ae445444702dc0b0ee560b4161877d0c8f0d9ad448"), 1532628770, 510283913 },
			{ 6048,   uint256("8cef466bdc7c9ed79fdd6b36babb5b6670c916c07d87b473327460e57d21799b"), 1533004749, 520187129 },
			{ 8064,   uint256("46a9dd847b62f278a26a93e3f7a4cc135b30754e07f2754d6ac0baf6870c8060"), 1533346700, 520376318 },
			{ 10080,  uint256("09cdb3d720a7095c7023241361ca8a317c75273fbdd0e5dc268667683e360780"), 1533694521, 520890958 },
			{ 12096,  uint256("2790468186eb341dfe06fc6b8a66ad48d2bfb597d0f612e2a917d703c951aee1"), 1534057013, 521637984 },
			{ 14112,  uint256("f5582a7bab4ec851172dd96b9c18d8a046081de65ed05f316e4dc14a5a8fe176"), 1534506459, 536912421 },
			{ 16128,  uint256("8b78d2fee6751c82d419e5eb98bbf83a1514a046ae9ffd1462aceaeed3f189ec"), 1534771533, 536935584 },
		};
		const char *IdChainTestNetDNSSeeds[] = {
			"did-testnet-001.elastos.org",
			"did-testnet-002.elastos.org",
			"did-testnet-003.elastos.org",
			"did-testnet-004.elastos.org",
			"did-testnet-005.elastos.org",
			NULL
		};
		const ELAChainParams IdChainTestNetParams = {
			.Raw = {
				IdChainTestNetDNSSeeds,
				21608,		// standardPort
				20180011,	// magicNumber
				0,			// services
				nullptr,
				IdChainTestNetCheckpoints,
				sizeof(IdChainTestNetCheckpoints) / sizeof(*IdChainTestNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "TestNet"
		};

		ChainParams::ChainParams(const ChainParams &chainParams) {
			_chainParams = boost::shared_ptr<ELAChainParams>(
					new ELAChainParams(*(ELAChainParams *) chainParams.getRaw()));
		}

		ChainParams::ChainParams(const CoinConfig &coinConfig) {
			_chainParams = boost::shared_ptr<ELAChainParams>(new ELAChainParams());
			tryInit(coinConfig);
		}

		std::string ChainParams::toString() const {
			//todo complete me
			return "";
		}

		BRChainParams *ChainParams::getRaw() const {
			return (BRChainParams *) _chainParams.get();
		}

		uint32_t ChainParams::getMagicNumber() const {
			return _chainParams->Raw.magicNumber;
		}

		void ChainParams::tryInit(const CoinConfig &coinConfig) {
			if (coinConfig.Type == Mainchain || coinConfig.Type == Normal) {
				if (coinConfig.NetType == "TestNet") {
					*_chainParams = MainChainTestNetParams;
				} else if (coinConfig.NetType == "RegNet") {
					*_chainParams = MainChainRegNetParams;
				} else if (coinConfig.NetType == "MainNet") {
					// TODO support MainNet later
					throw std::logic_error("Unsupport NetType=MainNet in CoinConfig");
				} else {
					Log::getLogger()->error("Invalid NetType = {} in CoinConfig", coinConfig.NetType);
					throw std::logic_error("Invalid NetType in CoinConfig");
				}
			} else if (coinConfig.Type == Idchain) {
				if (coinConfig.NetType == "TestNet") {
					*_chainParams = IdChainTestNetParams;
				} else if (coinConfig.NetType == "RegNet") {
					*_chainParams = IdChainRegNetParams;
				} else if (coinConfig.NetType == "MainNet") {
					// TODO support MainNet later
					throw std::logic_error("Unsupport NetType=MainNet in CoinConfig");
				} else {
					Log::getLogger()->error("Invalid NetType = {} in CoinConfig", coinConfig.NetType);
					throw std::logic_error("Invalid NetType in CoinConfig");
				}
			} else if (coinConfig.Type == Sidechain) {
				// TODO support anthor side chain later
				throw std::logic_error("Unsupport Type=Sidechain in CoinConfig");
			} else {
				Log::getLogger()->error("Invalid Type = {} in CoinConfig", coinConfig.Type);
				throw std::logic_error("Invalid Type in CoinConfig");
			}
		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_chainParams = boost::shared_ptr<ELAChainParams>(new ELAChainParams(*(ELAChainParams *) params.getRaw()));
			return *this;
		}

		uint32_t ChainParams::getTargetTimeSpan() const {
			return _chainParams->TargetTimeSpan;
		}

		uint32_t ChainParams::getTargetTimePerBlock() const {
			return _chainParams->TargetTimePerBlock;
		}

	}
}
