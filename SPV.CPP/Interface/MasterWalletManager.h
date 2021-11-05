/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_MASTERWALLETMANAGER_H__
#define __ELASTOS_SDK_MASTERWALLETMANAGER_H__

#include <map>

#include "IMasterWalletManager.h"

namespace Elastos {
	namespace ElaWallet {

#define SPV_API_PUBLIC  __attribute__((__visibility__("default")))

        class Config;
		class Lockable;

		class SPV_API_PUBLIC MasterWalletManager : public IMasterWalletManager {
		public:
			/**
			 * Constructor
			 * @param rootPath Specify directory for all config files, including mnemonic config files. Root should not be empty, otherwise will throw invalid argument exception.
			 * @param netType Value can only be one of "MainNet", "TestNet", "RegTest", "PrvNet"
			 * @param dataPath The path contains data of wallet created. If empty, data of wallet will store in rootPath.
			 * @param config If netType is "MainNet" or "TestNet" or "RegTest", config will be ignore.
			 *
			 * Config eg: netType: "MainNet"
			 * {
			 * 	"ELA": { },
			 * 	"IDChain": { },
			 * 	"ETHSC": { "ChainID": 20, "NetworkID": 20 },
			 * 	"ETHDID": { "ChainID": 20, "NetworkID": 20 },
			 * 	"ETHHECO": { "ChainID": 128, "NetworkID": 128 },
			 * 	"BTC": {}
			 * }
			 */
			explicit MasterWalletManager(const std::string &rootPath, const std::string &netType = "MainNet",
										 const nlohmann::json &config = nlohmann::json(),
										 const std::string &dataPath = "");

			virtual ~MasterWalletManager();

			virtual std::string GenerateMnemonic(const std::string &language, int wordCount = 12) const;

			virtual IMasterWallet *CreateMasterWallet(
				const std::string &masterWalletID,
				const std::string &mnemonic,
				const std::string &passphrase,
				const std::string &passwd,
				bool singleAddress);

            virtual IMasterWallet *CreateMasterWallet(
                    const std::string &masterWalletID,
                    const std::string &singlePrivateKey,
                    const std::string &passwd);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletID,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletID,
				const std::string &xprv,
				const std::string &payPassword,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual IMasterWallet *CreateMultiSignMasterWallet(
				const std::string &masterWalletID,
				const std::string &mnemonic,
				const std::string &passphrase,
				const std::string &payPassword,
				const nlohmann::json &cosigners,
				uint32_t m,
				bool singleAddress,
				bool compatible = false,
				time_t timestamp = 0);

			virtual std::vector<IMasterWallet *> GetAllMasterWallets() const;

			virtual std::vector<std::string> GetAllMasterWalletID() const;

			virtual bool WalletLoaded(const std::string &masterWalletID) const;

			virtual IMasterWallet *GetMasterWallet(
				const std::string &masterWalletID) const;

			virtual void DestroyWallet(
				const std::string &masterWalletID);

			virtual IMasterWallet *ImportWalletWithKeystore(
				const std::string &masterWalletID,
				const nlohmann::json &keystoreContent,
				const std::string &backupPassword,
				const std::string &payPassword);

			virtual IMasterWallet *ImportWalletWithMnemonic(
				const std::string &masterWalletID,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress,
				time_t timestamp = 0);

			virtual std::string GetVersion() const;

			virtual void FlushData();

			virtual void SetLogLevel(const std::string &level);

		protected:
			typedef std::map<std::string, IMasterWallet *> MasterWalletMap;

			void LoadMasterWalletID();

			IMasterWallet *LoadMasterWallet(const std::string &masterWalletID) const;

			void checkRedundant(IMasterWallet *wallet) const;

		protected:
			Lockable *_lock;
			Config *_config;
			std::string _rootPath;
			std::string _dataPath;
			mutable MasterWalletMap _masterWalletMap;
		};
	}
}

#endif //__ELASTOS_SDK_MASTERWALLETMANAGER_H__
