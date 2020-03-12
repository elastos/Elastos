// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PRVNET_H__
#define __ELASTOS_SDK_PRVNET_H__

namespace Elastos {
	namespace ElaWallet {
		const nlohmann::json DefaultPrvNetConfig = R"(
{
	"NetType": "PrvNet",
	"ELA": {
		"Index": 0,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2018201,
			"StandardPort": 10018,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"172.26.0.207"
			],
			"CheckPoints": [
				[0, "8df798783097be3a8f382f6537e47c7168d4bf4d741fa3fa8c48c607a06352cf", 1513936800, 486801407]
			]
		}
	},
	"IDChain": {
		"Index": 1,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2018202,
			"StandardPort": 10138,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"172.26.0.207"
			],
			"CheckPoints": [
				[0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407]
			]
		}
	},
	"TokenChain": {
		"Index": 2,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2019004,
			"StandardPort": 30618,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"172.26.0.165"
			],
			"CheckPoints": [
				[0,      "b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e", 1551744000, 486801407]
			]
		}
	},
	"ETHSC": {
		"Index": 3,
		"MinFee": 0,
		"FeePerKB": 0,
		"DisconnectionTime": 0,
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 0,
			"StandardPort": 0,
			"TargetTimeSpan": 0,
			"TargetTimePerBlock": 0,
			"DNSSeeds": [
				"127.0.0.1"
			],
			"CheckPoints": [
				[0, "0000000000000000000000000000000000000000000000000000000000000000", 1, 1]
			]
		}
	}
}
		)"_json;

	}
}

#endif

