# Elastos SPV C++

- [Summary](#summary)
- [Getting started](#getting-started)
  - [Initialize](#Initialize)
  - [Create master wallet](#create-master-wallet)
  - [Create sub wallet](#create-sub-wallet)
  - [Sub wallet](#sub-wallet)
  - [Main chain sub wallet](#main-chain-sub-wallet)
  - [ID chain sub wallet](#id-chain-sub-wallet)
  - [Side chain sub wallet](#side-chain-sub-wallet)
  - [Sign and publish transaction](#sign-and-publish-transaction)
  - [Remove master wallet](#remove-master-wallet)
  - [Deinitialize](#deinitialize)
- [Build guide](#build-guide)
- [Development](#development)
- [More](#more)

## Summary

This repository is a basic library aimed to provide a serials of wallet related interfaces for anyone who want to build a wallet themselves.

## Getting started

#### Initialize

Copy all files(CoinConfig.json, mnemonic_chinese.txt mnemonic_*.txt) from `Data` directory of source code to the `rootPath`, and then create an instance of **MasterWalletManager**

```c++
const std::string rootPath = "./Data"
try {
    MasterWalletManager *manager = new MasterWalletManager(rootPath);
    if (manager == nullptr) {
        // fail process
    }
} catch (...) {
	// exception process
}
```

Get created **MasterWallet** and **SubWallet** lists

```c++
try {
    std::vector<IMasterWallet *> masterWallets = manager->GetAllMasterWallets();
    if (masterWallets.size() == 0) {
        // see create master wallet below
    } else {
        for (size_t i = 0; i < masterWallets.size(); ++i) {
            std::vector<ISubWalelt *> subWallets = masterWallets[i]->GetAllSubWallets();
            // see transaction operation below
        }
    }
} catch (...) {
    // exception process
}
```

#### Create master wallet

If **MasterWallet** lists are empty, then create one as need

Create standard HD master wallet with mnemonic

```c++
const std::string walletID = "HDWalletID";
const std::string phrasePassword = "";
const std::string payPassword = "Pay_Password~123^456";
const bool singleAddress = false;
try {
    const std::string mnemonic = manager->GenerateMnemonic("english");
    IMasterWallet *masterWallet = manager->CreateMasterWallet(
        walletID, mnemonic, phrasePassword, payPassword, singleAddress);
} catch (...) {
    // exception process
}
```

Create 3/4 multi sign master wallet with mnemonic

```c++
// ...
const nlohmann::json coSigners = nlohmann::json::parse(
		"[\"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5\","
		" \"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D\","
		" \"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA\"]");
uint32_t requiredSignCount = 3;

try {
	// ...
    IMasterWallet *masterWallet = manager->CreateMultiSignMasterWallet(
        walletID, mnemonic, phrasePassword, payPassword, coSigners, requiredSignCount);
} catch (...) {
    // exception process
}
```

Create 3/4 readonly multi sign master wallet

```c++
// ...
nlohmann::json coSigners = nlohmann::json::parse(
		"[\"03FC8B9408A7C5AE6F8109BA97CE5429C1CD1F09C0655E4EC05FC0649754E4FB6C\","
		" \"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5\","
		" \"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D\","
		" \"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA\"]");
uint32_t requiredSignCount = 3;

try {
    // ...
    IMasterWallet *masterWallet = manager->CreateMultiSignMasterWallet(
        walletID, coSigners, requiredSignCount);
} catch (...) {
    // exception process
}
```

#### Create sub wallet

Get all supported sub wallet ID

```c++
// ...
try {
    std::vector<std::string> subWalletIDList = masterWallet->GetSupportedChains();
} catch (...) {
    // exception process
}
```

Create a subwallet

```c++
uint64_t feePerKB = 10000;
try {
    ISubWallet *subWallet = masterWallet->CreateSubWallet(subWalletIDList[0], feePerKB);
} catch (...) {
    // exception process
}
```

Register callback for sub wallet

```c++
class SubWalletCallback: public ISubWalletCallback {
    public:
        ~SubWalletCallback() {}
        SubWalletCallback() {}

        virtual void OnTransactionStatusChanged(
            const std::string &txid,const std::string &status,
            const nlohmann::json &desc,uint32_t confirms) {
            std::cout << "OnTransactionStatusChanged -> " << std::endl;
        }
        virtual void OnBlockSyncStarted() {
            std::cout << "OnBlockSyncStarted" << std::endl;
        }
        virtual void OnBlockHeightIncreased(uint32_t currentBlockHeight, int progress) {
            std::cout << "OnBlockHeightIncreased -> " << std::endl;
        }
        virtual void OnBlockSyncStopped() {
            std::cout << "OnBlockSyncStopped" << std::endl;
        }
        virtual void OnBalanceChanged(uint64_t balance) {
            std::cout << "OnBalanceChanged -> " << std::endl;
        }
}

try {
    SubWalletCallback *callback = new SubWalletCallback();
    subWallet->AddCallback(callback);
} catch (...) {
    // exception process
}
```

Remove callback from sub wallet

```c++
// ...
try {
    subWallet->RemoveCallback(callback);
} catch (...) {
    // exception process
}
```

#### Sub wallet

Create normal transfer asset transaction

```c++
const std::string fromAddress = "";
const std::string toAddress = "EYMVuGs1FscpgmghSzg243R6PzPiszrgj7";
uint64_t amount = 100000000; // 1 ELA
const std::string memo = "Hello elastos";
const std::string remark = "Hello SPV"
try {
    nlohmann::json tx = subWallet->CreateTransaction(
        fromAddress, toAddress, amount, memo, remark);
} catch (...) {
    // exception process
}
```

Create multi sign transaction

```c++
try {
    nlohmann::json tx = subWallet->CreateMultiSignTransaction(
        fromAddress, toAddress, amount, memo);
} catch (...) {
    // exception process
}
```

#### Main chain sub wallet

Deposit 1 ELA to ID chain

```c++
// get genesis address of ID chain
std::string toAddress;
try {
    IIdChainSubWallet *IDSubWallet = dynamic_cast<IIdChainSubWallet *>(subWallet);
    if (nullptr == IDSubWallet) {
        // subWallet is not instance of IIdChainSubWallet
    }
    toAddress = IDSubWallet->GetGenesisAddress();
} catch (...) {
    // exception process
}

// ...
uint64_t amount = 100000000;
// side chain message
nlohmann::json addressList, amountList, outputIndexList;
addresses.push_back("EUjtxVuLk3vA1VSqN3EKiK4dY5u7izJQWi");
amountList.push_back(amount);
outputIndexList.push_back(0); // the first output use for deposit
// ...
try {
	IMainchainSubWallet *mcSubWallet = dynamic_cast<IMainchainSubWallet *>(subWallet);
    if (nullptr == mcSubWallet) {
        // subWallet is not instance of IMainchainSubWallet
        // do something
    }
    nlohmann::json tx = mcSubWallet->CreateDepositTransaction(
        fromAddress, toAddress, amount, addresses,
        amountList, outputIndexList, memo, remark);
} catch (...) {
    // exception process
}
```

#### ID chain sub wallet

Create register ID transaction

```c++
// ...
const nlohmann::json payload = "...";
const nlohmann::json program = "...";
try {
    IIdChainSubWallet *IDSubWallet = dynamic_cast<IIdChainSubWallet *>(subWallet);
    if (nullptr == IDSubWallet) {
        // subWallet is not instance of IIdChainSubWallet
    }
    nlohmann::json tx = IDSubWallet->CreateIdTransaction(
        fromAddress, payload, program, memo, remark);
} catch (...) {
    // exception process
}
```

#### Side chain sub wallet

Withdraw 1 ELA from ID chain

```c++
// ...
uint64_t amount = 100000000;
// main chain message
nlohmann::json addressList, amountList, outputIndexList;
addressList.push_back("EYMVuGs1FscpgmghSzg243R6PzPiszrgj7");
amountList.push_back(amount);
outputIndexList.push_back(0);
try {
    ISidechainSubWallet *SideSubWallet = dynamic_cast<ISidechainSubWallet *>(subWallet);
    if (nullptr == SideSubWallet) {
        // subWallet is not instance of ISidechainSubWallet
    }
    nlohmann::json tx = SideSubWallet->CreateWithdrawTransaction(
        fromAddress, amount, addressList, amountList, outputIndexList);
} catch (...) {
    // exception process
}
```

#### Sign and publish transaction

Calculate `tx` fee

```c++
uint64_t feePerKB = 10000;
try {
    uint64_t fee = subWallet->CalculateTransactionFee(tx, feePerKB);
} catch (...) {
    // exception process
}
```

Update `tx` fee

```c++
try {
    nlohmann::json tx = subWallet->UpdateTransactionFee(tx, fee);
} catch (...) {
    // exception process
}
```

Sign `tx`

```c++
// ...
try {
    nlohmann::json tx = subWallet->SignTransaction(tx, payPassword);
} catch (...) {
    // exception process
}
```

If `tx` is multi sign transaction, we can check signed signers

```c++
// ...
try {
    nlohmann::json signedSigners = subWallet->GetTransactionSignedSigners(tx);
    std::cout << "Signed signers: " << signedSigners.dump() << std::endl;
} catch (...) {
    // exception process
}
```

After signed `tx`, it's time to publish `tx` now

```c++
// ...
try {
    nlohmann::json result = subWallet->PublishTransaction(tx);
    std::cout << "Published tx result: " << result.dump() << std::endl;
} catch (...) {
    // exception process
}
```

#### Remove master wallet

Destroy a master wallet created before, and remove local storage

```c++
try {
    manager->DestroyWallet(walletID);
} catch (...) {
    // exception process
}
```

#### Deinitialize

Delete instance **MasterWalletManager** before program exit

```c++
delete manager;
```




## Build guide

Make sure your computer have installed the required packages below:

- [git](https://www.git-scm.com/downloads)
- [cmake](https://cmake.org/download)
- [wget](https://www.gnu.org/software/wget)
- [xcode](https://developer.apple.com/xcode/download)  (for IOS or MacOS)
- [ndk](https://developer.android.com/ndk/downloads/)  (for Android)

### Prepare source code
```shell
$ cd /home/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update --force --recursive
```

### Build on Ubuntu/MacOS
```shell
$ cd /home/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build
$ cd cmake-build
$ cmake ..
$ make -j 8
```

### Build for IOS
1. Architecture armv7 and arm64.

   ```shell
    $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
    $ mkdir cmake-build-ios
    $ cd cmake-build-ios
    $ cmake -DSPV_PLATFORM=IOS -DCMAKE_INSTALL_PREFIX=$SOMEWHERE/spvsdk/ios/arm ..
    $ make -j 8 && make install
   ```

2. Simulator

   x86

   ```shell
   $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
   $ mkdir cmake-build-simulator
   $ cd cmake-build-simulator
   $ cmake -DSPV_PLATFORM=IOS -DIOS_PLATFORM=SIMULATOR -DCMAKE_INSTALL_PREFIX=$SOMEWHERE/spvsdk/ios/simulator ..
   $ make -j 8 && make install
   ```
   x86_64
   ```shell
   $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
   $ mkdir cmake-build-simulator64
   $ cd cmake-build-simulator64
   $ cmake -DSPV_PLATFORM=IOS -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_INSTALL_PREFIX=$SOMEWHERE/spvsdk/ios/simulator64 ..
   $ make -j 8 && make install
   ```

Note: If built successfully, you will see output static library in directory *cmake-build-ios/lib/* or in installed directory, which combined with all dependent static libraries(libsqlite.a libboost_*.a libcrypto.a libssl.a libbigint.a). Support minimum IOS target version is 10.0

### Build for Android

`NDK` version: r16+

Unzip to somewhere, for example */Users/xxx/dev/android-ndk-r16*

Set system environment variable **ANDROID_NDK** to */Users/xxx/dev/android-ndk-r16*

Support architecture **arm64-v8a** **armeabi-v7a** and simulator( **x86** **x86_64** )

Set **CMAKE_ANDROID_ARCH_ABI** properly to fit your need

```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk
$ cd cmake-build-ndk
$ cmake -DSPV_PLATFORM=Android -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a -DCMAKE_INSTALL_PREFIX=$SOMEWHERE/spvsdk/android/armeabi-v7a ..
$ make -j 8 && make install
```

## Development
Patches are welcome. Please submit pull requests against the **dev** branch.


## More

Learn more about this repository please refer to the following links:
- [interfaces document](https://raindust.github.io/Elastos.ELA.SPV.Cpp.Document/)
- [wiki](https://github.com/elastos/Elastos.ELA.SPV.Cpp/wiki)

