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
		"FeePerKB": 10000
	},
	"IDChain": {
		"Index": 1,
		"MinFee": 10000,
		"FeePerKB": 10000
	},
	"TokenChain": {
		"Index": 2,
		"MinFee": 10000,
		"FeePerKB": 10000
	},
	"ETHSC": {
		"Index": 3,
		"MinFee": 0,
		"FeePerKB": 0
	},
	"ETHDID": {
		"Index": 4,
		"MinFee": 0,
		"FeePerKB": 0
	}
}
		)"_json;

	}
}

#endif

