// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELACOINPATH_H__
#define __ELASTOS_SDK_ELACOINPATH_H__

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

#include "Mstream.h"
#include "SubWalletType.h"

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo {
		public:
			CoinInfo();

			const std::string &getChainId() const;

			void setChainId(const std::string &id);

			uint32_t getEarliestPeerTime() const;

			void setEaliestPeerTime(uint32_t time);

			int getIndex() const;

			void setIndex(int index);

			int getUsedMaxAddressIndex() const;

			void setUsedMaxAddressIndex(int index);

			bool getSingleAddress() const;

			void setSingleAddress(bool singleAddress);

			uint64_t getFeePerKb() const;

			void setFeePerKb(uint64_t fee);

			int getForkId() const;

			void setForkId(int forkId);

			SubWalletType getWalletType() const;

			void setWalletType(SubWalletType type);

			const std::string &getEncryptedKey() const;

			void setEncryptedKey(const std::string &key);

			const std::string &getChainCode() const;

			void setChainCode(const std::string &code);

			const std::string &getPublicKey() const;

			void setPublicKey(const std::string &pubKey);

			uint64_t getMinFee() const;

			void setMinFee(uint64_t fee);

		private:
			JSON_SM_LS(CoinInfo);
			JSON_SM_RS(CoinInfo);
			TO_JSON(CoinInfo);
			FROM_JSON(CoinInfo);

		private:
			std::string _chainId;
			uint32_t _earliestPeerTime;
			int _forkId;
			int _index;
			int _usedMaxAddressIndex;
			bool _singleAddress;
			uint64_t _minFee;
			uint64_t _feePerKb;
			SubWalletType _walletType;
			std::string _publicKey;
			std::string _encryptedKey;
			std::string _chainCode;
		};

	}
}

#endif //__ELASTOS_SDK_ELACOINPATH_H__
