// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdexcept>
#include <Core/BRBIP32Sequence.h>
#include <SDK/Common/Utils.h>

#include "AddressCache.h"
#include "Key.h"

namespace Elastos {
	namespace ElaWallet {

		AddressCache::AddressCache(const MasterPrivKey &masterPrivKey, DatabaseManager *databaseManager,
								   uint32_t internalCacheSize, uint32_t externalCacheSize) :
				_masterPrivKey(masterPrivKey),
				_databaseManager(databaseManager),
				_internalStartIndex(0),
				_externalStartIndex(0),
				_internalCacheSize(internalCacheSize),
				_externalCacheSize(externalCacheSize) {
		}

		AddressCache::~AddressCache() {

		}

		std::vector<std::string> AddressCache::FetchAddresses(size_t size, bool external) {
			if ((external && size > _databaseManager->getExternalAvailableAddresses(_externalStartIndex)) ||
				(!external && size > _databaseManager->getInternalAvailableAddresses(_internalStartIndex)))
				throw std::logic_error("Insufficient cache.");

			return external ? _databaseManager->getExternalAddresses(_externalStartIndex, size)
							: _databaseManager->getInternalAddresses(_internalStartIndex, size);
		}

		void AddressCache::Reset(const std::string &payPassword, uint32_t startIndex, bool external) {
			uint32_t availableSize = 0;
			uint32_t expandSize = 0;

			Key key;
			CMBlock keyData = Utils::decrypt(_masterPrivKey.GetEncryptedKey(), payPassword);
			char stmp[keyData.GetSize()];
			memcpy(stmp, keyData, keyData.GetSize());
			std::string secret(stmp, keyData.GetSize());
			key.setPrivKey(secret);

			if (external) {

				_externalStartIndex = startIndex;
				availableSize = _databaseManager->getExternalAvailableAddresses(_externalStartIndex);
				if (availableSize >= _externalCacheSize)
					return;

				expandSize = _externalCacheSize - availableSize;
				uint32_t indices[expandSize];
				for (uint32_t i = 0; i < expandSize; ++i) {
					indices[i] = _externalStartIndex + i;
				}

				BRKey keys[expandSize];
				UInt256 secreteInt = key.getSecret();
				Key::calculatePrivateKeyList(keys, expandSize, &secreteInt,
											 const_cast<UInt256 *>(&_masterPrivKey.GetChainCode()),
											 SEQUENCE_EXTERNAL_CHAIN, indices);
				std::vector<std::string> newAddresses;
				for (size_t i = 0; i < expandSize; ++i) {
					//todo potential performance hot pot, fix me in future
					Key temp(keys[i]);
					newAddresses.push_back(temp.keyToAddress(ELA_STANDARD));
				}
				_databaseManager->putExternalAddresses(availableSize + _externalStartIndex, newAddresses);
			} else {

				_internalStartIndex = startIndex;
				availableSize = _databaseManager->getInternalAvailableAddresses(_internalStartIndex);
				if (availableSize >= _internalCacheSize)
					return;

				expandSize = _internalCacheSize - availableSize;
				uint32_t indices[expandSize];
				for (uint32_t i = 0; i < expandSize; ++i) {
					indices[i] = _internalStartIndex + i;
				}

				BRKey keys[expandSize];
				UInt256 secreteInt = key.getSecret();
				Key::calculatePrivateKeyList(keys, expandSize, &secreteInt,
											 const_cast<UInt256 *>(&_masterPrivKey.GetChainCode()),
											 SEQUENCE_INTERNAL_CHAIN, indices);
				std::vector<std::string> newAddresses;
				for (size_t i = 0; i < expandSize; ++i) {
					//todo potential performance hot pot, fix me in future
					Key temp(keys[i]);
					newAddresses.push_back(temp.keyToAddress(ELA_STANDARD));
				}
				_databaseManager->putInternalAddresses(availableSize + _internalStartIndex, newAddresses);
			}
		}

	}
}