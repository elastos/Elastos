// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

namespace Elastos {
	namespace SDK {

		CoinInfo::CoinInfo() :
				_chainId(""),
				_earliestPeerTime(0),
				_index(0),
				_usedMaxAddressIndex(0),
				_singleAddress(false),
				_balanceUnit(1.0) {

		}

		const std::string &CoinInfo::getChainId() const {
			return _chainId;
		}

		void CoinInfo::setChainId(const std::string &id) {
			_chainId = id;
		}

		uint32_t CoinInfo::getEarliestPeerTime() const {
			return _earliestPeerTime;
		}

		void CoinInfo::setEaliestPeerTime(uint32_t time) {
			_earliestPeerTime = time;
		}

		int CoinInfo::getIndex() const {
			return _index;
		}

		void CoinInfo::setIndex(int index) {
			_index = index;
		}

		int CoinInfo::getUsedMaxAddressIndex() const {
			return _usedMaxAddressIndex;
		}

		void CoinInfo::setUsedMaxAddressIndex(int index) {
			_usedMaxAddressIndex = index;
		}

		bool CoinInfo::getSingleAddress() const {
			return _singleAddress;
		}

		void CoinInfo::setSingleAddress(bool singleAddress) {
			_singleAddress = singleAddress;
		}

		double CoinInfo::getBalanceUnit() const {
			return _balanceUnit;
		}

		void CoinInfo::setBalanceUnit(double balanceUnit) {
			_balanceUnit = balanceUnit;
		}
	}
}