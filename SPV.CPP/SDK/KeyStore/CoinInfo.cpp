// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

namespace Elastos {
	namespace ElaWallet {

		CoinInfo::CoinInfo() :
				_chainId(""),
				_earliestPeerTime(0),
				_forkId(0),
				_index(0),
				_usedMaxAddressIndex(0),
				_singleAddress(false),
				_enableP2P(true),
				_feePerKb(0),
				_minFee(0),
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

		uint32_t CoinInfo::getIndex() const {
			return _index;
		}

		void CoinInfo::setIndex(uint32_t index) {
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
			j["MinFee"] = p._minFee;
			j["FeePerKB"] = p._feePerKb;
			j["EnableP2P"] = p._enableP2P;
			j["ReconnectSeconds"] = p._reconnectSeconds;
			j["ChainCode"] = p._chainCode;
			j["PublicKey"] = p._publicKey;
		}

		void from_json(const nlohmann::json &j, CoinInfo &p) {
			p._chainId = j["ChainID"].get<std::string>();
			p._earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();
			p._index = j["Index"].get<int>();
			p._usedMaxAddressIndex = j["UsedMaxAddressIndex"].get<int>();
			p._singleAddress = j["SingleAddress"].get<bool>();
			p._walletType = (SubWalletType) j["WalletType"].get<int>();
			p._minFee = j["MinFee"].get<uint64_t>();
			p._feePerKb = j["FeePerKB"].get<uint64_t>();
			p._reconnectSeconds = j["ReconnectSeconds"].get<uint32_t>();
			p._chainCode = j["ChainCode"].get<std::string>();
			p._publicKey = j["PublicKey"].get<std::string>();
			if (j.find("EnableP2P") != j.end())
				p._enableP2P = j["EnableP2P"].get<bool>();
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

		uint64_t CoinInfo::getMinFee() const {
			return _minFee;
		}

		void CoinInfo::setMinFee(uint64_t fee) {
			_minFee = fee;
		}

		const std::string &CoinInfo::getGenesisAddress() const {
			return _genesisAddress;
		}

		void CoinInfo::setGenesisAddress(const std::string &address) {
			_genesisAddress = address;
		}

		bool CoinInfo::getEnableP2P() const {
			return _enableP2P;
		}

		void CoinInfo::setEnableP2P(bool enable) {
			_enableP2P = enable;
		}

		uint32_t CoinInfo::getReconnectSeconds() const {
			return _reconnectSeconds;
		}

		void CoinInfo::setReconnectSeconds(uint32_t reconnectSeconds) {
			_reconnectSeconds = reconnectSeconds;
			}

		const std::string &CoinInfo::getChainCode() const {
			return _chainCode;
		}

		void CoinInfo::setChainCode(const std::string &code) {
			_chainCode = code;
		}

		const std::string &CoinInfo::getPublicKey() const {
			return _publicKey;
		}

		void CoinInfo::setPublicKey(const std::string &pubKey) {
			_publicKey = pubKey;
		}
	}
}