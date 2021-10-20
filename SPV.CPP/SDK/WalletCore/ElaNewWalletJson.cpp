// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"
#include "CoinInfo.h"
#include "HDKeychain.h"
#include "BIP39.h"
#include "Base58.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <support/BRKey.h>

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson() :
			_singleAddress(false) {

		}

		ElaNewWalletJson::~ElaNewWalletJson() {
		}

		void ElaNewWalletJson::AddCoinInfo(const CoinInfoPtr &info) {
			_coinInfoList.push_back(info);
		}

		void ElaNewWalletJson::ClearCoinInfo() {
			_coinInfoList.clear();
		}

		const std::vector<CoinInfoPtr> &ElaNewWalletJson::GetCoinInfoList() const {
			return _coinInfoList;
		}

		void ElaNewWalletJson::SetCoinInfoList(const std::vector<CoinInfoPtr> &list) {
			_coinInfoList = list;
		}

		bool ElaNewWalletJson::SingleAddress() const {
			return _singleAddress;
		}

		void ElaNewWalletJson::SetSingleAddress(bool value) {
			_singleAddress = value;
		}

		const std::string &ElaNewWalletJson::OwnerPubKey() const {
			return _ownerPubKey;
		}

		void ElaNewWalletJson::SetOwnerPubKey(const std::string &pubkey) {
			_ownerPubKey = pubkey;
		}

		const std::string &ElaNewWalletJson::xPubKeyHDPM() const {
			return _xPubKeyHDPM;
		}

		void ElaNewWalletJson::SetxPubKeyHDPM(const std::string &xpub) {
			_xPubKeyHDPM = xpub;
		}

		const std::string &ElaNewWalletJson::GetSeed() const {
			return _seed;
		}

		void ElaNewWalletJson::SetSeed(const std::string &seed) {
			_seed = seed;
		}

		const std::string &ElaNewWalletJson::GetETHSCPrimaryPubKey() const {
			return _ethscPrimaryPubKey;
		}

		void ElaNewWalletJson::SetETHSCPrimaryPubKey(const std::string &pubkey) {
			_ethscPrimaryPubKey = pubkey;
		}

        const std::string &ElaNewWalletJson::GetxPubKeyBitcoin() const {
            return _xPubKeyBitcoin;
        }

        void ElaNewWalletJson::SetxPubKeyBitcoin(const std::string &xpub) {
            _xPubKeyBitcoin = xpub;
        }

		nlohmann::json ElaNewWalletJson::ToJson(bool withPrivKey) const {
			nlohmann::json j = ElaWebWalletJson::ToJson(withPrivKey);
			ToJsonCommon(j);
			return j;
		}

		void ElaNewWalletJson::FromJson(const nlohmann::json &j) {
			ElaWebWalletJson::FromJson(j);
			FromJsonCommon(j);
		}

		void ElaNewWalletJson::ToJsonCommon(nlohmann::json &j) const {
			nlohmann::json coinInfoList;
			for (size_t i = 0; i < _coinInfoList.size(); ++i)
				coinInfoList.push_back(_coinInfoList[i]->ToJson());

			j["CoinInfoList"] = coinInfoList;
			j["SingleAddress"] = _singleAddress;
			j["OwnerPubKey"] = _ownerPubKey;
			j["xPubKeyHDPM"] = _xPubKeyHDPM;
			j["seed"] = _seed;
			j["ethscPrimaryPubKey"] = _ethscPrimaryPubKey;
            j["xPubKeyBitcoin"] = _xPubKeyBitcoin;
		}

		void ElaNewWalletJson::FromJsonCommon(const nlohmann::json &j) {
			if (j.find("CoinInfoList") != j.end()) {
				_coinInfoList.clear();
				nlohmann::json coinInfoList = j["CoinInfoList"];
				for (nlohmann::json::iterator it = coinInfoList.begin(); it != coinInfoList.end(); ++it) {
					CoinInfoPtr coinInfo(new CoinInfo());
					coinInfo->FromJson((*it));
					_coinInfoList.push_back(coinInfo);
				}
			}

			if (j.find("SingleAddress") != j.end()) {
				_singleAddress = j["SingleAddress"].get<bool>();
			}

			if (j.find("OwnerPubKey") != j.end()) {
				_ownerPubKey = j["OwnerPubKey"].get<std::string>();
			}

			if (j.find("xPubKeyHDPM") != j.end()) {
				_xPubKeyHDPM = j["xPubKeyHDPM"].get<std::string>();
			}

			if (j.find("ethscPrimaryPubKey") != j.end()) {
				_ethscPrimaryPubKey = j["ethscPrimaryPubKey"].get<std::string>();
			}

            if (j.find("xPubKeyBitcoin") != j.end()) {
                _xPubKeyBitcoin = j["xPubKeyBitcoin"].get<std::string>();
            }

			if (j.find("CoSigners") != j.end() && j["Type"] == "MultiSign") {
				ErrorChecker::ThrowParamException(Error::KeyStore, "Unsupport old version multi-sign keystore");
			}

			if (j.find("RequiredSignCount") != j.end()) {
				ErrorChecker::ThrowParamException(Error::KeyStore, "Unsupport old version multi-sign keystore");
			}

			std::string passphrase;
			if (j.find("PhrasePassword") != j.end()) {
				passphrase = j["PhrasePassword"].get<std::string>();
				if (!passphrase.empty())
					_mnemonicHasPassphrase = true;
			}

			if (j.find("IsSingleAddress") != j.end()) {
				_singleAddress = j["IsSingleAddress"];
			}

            if (j.find("seed") != j.end()) {
                _seed = j["seed"].get<std::string>();
            }

            if (_seed.empty() && !_mnemonic.empty() &&
                    (!_mnemonicHasPassphrase || (_mnemonicHasPassphrase && !passphrase.empty()))) {
                Log::info("Regerate seed from old keystore");
                uint512 seed = BIP39::DeriveSeed(_mnemonic, passphrase);
                bytes_t seedBytes = seed.bytes();
                _seed = seedBytes.getHex();
            }

            if (_xPrivKey.empty() && !_seed.empty()) {
                Log::info("Regenerate xprv from old keystore");
                bytes_t seedBytes(_seed);
                HDSeed hdseed(seedBytes);
                HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
                _xPrivKey = Base58::CheckEncode(rootkey.extkey());
            }

            if (!_xPrivKey.empty()) {
                bytes_t bytes;
                Base58::CheckDecode(_xPrivKey, bytes);
                HDKeychain rootkey(CTElastos, bytes);

                _ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();
                _xPubKeyHDPM = Base58::CheckEncode(rootkey.getChild("45'").getPublic().extkey());
                _xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

                HDKeychain requestKey = rootkey.getChild("1'/0");
                _requestPrivKey = requestKey.privkey().getHex();
                _requestPubKey = requestKey.pubkey().getHex();
            }

            if (_ethscPrimaryPubKey.empty() && !_seed.empty()) {
                bytes_t seedBytes(_seed);
                HDSeed hdseed(seedBytes);
                HDKeychain rootkey(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));
                _ethscPrimaryPubKey = rootkey.getChild("44'/60'/0'/0/0").uncompressed_pubkey().getHex();
            }

            if (_xPubKeyBitcoin.empty() && !_seed.empty()) {
                Log::info("Regenerate btc masterPubKey from old keystore");
                bytes_t seedBytes(_seed);
                HDSeed hdseed(seedBytes);
                HDKeychain rootkey(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));

                _xPubKeyBitcoin = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());
            }
		}

	}
}
