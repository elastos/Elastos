// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ChainParams.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

#include <Core/BRMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		ChainParams::ChainParams(const ChainParams &params) {
			_dnsSeeds.insert(_dnsSeeds.begin(), params._dnsSeeds.begin(), params._dnsSeeds.end());
			_checkpoints.insert(_checkpoints.begin(), params._checkpoints.begin(), params._checkpoints.end());

			_standardPort = params._standardPort;
			_magicNumber = params._magicNumber;
			_services = params._services;
			_targetTimeSpan = params._targetTimeSpan;
			_targetTimePerBlock = params._targetTimePerBlock;
		}

		ChainParams::ChainParams(const CoinConfig &config) {
			if ("MainNet" == config.NetType) {
				MainNetParamsInit(config.Type);
			} else if ("TestNet" == config.NetType) {
				TestNetParamsInit(config.Type);
			} else if ("RegNet" == config.NetType || "RegTest" == config.NetType) {
				RegNetParamsInit(config.Type);
			} else {
				ParamChecker::checkCondition(true, Error::WrongNetType,
											 "Invalid net type " + config.NetType + " in coin config");
			}

		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_dnsSeeds.clear();
			_dnsSeeds.insert(_dnsSeeds.begin(), params._dnsSeeds.begin(), params._dnsSeeds.end());
			_checkpoints.clear();
			_checkpoints.insert(_checkpoints.begin(), params._checkpoints.begin(), params._checkpoints.end());

			_standardPort = params._standardPort;
			_magicNumber = params._magicNumber;
			_services = params._services;
			_targetTimeSpan = params._targetTimeSpan;
			_targetTimePerBlock = params._targetTimePerBlock;

			return *this;
		}

		const std::vector<std::string> &ChainParams::GetDNSSeeds() const {
			return _dnsSeeds;
		}

		const CheckPoint &ChainParams::GetLastCheckpoint() const {
			return _checkpoints.back();
		}

		const CheckPoint &ChainParams::GetFirstCheckpoint() const {
			return _checkpoints.front();
		}

		const std::vector<CheckPoint> &ChainParams::GetCheckpoints() const {
			return _checkpoints;
		}

		const uint32_t &ChainParams::GetMagicNumber() const {
			return _magicNumber;
		}

		const uint16_t &ChainParams::GetStandardPort() const {
			return _standardPort;
		}

		const uint64_t &ChainParams::GetServices() const {
			return _services;
		}

		const uint32_t &ChainParams::GetTargetTimeSpan() const {
			return _targetTimeSpan;
		}

		const uint32_t &ChainParams::GetTargetTimePerBlock() const {
			return _targetTimePerBlock;
		}

		// main net
		void ChainParams::MainNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-mainnet-002.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-006.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-014.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-016.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-021.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-003.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-007.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-015.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-017.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-022.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-004.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-023.elastos.org");
			_checkpoints.emplace_back(0,      "05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "333d9a0e874cf1b165a998c061c2f8be8e03ce31712046d001823d528e1fec84", 1513999567, 503353328);
			_checkpoints.emplace_back(12096,  "6347d62e227dbf74c8a4c478fa4f8d351f24a3db3f5a01aad32ea262b6e6f6aa", 1515218030, 492349017);
			_checkpoints.emplace_back(22176,  "a32a6a82af8eb8e25543ed09671e52f7a54b4adf2cc703134bc122010f1f73af", 1516694131, 503433043);
			_checkpoints.emplace_back(32256,  "3c6f75a2a6f9d37918036b912cb341ccd687d04ff354df4afe4aa83a3a97f0eb", 1517916962, 503438901);
			_checkpoints.emplace_back(42336,  "5512e09b1cc98893969a3801f5f02de7b5146748b99142162803d3fc65b094a1", 1519125604, 503440501);
			_checkpoints.emplace_back(52416,  "2c3c8ebd558387a59880046b1f7dea9f55ec2d2525811806ab673411de078896", 1520340390, 503442853);
			_checkpoints.emplace_back(62496,  "8c6d9086e3944cc56bef87eaafb6d2cca23e7e7b8ae637bc390e6ae61bcd0fa8", 1521478059, 493673302);
			_checkpoints.emplace_back(72576,  "a25db21f09d01ac77bfa3c88eb4ca25d87b357333721a07c1d4d568bc7dddaa4", 1522696898, 493968530);
			_checkpoints.emplace_back(82656,  "d3238d44fcaa42ec51f9314185eb57950f00184152ccc38ff94c99eaba27ac32", 1523909845, 494238365);
			_checkpoints.emplace_back(92736,  "9f2e7f0c8668cb37e7e249123ba5436239245c3a582aac2cfa00e63f84fd7259", 1525118590, 494040753);
			_checkpoints.emplace_back(102816, "21fbf33a8ac97d4da0660fc8cf1d4b42ced6cd39cb6ff9f81aa4ed898b4adfb2", 1526317611, 492534043);
			_checkpoints.emplace_back(112896, "db22a1202b815fbc483a6a34f7faca80286351599082b1beb1c9155a6fb35b03", 1527524832, 492355105);
			_checkpoints.emplace_back(122976, "4755a56fa7a9bfa12125d573bf4c47af1caaf3b749de890bf1881d9de04778e2", 1528725950, 491428429);
			_checkpoints.emplace_back(133056, "668fcd9479fff7bbd267746046ad9fe1875be00d34fa67e300d83d199e9e5624", 1530001812, 503350194);
			_checkpoints.emplace_back(143136, "c7890d31f73b85c03593f7c6dace0a7604e6709519cff75cf945b7c83b0f7aee", 1531153562, 490135368);
			_checkpoints.emplace_back(153216, "1bc095d2c47131e1a4e1d08ca533b9caac30008e319441b7d202ddfe75e263cb", 1532368202, 490129493);
			_checkpoints.emplace_back(163296, "dc35e7815ba7f2def7fb656cca1bde709ebe99cd4f66c39ac573a2763be559d9", 1533576903, 490408901);
			_checkpoints.emplace_back(173376, "ce52eca3db14476ae7b01faa0258bc21b4b93fe0b0364342785cf479f213f851", 1534787121, 489990925);
			_checkpoints.emplace_back(183456, "03665026718f02ccb9c764aafe20ab5c1f4963cf7eac6931a300d40b28343f59", 1535433865, 457157190);
			_checkpoints.emplace_back(193536, "ad48cf72bbee8634044ac24dd5ea207c0ba8d6e0dec9b97a64b3eea55c73aa79", 1535895095, 407453180);
			_checkpoints.emplace_back(203616, "5e230d36db48b3951bd7d4979d25aa18a83c24941b083d63bfef286489acce36", 1537105427, 407173769);
			_checkpoints.emplace_back(213696, "d01abf397fd29c2ba66da728c35834aa0c8a21a7d1c9e6f2e7db4c90059ddd26", 1538328848, 407607682);
			_checkpoints.emplace_back(223776, "ffc80b94b160752fcb89e3d021f65040f5c0861d23625d7437930215ecc11b48", 1539537072, 407725582);
			_checkpoints.emplace_back(233856, "5ace1133b6a861b20a4b3885c686b3f6d54829504105019d28b51358721e12e8", 1540749706, 407616602);
			_checkpoints.emplace_back(243936, "e14e533e6d6640ef026f29cc1ab14eaeaa4cd999072ca11d8d15e066665c688f", 1541951161, 407273974);
			_checkpoints.emplace_back(254016, "6803d7f909508a3d36704edffc5acfbf87e02d1fc2250257d862f173aca9ed0d", 1543162984, 407276433);
			_checkpoints.emplace_back(258048, "02fa157f43a6fae7f464c025ebbadc778dcda972df712918b410e3d136a14e12", 1543653867, 407635758);
			_standardPort = 20866;
			_magicNumber = 2017001;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::MainNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-mainnet-001.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-002.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-003.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-004.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-005.elastos.org");
			_checkpoints.emplace_back(0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1530360000, 486801407);
			_checkpoints.emplace_back(2016,  "df060a6475ace78657e1ec945cebb37f2ffb6367b185f9ba5edfa987f16b84f4", 1531426582, 520126740);
			_checkpoints.emplace_back(6048,  "4e6a53d3e3e61d4883a10d62fb42afb62ad6f10807bcc3791db284f43b063671", 1532395676, 522028475);
			_checkpoints.emplace_back(10080, "00ac65927dcf741e87b0560b9676a35abc5d1ab74afc6144f4159315f15561d2", 1535069960, 539089007);
			_checkpoints.emplace_back(14112, "d342ee0817e30a57a515ccfaaf836e7314aac1235d0659d5e6ffc46671ae4979", 1535367638, 527395580);
			_checkpoints.emplace_back(18144, "fc4e7ba460e38964faeb33873418c9fcd3ae2648a543cc1e8f5c47806b593ac8", 1535503457, 504235070);
			_checkpoints.emplace_back(22176, "9ef0a2d21af1f1ebba5b092ce0f672b221ea7d1f2e765790c0741aaff7667da2", 1535640920, 486964831);
			_checkpoints.emplace_back(26208, "a9eb520903f5f1ec1a11a3f5040791d59ae63b8e16846a994bbd303ba43b50db", 1536008684, 475415774);
			_checkpoints.emplace_back(30240, "470d858db326df0e738248ac5f5811f1b31a0c1ca1892f17a93673efefde3a45", 1536500538, 476055185);
			_checkpoints.emplace_back(34272, "f78fc41c0d2ba1c0c133ebcb351ecfedc42324a31201b6c207d0165016430a74", 1536991969, 476548897);
			_checkpoints.emplace_back(38304, "9592ad50c220ea2900d0c3659df470b4e8e459ac6ee2d43ddc9b6ddd15166645", 1537492347, 477909524);
			_checkpoints.emplace_back(42336, "548c15f8daede3d85aed4a6ad8bf013c6cb2b3c7ea7aba0134ffb2adc450850a", 1537976718, 478079428);
			_checkpoints.emplace_back(46368, "f9e8780e4df7eeecf80f8cb8fcdd2f08f2357da9f50557235373c20123e9fa45", 1538494105, 486580306);
			_checkpoints.emplace_back(50400, "b877aa70a40ec714b211c19d0f00060f86e659acf2766bd2a3d6a2fc24f29a6d", 1539105182, 486700362);
			_checkpoints.emplace_back(54432, "e40aab4302d385b0ec4cc85ca8a1c1a8c338e2b01d0a67c70e7f9bc0b7e4995b", 1539618297, 486739142);
			_checkpoints.emplace_back(58464, "800c33ab6faf802559f94106f9012f734ff08a2f183f6eccebc3016735557602", 1540197515, 487108954);
			_checkpoints.emplace_back(62496, "760a11232f9fcf83dd30a17f9feac767c3a1158371fa0e051b1f2956e8b96ecc", 1540686763, 487128669);
			_checkpoints.emplace_back(66528, "886813b522b8ed25b5ed49a466a60dabc9ec873505f6febf650013d0de8ff760", 1541180178, 487200624);
			_checkpoints.emplace_back(70560, "a8efba9d67b13d05b1037b8a53ee917c19e1f0505808b8ba345ee6dd29fb2f5e", 1541773076, 488572069);
			_checkpoints.emplace_back(74592, "63ba136d741cdc27fa7c03ce438c1d2b9caf7540a3be801c967c09697f0458dc", 1542924090, 503654166);
			_checkpoints.emplace_back(76608, "5936928083508074454609d054f1da99b1b9342b5df46dcf1af87347382f9d3e", 1543749293, 520134229);
			_standardPort = 20608;
			_magicNumber = 2017002;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		// test net
		void ChainParams::TestNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-testnet-002.elastos.org");
			_dnsSeeds.emplace_back("node-testnet-003.elastos.org");
			_dnsSeeds.emplace_back("node-testnet-004.elastos.org");
			_checkpoints.emplace_back(0,      "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048);
			_checkpoints.emplace_back(8064,   "fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158", 1522645593, 494006531);
			_checkpoints.emplace_back(14112,  "20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e", 1523380720, 494547269);
			_checkpoints.emplace_back(20160,  "0fd0ecfabdd3f3405b9808e4f67749232d23404fd1c90a4e2c755ead8651e759", 1524792298, 503351573);
			_checkpoints.emplace_back(26208,  "d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c", 1525542345, 503358411);
			_checkpoints.emplace_back(32256,  "0c03acf34f4cd5df02062e658f91dfe884788831b940b18a136f7d33adae00b0", 1526270878, 503361032);
			_checkpoints.emplace_back(38304,  "ea313df1089b8b001c40565aa5e90d17da51885a29459b966d0019ac7dd90002", 1526997348, 503361573);
			_checkpoints.emplace_back(44352,  "77fcd9c9e3a875b46d0029805cfe3bdb06f7beef49ea40eaadb20368cd0bea2e", 1527704178, 503349609);
			_checkpoints.emplace_back(50400,  "176751ba71545428b0a1740abcd0f1846b97b1455fbbcb27a5c778a575fa8b71", 1528433088, 503351979);
			_checkpoints.emplace_back(56448,  "d6faaa1526829d58cbd2c7afbabdb7bffefcc5494d808a79ad79d9d6d6fb17af", 1529153218, 494866089);
			_checkpoints.emplace_back(62496,  "8cb6da886b6258e8a6daefce9d5566575bd2ba95b4bd684105954ab9e69be042", 1529864598, 493529298);
			_checkpoints.emplace_back(68544,  "c7595156595f1b925f5dd3aeeaabc9c3b3605e5d9ba934c396a31126398a10fe", 1530583548, 492729858);
			_checkpoints.emplace_back(74592,  "47ea897ceb1ad471b5a7277e348ad43456b08d149db39b419174cb295be4fed8", 1531345056, 503351198);
			_checkpoints.emplace_back(80640,  "db4f79022b23ffada435eb3d0681deb11a1591428044cf4625188f1c6277f9b1", 1532146578, 503381165);
			_checkpoints.emplace_back(86688,  "a25b7207c98cbbede93db67ca41a0ac308d198c556eea955ff63f52b0b70131a", 1532870279, 503379835);
			_checkpoints.emplace_back(92736,  "c5134b4d7b275a6109cf2f1e7c4f31ad96a4c6c19d2bfd98e7b655d9d49d5723", 1533598732, 503377199);
			_checkpoints.emplace_back(98784,  "552732e51a44b3cf99992447b9b3f79226fa90f0f9f1b47e15836abc2c334322", 1534328247, 503380456);
			_checkpoints.emplace_back(104832, "ec5c35a62347241f33949ac5b072af32a3fe0f3e9cc1ccbabe7598629c94940a", 1535057104, 503376374);
			_checkpoints.emplace_back(110880, "743823d8e5d1e74aa7aa8de9a20ca3b42dad4c1115b1631d6071d66d68f6a4f9", 1535780560, 503376620);
			_checkpoints.emplace_back(116928, "7151ea9c0d36479c9f6e92776b88f9f925bcbcddcff44deb7463205dee8f06d3", 1536517600, 503382324);
			_checkpoints.emplace_back(122976, "e8e5c0ca094d156fb5ce864c3a3ec68925de51a187060920b1252936b21356cd", 1537239070, 503382468);
			_checkpoints.emplace_back(129024, "1e08db102db105f1d58703da1153322001533a3d4fe31a794dfb118600f1a615", 1537969555, 503380928);
			_checkpoints.emplace_back(131040, "2f8549b8a30527a5878a3c105b4c6d8bb4faf4619c6917d704001424f6625b5c", 1538216125, 503384495);
			_checkpoints.emplace_back(133056, "2a8016306ba32a9b4451a5024a463f1e59240920963cddaf011616e3be020c83", 1538451280, 503379347);
			_checkpoints.emplace_back(135072, "681fee1c6ec452b1c38b5aadb26b9b60371b8104136bcef628c5b828643aebf9", 1538701705, 503383651);
			_checkpoints.emplace_back(137088, "b1f56177099df2ca6c0f2ddcba343589d2aadab8d32dfa71507413a1d9d71b25", 1539108569, 494074913);
			_checkpoints.emplace_back(139104, "3c67aa0d2081ad03e988c883a747febee66fb19b935edf27e60f6571c4a14c7e", 1539423774, 503391211);
			_checkpoints.emplace_back(141120, "8c1451e25bc148f16c99121945fcc97d03ca4ef480fa55b84611cd6b52672889", 1539676171, 503378272);
			_checkpoints.emplace_back(143136, "22a3efa19fcfdbcd5a1be9c4e66dadf317f31c6de53ef41a5603dc1d04d6fa21", 1539918360, 503378173);
			_checkpoints.emplace_back(145152, "602220ec448dd2b80b5fd9e671347d8f5780d3360bd10b0a89e56f17490885d7", 1540128615, 503352411);
			_checkpoints.emplace_back(147168, "4a0ffb895b654fc46d068eb8110d715c176119ad13e9cffdda5bf88db412590c", 1540414770, 503373743);
			_checkpoints.emplace_back(149184, "f0ee2f21636023aa2640fdda5c6401085f4e1ba9d0f01453d116d2e61190dcdc", 1540592838, 491207374);
			_checkpoints.emplace_back(151200, "0536033af4e39efb760d9070a98e546ba53cc16d36841d440d3e7686cb852f79", 1540978309, 503363606);
			_checkpoints.emplace_back(153216, "e648b4fcafe6e408d84114a44b4e9e53d194bf5df187d00481eeb1a30e5f566f", 1541250942, 503380880);
			_checkpoints.emplace_back(155232, "6450e98c2fc6504d1f8234364580d3d798399b98663b00527249f4ffdd139997", 1541494485, 503383019);
			_checkpoints.emplace_back(157248, "7291748edc1c325aa704bada0925532bae76a59e4a6f5286b8643e4c6588821e", 1541713569, 503358385);
			_checkpoints.emplace_back(159264, "a72b0aab079818986cca9fbee3155fe9d20c7cfb64a1aed3edfa603d6d688c6e", 1541946004, 503356074);
			_checkpoints.emplace_back(161280, "ad806567458832517936b917c2afdd4823ca1f18ebe262ffed9a41c5d45c925b", 1542234911, 503380960);
			_checkpoints.emplace_back(163296, "95da2b5178376f4217237d960093dd7a6b5f0bdde86f5681bc463b121c2c77e2", 1542480644, 503376144);
			_checkpoints.emplace_back(165312, "c4aea704d135021876cebe2f7801af6d2f9e1258ab0f53814989c7631cbf890b", 1542741008, 503404457);
			_checkpoints.emplace_back(167328, "0e81022030c88a719b5d60b638a9d9e2ae0b0688eb309ff3ab47c82975936daf", 1542969353, 503382380);
			_checkpoints.emplace_back(173376, "20c8f646071b2d9a41abcecb55bf09e2af7caa1c692f571e8f6e2273cb547738", 1543844436, 503357414);
			_standardPort = 21866;
			_magicNumber = 2018001;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::TestNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-testnet-001.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-002.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-003.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-004.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-005.elastos.org");
			_checkpoints.emplace_back(0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,  "9b3069a05478988d4c9d2d4b941af048680dbe92353bc8f0cf282766aa935edb", 1532276131, 505977014);
			_checkpoints.emplace_back(4032,  "6e8f5e21ddd736bb62dcd3ae445444702dc0b0ee560b4161877d0c8f0d9ad448", 1532628770, 510283913);
			_checkpoints.emplace_back(6048,  "8cef466bdc7c9ed79fdd6b36babb5b6670c916c07d87b473327460e57d21799b", 1533004749, 520187129);
			_checkpoints.emplace_back(8064,  "46a9dd847b62f278a26a93e3f7a4cc135b30754e07f2754d6ac0baf6870c8060", 1533346700, 520376318);
			_checkpoints.emplace_back(10080, "09cdb3d720a7095c7023241361ca8a317c75273fbdd0e5dc268667683e360780", 1533694521, 520890958);
			_checkpoints.emplace_back(12096, "2790468186eb341dfe06fc6b8a66ad48d2bfb597d0f612e2a917d703c951aee1", 1534057013, 521637984);
			_checkpoints.emplace_back(14112, "f5582a7bab4ec851172dd96b9c18d8a046081de65ed05f316e4dc14a5a8fe176", 1534506459, 536912421);
			_checkpoints.emplace_back(16128, "8b78d2fee6751c82d419e5eb98bbf83a1514a046ae9ffd1462aceaeed3f189ec", 1534771533, 536935584);
			_checkpoints.emplace_back(18144, "ba98bdbeb358fc2ac565dd3a6f706cef7dfdc60063f1f3517311744c3a908de4", 1535034489, 536952176);
			_checkpoints.emplace_back(20160, "b4d0dcec130dccd6cfa61880cdfe30259e90c69d1f83056f0495910cca05246b", 1535300757, 536980894);
			_checkpoints.emplace_back(22176, "9738541d86b7843be83926be997164615a3102e33edece26da351fdae70d2d6a", 1535557289, 536997059);
			_checkpoints.emplace_back(24192, "e9198d6fdd89d292f7e967266c22c59e245d015d93c0428626211965b8349d05", 1535828463, 537035498);
			_checkpoints.emplace_back(26208, "8636093320982e3707224cb19252c2f2de124bd1c480c3e4ba35f6d564e2d713", 1536086668, 537085786);
			_checkpoints.emplace_back(28224, "b98d6c3e7f424d44bf4f21f9093be7b2d29f2134137949769718c33ae22bd205", 1536361613, 537169686);
			_checkpoints.emplace_back(30240, "8c43de5c79cd48da6754b2098fcb230d525b7b0f1e44fc7c87b2b38064269991", 1536626736, 537255791);
			_checkpoints.emplace_back(32256, "a8000672bff80dcf7117062073df6f3cb511be0e49c2862f91432a00eeef053e", 1536886078, 537301386);
			_checkpoints.emplace_back(34272, "640298c39b33c1298d87e482115053177ae60b721e23801db0061bb4e24d393b", 1537146343, 537429158);
			_checkpoints.emplace_back(36288, "5d2a3ae65b42c7233e2fcbfc2a4f86eb24a693ab7e3b47c38830e57c956ff49c", 1537593491, 539041984);
			_checkpoints.emplace_back(38304, "3763dde6fa684f31615b4237a17116777bf807a19701c3d5d02604e9bdcea1a6", 1538035204, 545259519);
			_checkpoints.emplace_back(40320, "25ad2ec3bd7531301d367b4f7b1357e567665213c776744775fed7f80b9b0349", 1538398560, 545259519);
			_checkpoints.emplace_back(42336, "18086d3158974bbc1a331b194e0580504ed76058d88da84fc446b0020b63c329", 1538663566, 545259519);
			_standardPort = 21608;
			_magicNumber = 201812151;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		// reg net (reg test)
		void ChainParams::RegNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-regtest-002.elastos.org");
			_dnsSeeds.emplace_back("node-regtest-003.elastos.org");
			_dnsSeeds.emplace_back("node-regtest-004.elastos.org");
			_checkpoints.emplace_back(0,      "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048);
			_checkpoints.emplace_back(8064,   "fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158", 1522645593, 494006531);
			_checkpoints.emplace_back(14112,  "20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e", 1523380720, 494547269);
			_checkpoints.emplace_back(20160,  "40eec8f7f3c7f4d9ea4c7dbe5eb85456c72ee79f7e5940a5e7d5f4c80bc1e7a7", 1524736650, 503356298);
			_checkpoints.emplace_back(26208,  "3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a", 1525472220, 503357792);
			_checkpoints.emplace_back(32256,  "9ba5fec5acaf23302110e333b022a074de88b4e36b9264e98297cbaec09f591f", 1526225370, 503370323);
			_checkpoints.emplace_back(38304,  "9307bcfe0fe867a824e3a9139d626a607aa1c09396c027df8fa243b4110f499d", 1527042222, 503415621);
			_checkpoints.emplace_back(44352,  "cf8516eb910306862bcc206519b0e6eb187dc20ce384ca525dd1c0ec751c92f2", 1528221733, 503374579);
			_checkpoints.emplace_back(50400,  "8e34fdc18ef27996022cc36d85f856690990faa077d92b78989c04f113205cd0", 1528926400, 493749763);
			_checkpoints.emplace_back(56448,  "74d8765907eb63ba286d2d8373de916c25c5daf5250c25d652597c2422e0bc9c", 1530223515, 503358059);
			_checkpoints.emplace_back(62496,  "398e154da8acf7565af3a8cae969b2aa8cdf0abb5817bcb0e2b64c8155557416", 1531091111, 503350455);
			_checkpoints.emplace_back(68544,  "2b24892cda519c15de01dd18298853c5dcd5f278c8a92ccc7e1060ca2f206c71", 1531796891, 503349562);
			_checkpoints.emplace_back(74592,  "f7afa994b23a5628f109e1de8563f4ae5963a814b1c972425d921d581bcec7a0", 1532636667, 503374250);
			_checkpoints.emplace_back(80640,  "bcadac789eec2262f099cc0f57ef53937520ddbc298cb2341d00bce1cbabd7ec", 1533423380, 503380311);
			_checkpoints.emplace_back(86688,  "7de2bfb514c7c466aa4903d47d9e5ea89d8cd13e60fc134b1ac7a6337499fcab", 1534151990, 503377735);
			_checkpoints.emplace_back(92736,  "d19883bc202ef4802f73bd18ce2456c8c933e9a2793bb566efc21cf3a48fe52c", 1536149363, 503375367);
			_checkpoints.emplace_back(98784,  "a8efe800d2a18bf7dca867e307561d11e33abe20fc8a6c20e82f05efcaa3f6cd", 1536880958, 503376323);
			_checkpoints.emplace_back(104832, "bfe6f03363b445f7c6d6acbb8b8ab71641178e0d1f612fc459a622b19c741377", 1537577738, 503359020);
			_checkpoints.emplace_back(110880, "3f2ea95ca953b97a638a24627a41826973a485fc7c0e661ad7c6cb5870910e5f", 1538309387, 503358893);
			_checkpoints.emplace_back(112896, "8ac3137dfb231d3f89878f28342791e457efb029c6710ead8ed0d759ae6dc8e2", 1538558993, 503359813);
			_checkpoints.emplace_back(114912, "4d2197b83c20edac4125a9414b56d0a9a0f0252858c7eb95b50805626f1c7256", 1538794988, 503359557);
			_standardPort = 22866;
			_magicNumber = 20180627;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::RegNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-regtest-001.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-002.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-003.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-004.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-005.elastos.org");
			_checkpoints.emplace_back(0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407);
			_standardPort = 22608;
			_magicNumber = 201809031;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::MainNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				MainNetMainChainParamsInit();
			} else if (type == Idchain) {
				MainNetIDChainParamsInit();
			} else {
				ParamChecker::checkCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

		void ChainParams::TestNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				TestNetMainChainParamsInit();
			} else if (type == Idchain) {
				TestNetIDChainParamsInit();
			} else {
				ParamChecker::checkCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

		void ChainParams::RegNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				RegNetMainChainParamsInit();
			} else if (type == Idchain) {
				RegNetIDChainParamsInit();
			} else {
				ParamChecker::checkCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

	}
}

