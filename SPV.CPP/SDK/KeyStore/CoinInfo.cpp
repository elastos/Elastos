// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

namespace Elastos {
	namespace SDK {

		CoinInfo::CoinInfo() :
				_chainId(""),
				_earliestPeerTime(0),
				_forkId(0),
				_index(0),
				_usedMaxAddressIndex(0),
				_singleAddress(false),
				_feePerKb(0),
				_walletType(Normal) {

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

		uint64_t CoinInfo::getFeePerKb() const {
			return _feePerKb;
		}

		void CoinInfo::setFeePerKb(uint64_t fee) {
			_feePerKb = fee;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const CoinInfo &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, CoinInfo &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const CoinInfo &p) {
			j["ChainID"] = p._chainId;
			j["EarliestPeerTime"] = p._earliestPeerTime;
			j["Index"] = p._index;
			j["UsedMaxAddressIndex"] = p._usedMaxAddressIndex;
			j["SingleAddress"] = p._singleAddress;
			j["WalletType"] = int(p.getWalletType());
			j["EncryptedKey"] = p._encryptedKey;
			j["ChainCode"] = p._chainCode;
		}

		void from_json(const nlohmann::json &j, CoinInfo &p) {
			p._chainId = j["ChainID"].get<std::string>();
			p._earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();
			p._index = j["Index"].get<int>();
			p._usedMaxAddressIndex = j["UsedMaxAddressIndex"].get<int>();
			p._singleAddress = j["SingleAddress"].get<bool>();
			p._walletType = (SubWalletType) j["WalletType"].get<int>();
			p._encryptedKey = j["EncryptedKey"].get<std::string>();
			p._chainCode = j["ChainCode"].get<std::string>();
		}

		int CoinInfo::getForkId() const {
			return _forkId;
		}

		void CoinInfo::setForkId(int forkId) {
			_forkId = forkId;
		}

		SubWalletType CoinInfo::getWalletType() const {
			return _walletType;
		}

		void CoinInfo::setWalletType(SubWalletType type) {
			_walletType = type;
		}

		const std::string &CoinInfo::getEncryptedKey() const {
			return _encryptedKey;
		}

		void CoinInfo::setEncryptedKey(const std::string &key) {
			_encryptedKey = key;
		}

		const std::string &CoinInfo::getChainCode() const {
			return _chainCode;
		}

		void CoinInfo::setChainCode(const std::string &code) {
			_chainCode = code;
		}
	}
}