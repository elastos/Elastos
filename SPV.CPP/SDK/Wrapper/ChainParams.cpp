// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ChainParams.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

#include <Core/BRChainParams.h>
#include <Core/BRMerkleBlock.h>
#include <Core/BRChainParams.h>

namespace Elastos {
	namespace ElaWallet {
		// MainNet node
		const BRCheckPoint MainChainMainNetCheckpoints[] = {
			{ 0,       uint256("05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d"), 1513936800, 486801407 },
			{ 6048,    uint256("7f7318b40759af62ef67acd02ff864322f0f92bf8daad92eb3d2b696ab25c5de"), 1514477782, 492335039 },
			{ 14112,   uint256("5e2820cc0f6fa4e73fb9cba4c9545d9cca83c8b2ecc0bbe93dd2b02739ac1ccf"), 1515717109, 503425843 },
			{ 22176,   uint256("a32a6a82af8eb8e25543ed09671e52f7a54b4adf2cc703134bc122010f1f73af"), 1516694131, 503433043 },
			{ 30240,   uint256("b07a36273b890f236a4bc1f76eacb4887b98dc91ae6aac029f103ee005142c6b"), 1517660585, 503424760 },
			{ 38304,   uint256("100d5be4ae184053c2711463fada2c656f4cb688ef9e9c8bbc0b55437a191505"), 1518639973, 503433549 },
			{ 46368,   uint256("394c20217b9a5cb6b313ea505b5addce706530efebd807d303a13cb081b80288"), 1519613691, 503441261 },
			{ 54432,   uint256("a8ac1d106997b77d1ccc6d599f9c1fffc527e786611a377d8eba7e6ce68b41f0"), 1520541067, 503426764 },
			{ 62496,   uint256("8c6d9086e3944cc56bef87eaafb6d2cca23e7e7b8ae637bc390e6ae61bcd0fa8"), 1521478059, 493673302 },
			{ 70560,   uint256("bba9a93913f584e306e6e9adc86aba8ea647e7a198331c6052a13ef8d6be34f9"), 1522451759, 493739826 },
			{ 78624,   uint256("6d656e3779ed512d2dc2dcf2c409fe2d25fa2f31317dbaaced0b65e1e890a8c8"), 1523423918, 493896532 },
			{ 86688,   uint256("148e13309f89f9dbdc67c4bbfdc597d24906cbd12d82d3c164764ab9aec9c7db"), 1524397043, 494225470 },
			{ 94752,   uint256("e6de6c24d85c365a9e87f1eccc843c8fec40485c3567b088b3b3ebd9419fdab4"), 1525362338, 493998710 },
			{ 102816,  uint256("21fbf33a8ac97d4da0660fc8cf1d4b42ced6cd39cb6ff9f81aa4ed898b4adfb2"), 1526317611, 492534043 },
			{ 110880,  uint256("2b73b2a9c38ef81a65df91529082e44c32a64b4ff94e9a2096520553d62ba75f"), 1527287623, 492595836 },
			{ 118944,  uint256("1f323e8d7f24166c31c9ba1db896c4833ea4d556a436cc41337bbeb56690a05d"), 1528260240, 492752898 },
			{ 127008,  uint256("e6ac013fe5fb1a19df564e7246f30416c3396cb182bc6c4d3a550bf880ee2877"), 1529238500, 492897549 },
			{ 135072,  uint256("0b82ea6c60f3b79c36aca61e46365a269a0d0d9bea95c09cf26aef097770d0ba"), 1530242630, 503350076 },
			{ 143136,  uint256("c7890d31f73b85c03593f7c6dace0a7604e6709519cff75cf945b7c83b0f7aee"), 1531153562, 490135368 },
			{ 151200,  uint256("018c0b437de884310d33d43fb4980e6900ec11d359d9cb4182a43c1a662e9879"), 1532126538, 490309082 },
			{ 159264,  uint256("767b2e226cf0e6f2805fbeb6d74122ec6f7ad7b7ab68556417709bafc532a8ba"), 1533090476, 490037606 },
			{ 167328,  uint256("6a5f9d7a97378ee9ea5032f54e20943efd83e88a4eb428eccf2fdb4ff493de00"), 1534068603, 490143773 },
			{ 175392,  uint256("c0e0d7a787dfb3f963350586d43ae01756952bfdab180b3c3cf66370c3e537bb"), 1535033091, 489971771 },
			{ 183456,  uint256("03665026718f02ccb9c764aafe20ab5c1f4963cf7eac6931a300d40b28343f59"), 1535433865, 457157190 },
			{ 191520,  uint256("dece794f70dd3f137dcb4af48b9edbd27cb9b7e7804a6318ea191f0d83d2128b"), 1535691721, 410486883 },
			{ 199584,  uint256("3aa6479764a47b48c80c74f42f04c745585e52ea33ab78ec4e374df45868cae1"), 1536624113, 407369294 },
			{ 207648,  uint256("348cab05f293ce1b4848c30a0f89b8b6932b2dabf4bf96a6a0c54aedf0862769"), 1537598310, 407742976 },
			{ 215712,  uint256("7f7c3410c951c2b6b1d040d83495c832eac37a14a74ac7766b931ccb08582c0b"), 1538570196, 407771639 },
		};
		const char *MainChainMainNetDNSSeeds[] = {
			"node-mainnet-002.elastos.org",
			"node-mainnet-006.elastos.org",
			"node-mainnet-014.elastos.org",
			"node-mainnet-016.elastos.org",
			"node-mainnet-021.elastos.org",
			"node-mainnet-003.elastos.org",
			"node-mainnet-007.elastos.org",
			"node-mainnet-015.elastos.org",
			"node-mainnet-017.elastos.org",
			"node-mainnet-022.elastos.org",
			"node-mainnet-004.elastos.org",
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
			{ 10080,  uint256("00ac65927dcf741e87b0560b9676a35abc5d1ab74afc6144f4159315f15561d2"), 1535069960, 539089007 },
			{ 12096,  uint256("a942ab0b540e92e96bbced320ec9162354c971e5812e904e73010ff94eee2b16"), 1535300054, 538192810 },
			{ 14112,  uint256("d342ee0817e30a57a515ccfaaf836e7314aac1235d0659d5e6ffc46671ae4979"), 1535367638, 527395580 },
			{ 16128,  uint256("2106fe8000c01eecba9d5396f26c401dc5f4a1a90dd9f3f728e3a770afbc842b"), 1535435331, 520256486 },
			{ 18144,  uint256("fc4e7ba460e38964faeb33873418c9fcd3ae2648a543cc1e8f5c47806b593ac8"), 1535503457, 504235070 },
			{ 20160,  uint256("2e42266a8b6abff2f1122885a0f0f00c1f1adaeb4ed0572aee1bbc681f41f5f9"), 1535572617, 492035370 },
			{ 22176,  uint256("9ef0a2d21af1f1ebba5b092ce0f672b221ea7d1f2e765790c0741aaff7667da2"), 1535640920, 486964831 },
			{ 24192,  uint256("d32c06fa82a121ef6e94f8292ea7d27db5125195d5f9a6bac9c0f224f7a06c2d"), 1535773072, 476801991 },
			{ 26208,  uint256("a9eb520903f5f1ec1a11a3f5040791d59ae63b8e16846a994bbd303ba43b50db"), 1536008684, 475415774 },
			{ 28224,  uint256("682a75e280934562bd92e219581dc61fc347b1bc6e5381371397c5202db81e53"), 1536252045, 475473438 },
			{ 30240,  uint256("470d858db326df0e738248ac5f5811f1b31a0c1ca1892f17a93673efefde3a45"), 1536500538, 476055185 },
			{ 32256,  uint256("6695557d6e012b9d122cf96ed20aa23d8a37518f30e7a8d58f740606ddb03499"), 1536743006, 476264083 },
			{ 34272,  uint256("f78fc41c0d2ba1c0c133ebcb351ecfedc42324a31201b6c207d0165016430a74"), 1536991969, 476548897 },
			{ 36288,  uint256("d22c1443f5b3eff60a95303766eb4397bc9c027e3ab87b5aedbaa967c7d8d130"), 1537238408, 476538986 },
			{ 38304,  uint256("9592ad50c220ea2900d0c3659df470b4e8e459ac6ee2d43ddc9b6ddd15166645"), 1537492347, 477909524 },
			{ 40320,  uint256("7cd2999991ad8d9145035a1506a97d29641cde4d44f78533715cf469e19755ee"), 1537730755, 477389634 },
			{ 42336,  uint256("548c15f8daede3d85aed4a6ad8bf013c6cb2b3c7ea7aba0134ffb2adc450850a"), 1537976718, 478079428 },
			{ 44352,  uint256("b3af4dd06d5b5f3e033d36a6096665fb82ddd52453191186ace7bd25151d442d"), 1538227932, 486572564 },
			{ 46368,  uint256("f9e8780e4df7eeecf80f8cb8fcdd2f08f2357da9f50557235373c20123e9fa45"), 1538494105, 486580306 },
			{ 48384,  uint256("5b968beba5751a0c4e6deda9805fb3b33e7dfd568d51cb460f22dbde8c730b6d"), 1538805631, 486623668 },
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
			{ 8064,    uint256("fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158"), 1522645593, 494006531 },
			{ 14112,   uint256("20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e"), 1523380720, 494547269 },
			{ 20160,   uint256("0fd0ecfabdd3f3405b9808e4f67749232d23404fd1c90a4e2c755ead8651e759"), 1524792298, 503351573 },
			{ 26208,   uint256("d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c"), 1525542345, 503358411 },
			{ 32256,   uint256("0c03acf34f4cd5df02062e658f91dfe884788831b940b18a136f7d33adae00b0"), 1526270878, 503361032 },
			{ 38304,   uint256("ea313df1089b8b001c40565aa5e90d17da51885a29459b966d0019ac7dd90002"), 1526997348, 503361573 },
			{ 44352,   uint256("77fcd9c9e3a875b46d0029805cfe3bdb06f7beef49ea40eaadb20368cd0bea2e"), 1527704178, 503349609 },
			{ 50400,   uint256("176751ba71545428b0a1740abcd0f1846b97b1455fbbcb27a5c778a575fa8b71"), 1528433088, 503351979 },
			{ 56448,   uint256("d6faaa1526829d58cbd2c7afbabdb7bffefcc5494d808a79ad79d9d6d6fb17af"), 1529153218, 494866089 },
			{ 62496,   uint256("8cb6da886b6258e8a6daefce9d5566575bd2ba95b4bd684105954ab9e69be042"), 1529864598, 493529298 },
			{ 68544,   uint256("c7595156595f1b925f5dd3aeeaabc9c3b3605e5d9ba934c396a31126398a10fe"), 1530583548, 492729858 },
			{ 74592,   uint256("47ea897ceb1ad471b5a7277e348ad43456b08d149db39b419174cb295be4fed8"), 1531345056, 503351198 },
			{ 80640,   uint256("db4f79022b23ffada435eb3d0681deb11a1591428044cf4625188f1c6277f9b1"), 1532146578, 503381165 },
			{ 86688,   uint256("a25b7207c98cbbede93db67ca41a0ac308d198c556eea955ff63f52b0b70131a"), 1532870279, 503379835 },
			{ 92736,   uint256("c5134b4d7b275a6109cf2f1e7c4f31ad96a4c6c19d2bfd98e7b655d9d49d5723"), 1533598732, 503377199 },
			{ 98784,   uint256("552732e51a44b3cf99992447b9b3f79226fa90f0f9f1b47e15836abc2c334322"), 1534328247, 503380456 },
			{ 104832,  uint256("ec5c35a62347241f33949ac5b072af32a3fe0f3e9cc1ccbabe7598629c94940a"), 1535057104, 503376374 },
			{ 110880,  uint256("743823d8e5d1e74aa7aa8de9a20ca3b42dad4c1115b1631d6071d66d68f6a4f9"), 1535780560, 503376620 },
			{ 116928,  uint256("7151ea9c0d36479c9f6e92776b88f9f925bcbcddcff44deb7463205dee8f06d3"), 1536517600, 503382324 },
			{ 122976,  uint256("e8e5c0ca094d156fb5ce864c3a3ec68925de51a187060920b1252936b21356cd"), 1537239070, 503382468 },
			{ 129024,  uint256("1e08db102db105f1d58703da1153322001533a3d4fe31a794dfb118600f1a615"), 1537969555, 503380928 },
			{ 131040,  uint256("2f8549b8a30527a5878a3c105b4c6d8bb4faf4619c6917d704001424f6625b5c"), 1538216125, 503384495 },
			{ 133056,  uint256("2a8016306ba32a9b4451a5024a463f1e59240920963cddaf011616e3be020c83"), 1538451280, 503379347 },
			{ 135072,  uint256("681fee1c6ec452b1c38b5aadb26b9b60371b8104136bcef628c5b828643aebf9"), 1538701705, 503383651 },
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
			{ 24192,  uint256("e9198d6fdd89d292f7e967266c22c59e245d015d93c0428626211965b8349d05"), 1535828463, 537035498 },
			{ 26208,  uint256("8636093320982e3707224cb19252c2f2de124bd1c480c3e4ba35f6d564e2d713"), 1536086668, 537085786 },
			{ 28224,  uint256("b98d6c3e7f424d44bf4f21f9093be7b2d29f2134137949769718c33ae22bd205"), 1536361613, 537169686 },
			{ 30240,  uint256("8c43de5c79cd48da6754b2098fcb230d525b7b0f1e44fc7c87b2b38064269991"), 1536626736, 537255791 },
			{ 32256,  uint256("a8000672bff80dcf7117062073df6f3cb511be0e49c2862f91432a00eeef053e"), 1536886078, 537301386 },
			{ 34272,  uint256("640298c39b33c1298d87e482115053177ae60b721e23801db0061bb4e24d393b"), 1537146343, 537429158 },
			{ 36288,  uint256("5d2a3ae65b42c7233e2fcbfc2a4f86eb24a693ab7e3b47c38830e57c956ff49c"), 1537593491, 539041984 },
			{ 38304,  uint256("3763dde6fa684f31615b4237a17116777bf807a19701c3d5d02604e9bdcea1a6"), 1538035204, 545259519 },
			{ 40320,  uint256("25ad2ec3bd7531301d367b4f7b1357e567665213c776744775fed7f80b9b0349"), 1538398560, 545259519 },
			{ 42336,  uint256("18086d3158974bbc1a331b194e0580504ed76058d88da84fc446b0020b63c329"), 1538663566, 545259519 },
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
			{ 8064,    uint256("fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158"), 1522645593, 494006531 },
			{ 14112,   uint256("20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e"), 1523380720, 494547269 },
			{ 20160,   uint256("40eec8f7f3c7f4d9ea4c7dbe5eb85456c72ee79f7e5940a5e7d5f4c80bc1e7a7"), 1524736650, 503356298 },
			{ 26208,   uint256("3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a"), 1525472220, 503357792 },
			{ 32256,   uint256("9ba5fec5acaf23302110e333b022a074de88b4e36b9264e98297cbaec09f591f"), 1526225370, 503370323 },
			{ 38304,   uint256("9307bcfe0fe867a824e3a9139d626a607aa1c09396c027df8fa243b4110f499d"), 1527042222, 503415621 },
			{ 44352,   uint256("cf8516eb910306862bcc206519b0e6eb187dc20ce384ca525dd1c0ec751c92f2"), 1528221733, 503374579 },
			{ 50400,   uint256("8e34fdc18ef27996022cc36d85f856690990faa077d92b78989c04f113205cd0"), 1528926400, 493749763 },
			{ 56448,   uint256("74d8765907eb63ba286d2d8373de916c25c5daf5250c25d652597c2422e0bc9c"), 1530223515, 503358059 },
			{ 62496,   uint256("398e154da8acf7565af3a8cae969b2aa8cdf0abb5817bcb0e2b64c8155557416"), 1531091111, 503350455 },
			{ 68544,   uint256("2b24892cda519c15de01dd18298853c5dcd5f278c8a92ccc7e1060ca2f206c71"), 1531796891, 503349562 },
			{ 74592,   uint256("f7afa994b23a5628f109e1de8563f4ae5963a814b1c972425d921d581bcec7a0"), 1532636667, 503374250 },
			{ 80640,   uint256("bcadac789eec2262f099cc0f57ef53937520ddbc298cb2341d00bce1cbabd7ec"), 1533423380, 503380311 },
			{ 86688,   uint256("7de2bfb514c7c466aa4903d47d9e5ea89d8cd13e60fc134b1ac7a6337499fcab"), 1534151990, 503377735 },
			{ 92736,   uint256("d19883bc202ef4802f73bd18ce2456c8c933e9a2793bb566efc21cf3a48fe52c"), 1536149363, 503375367 },
			{ 98784,   uint256("a8efe800d2a18bf7dca867e307561d11e33abe20fc8a6c20e82f05efcaa3f6cd"), 1536880958, 503376323 },
			{ 104832,  uint256("bfe6f03363b445f7c6d6acbb8b8ab71641178e0d1f612fc459a622b19c741377"), 1537577738, 503359020 },
			{ 110880,  uint256("3f2ea95ca953b97a638a24627a41826973a485fc7c0e661ad7c6cb5870910e5f"), 1538309387, 503358893 },
			{ 112896,  uint256("8ac3137dfb231d3f89878f28342791e457efb029c6710ead8ed0d759ae6dc8e2"), 1538558993, 503359813 },
			{ 114912,  uint256("4d2197b83c20edac4125a9414b56d0a9a0f0252858c7eb95b50805626f1c7256"), 1538794988, 503359557 },
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
