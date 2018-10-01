// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRChainParams.h>
#include <BRMerkleBlock.h>
#include <Core/BRChainParams.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace ElaWallet {
		// MainNet node
		const BRCheckPoint MainChainMainNetCheckpoints[] = {
			{ 0,       uint256("05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d"), 1513936800, 486801407 },
			{ 2016,    uint256("333d9a0e874cf1b165a998c061c2f8be8e03ce31712046d001823d528e1fec84"), 1513999567, 503353328 },
			{ 14112,   uint256("5e2820cc0f6fa4e73fb9cba4c9545d9cca83c8b2ecc0bbe93dd2b02739ac1ccf"), 1515717109, 503425843 },
			{ 26208,   uint256("6b8e6679cafaf0ea3c93f0691677a308c575b0058914a32fa2575dd44664c56f"), 1517182721, 503435754 },
			{ 38304,   uint256("100d5be4ae184053c2711463fada2c656f4cb688ef9e9c8bbc0b55437a191505"), 1518639973, 503433549 },
			{ 50400,   uint256("52b31a0007ccd6d4c3b90237dd86b4396dfe75440bd5fb5cb658a8e6c9777d65"), 1520089211, 503424362 },
			{ 62496,   uint256("8c6d9086e3944cc56bef87eaafb6d2cca23e7e7b8ae637bc390e6ae61bcd0fa8"), 1521478059, 493673302 },
			{ 74592,   uint256("c4e5ccd88ae8ef75df9905aa57078cd97fb13ff7ad10f845b54774db7493d920"), 1522933778, 493681026 },
			{ 86688,   uint256("148e13309f89f9dbdc67c4bbfdc597d24906cbd12d82d3c164764ab9aec9c7db"), 1524397043, 494225470 },
			{ 98784,   uint256("78b40cf9dd20d7f216bb415cbf29419eff72a00e7fd1df892c454e7259131f2a"), 1525830606, 492702149 },
			{ 110880,  uint256("2b73b2a9c38ef81a65df91529082e44c32a64b4ff94e9a2096520553d62ba75f"), 1527287623, 492595836 },
			{ 122976,  uint256("4755a56fa7a9bfa12125d573bf4c47af1caaf3b749de890bf1881d9de04778e2"), 1528725950, 491428429 },
			{ 135072,  uint256("0b82ea6c60f3b79c36aca61e46365a269a0d0d9bea95c09cf26aef097770d0ba"), 1530242630, 503350076 },
			{ 147168,  uint256("2036edb7c8eee311231efca372681afc37026751befcc3381bbaabe9c26f67dd"), 1531637428, 490126892 },
			{ 159264,  uint256("767b2e226cf0e6f2805fbeb6d74122ec6f7ad7b7ab68556417709bafc532a8ba"), 1533090476, 490037606 },
			{ 173376,  uint256("ce52eca3db14476ae7b01faa0258bc21b4b93fe0b0364342785cf479f213f851"), 1534787121, 489990925 },
		};
		const char *MainChainMainNetDNSSeeds[] = {
			"node-mainnet-003.elastos.org",
			"node-mainnet-008.elastos.org",
			"node-mainnet-015.elastos.org",
			"node-mainnet-018.elastos.org",
			"node-mainnet-023.elastos.org",
			NULL
		};
		const ELAChainParams MainChainMainNetParams = {
			.Raw = {
				MainChainMainNetDNSSeeds,
				20866,        // standardPort
				2017001,    // magicNumber
				0,            // services
				nullptr,
				MainChainMainNetCheckpoints,
				sizeof(MainChainMainNetCheckpoints) / sizeof(*MainChainMainNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "MainNet"
		};

		// MainNet did
		const BRCheckPoint IdChainMainNetCheckpoints[] = {
			{ 0,      uint256("56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3"), 1530360000, 486801407 },
			{ 2016,   uint256("df060a6475ace78657e1ec945cebb37f2ffb6367b185f9ba5edfa987f16b84f4"), 1531426582, 520126740 },
			{ 4032,   uint256("b731b13902f3f2d275589828504b4bc2a8d67d78dbb08e323925282d27fac755"), 1531907920, 520341697 },
			{ 6048,   uint256("4e6a53d3e3e61d4883a10d62fb42afb62ad6f10807bcc3791db284f43b063671"), 1532395676, 522028475 },
			{ 8064,   uint256("83bcb00de21a4390959a3f126c44a2547ada830e1411fde1cdef4fb61eb7598d"), 1534765774, 537293716 },

		};
		const char *IdChainMainNetDNSSeeds[] = {
			"did-mainnet-001.elastos.org",
			"did-mainnet-002.elastos.org",
			"did-mainnet-003.elastos.org",
			"did-mainnet-004.elastos.org",
			"did-mainnet-005.elastos.org",
			NULL
		};
		const ELAChainParams IdChainMainNetParams = {
			.Raw = {
				IdChainMainNetDNSSeeds,
				20608,        // standardPort
				2017002,    // magicNumber
				0,            // services
				nullptr,
				IdChainMainNetCheckpoints,
				sizeof(IdChainMainNetCheckpoints) / sizeof(*IdChainMainNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "MainNet"
		};

		// TestNet node
		const BRCheckPoint MainChainTestNetCheckpoints[] = {
			{ 0,       uint256("6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3"), 1513936800, 486801407 },
			{ 2016,    uint256("99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767"), 1517940844, 503906048 },
			{ 10080,   uint256("4dbf87bdf287319958c480fbc9249cbc93847a54f05ccb13f8283de1cf365158"), 1522878764, 493221476 },
			{ 18144,   uint256("a299a6fde95bde058996253fb090658ca7b19b6366c48c053ac43302728d380d"), 1523866303, 494684787 },
			{ 26208,   uint256("d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c"), 1525542345, 503358411 },
			{ 34272,   uint256("d0a25ed7d01706dc11e66737e13e1eef9f2cfbab369cf6e1f507af6809d75d3f"), 1526516129, 503360832 },
			{ 42336,   uint256("5889b5b496311147aecf78caf9a6e6715175e8d8465c7efc1476d78e722e421e"), 1527464757, 503351150 },
			{ 50400,   uint256("176751ba71545428b0a1740abcd0f1846b97b1455fbbcb27a5c778a575fa8b71"), 1528433088, 503351979 },
			{ 58464,   uint256("f4531446ea034ef6f2c67ae58005cc13a6729447038d01532a8c0412eaac3cf7"), 1529390060, 494211015 },
			{ 66528,   uint256("c070d56151de392bc0e13201f5cb3e1bd0cb606aae83082fc1dd26c8368bdd0c"), 1530343250, 492581027 },
			{ 74592,   uint256("47ea897ceb1ad471b5a7277e348ad43456b08d149db39b419174cb295be4fed8"), 1531345056, 503351198 },
			{ 82656,   uint256("c9456b3feeae9c44bc7091487786505d50cddb1fc5b6aecced0924d55a8950d4"), 1532387827, 503383240 },
			{ 90720,   uint256("801a188478b77b8f0b07320ef6ff3ccc7d6931804fa328f57e91176d65ee9326"), 1533356482, 503378206 },
			{ 98784,   uint256("552732e51a44b3cf99992447b9b3f79226fa90f0f9f1b47e15836abc2c334322"), 1534328247, 503380456 },
			{ 102816,  uint256("b5e07af958149f5332336be914dbeba25da11dbd81cc0be9421352cb6fb02379"), 1534812379, 503379290 },
			{ 104832,  uint256("ec5c35a62347241f33949ac5b072af32a3fe0f3e9cc1ccbabe7598629c94940a"), 1535057104, 503376374 },
			{ 106848,  uint256("24db6b68c5978094a1b51cd4b611b295f4157ccec24e6eb16a8086c94df5fde5"), 1535300839, 503379405 },
			{ 108864,  uint256("d9e1f052277455988c252bf7e32282014244b6b810eebb314a8bb917fd9d2bb7"), 1535535280, 503377204 },
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

		// TestNet did
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
			{ 18144,  uint256("ba98bdbeb358fc2ac565dd3a6f706cef7dfdc60063f1f3517311744c3a908de4"), 1535034489, 536952176 },
			{ 20160,  uint256("b4d0dcec130dccd6cfa61880cdfe30259e90c69d1f83056f0495910cca05246b"), 1535300757, 536980894 },
			{ 22176,  uint256("9738541d86b7843be83926be997164615a3102e33edece26da351fdae70d2d6a"), 1535557289, 536997059 },
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
				21608,        // standardPort
				20180011,    // magicNumber
				0,            // services
				nullptr,
				IdChainTestNetCheckpoints,
				sizeof(IdChainTestNetCheckpoints) / sizeof(*IdChainTestNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "TestNet"
		};


		// RegTest node
		const BRCheckPoint MainChainRegNetCheckpoints[] = {
			{ 0,       uint256("6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3"), 1513936800, 486801407 },
			{ 2016,    uint256("99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767"), 1517940844, 503906048 },
			{ 10080,   uint256("4dbf87bdf287319958c480fbc9249cbc93847a54f05ccb13f8283de1cf365158"), 1522878764, 493221476 },
			{ 18144,   uint256("a299a6fde95bde058996253fb090658ca7b19b6366c48c053ac43302728d380d"), 1523866303, 494684787 },
			{ 26208,   uint256("3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a"), 1525472220, 503357792 },
			{ 34272,   uint256("8dfe3ec6125c06c40bcd9a72963b6e60dc214b7a61ad2e805b138f60ac7e92de"), 1526448032, 503363437 },
			{ 42336,   uint256("8b638a328e1f1930fba8cb5ee0703a2c16aa362a52dc8b8729a33fb487706441"), 1527512109, 503361968 },
			{ 50400,   uint256("8e34fdc18ef27996022cc36d85f856690990faa077d92b78989c04f113205cd0"), 1528926400, 493749763 },
			{ 58464,   uint256("5fadb0eb085f90998857f18b0524db988d323650b50ce8ad2289320c1a9b9a07"), 1530458426, 503351139 },
			{ 66528,   uint256("2fec85c9bc0a9f9b8f6ac106bfae0c957485d456eec86f21eafddff1ace2fffe"), 1531553227, 503351485 },
			{ 74592,   uint256("f7afa994b23a5628f109e1de8563f4ae5963a814b1c972425d921d581bcec7a0"), 1532636667, 503374250 },
			{ 82656,   uint256("3389fef7151029d9dca13cf109e668c0d6b96af8d0f56422c9c776b58313af62"), 1533667029, 503380217 },
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

		// RegTest did
		const BRCheckPoint IdChainRegNetCheckpoints[] = {
			{ 0,      uint256("56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3"), 1513936800, 486801407 },
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
				201809031,    // magicNumber
				0,            // services
				nullptr,
				IdChainRegNetCheckpoints,
				sizeof(IdChainRegNetCheckpoints) / sizeof(*IdChainRegNetCheckpoints)
			},
			.TargetTimeSpan = 86400,
			.TargetTimePerBlock = 120,
			.NetType = "RegNet"
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
					*_chainParams = MainChainMainNetParams;
				} else {
					ParamChecker::checkCondition(true, Error::WrongNetType,
												 "Invalid net type " + coinConfig.NetType + " in coin config");
				}
			} else if (coinConfig.Type == Idchain) {
				if (coinConfig.NetType == "TestNet") {
					*_chainParams = IdChainTestNetParams;
				} else if (coinConfig.NetType == "RegNet") {
					*_chainParams = IdChainRegNetParams;
				} else if (coinConfig.NetType == "MainNet") {
					*_chainParams = IdChainMainNetParams;
				} else {
					ParamChecker::checkCondition(true, Error::WrongNetType,
												 "Invalid net type " + coinConfig.NetType + " in coin config");
				}
			} else {
				ParamChecker::checkCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
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
