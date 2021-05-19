---
title: Your First Smart Contract (on Elastos ETH Sidechain)
sidebar_label: Your First Smart Contract
---

### Smart contract code

Go to [Remix website](https://remix.ethereum.org):
![remix_browser](assets/remix_browser.png)

Create a new file **SimpleStorage.sol**, then copy & paste the following code into the file:

```solidity
pragma solidity 0.5.1;

contract SimpleStorage {
  uint storedData;

  function set(uint x) public {
    storedData = x;
  }

  function get() public view returns (uint) {
    return storedData;
  }
}
```

What the contract code does is very simple, it has a variable `storedData` and 2 functions `set` and `get` to set and get the value of `storedData`.

### Compile

![remix_compile](assets/remix_compile.png)

Switch to compile tab to compile the code, make sure the compiler version is the same as the version in the code, which is **0.5.1** in our code.

Click **Compile SimpleStorage.sol** button to start compile, it should be successful in seconds.

### Deploy

![remix_deploy](assets/remix_deploy.png)

After compile successfully, we are going to deploy the smart contract. Switch to **Deploy** tab and you will set the deploy interface.

Select Environment and choose **Injected Web3** to utilize metamask injected capability, you should be able to see the account info soon.

Click **Deploy** button to start deploy the smart contract to testnet.

![remix_deploy_metamask](assets/remix_deploy_metamask.png)

Metamask will be brought up and ask you confirm **gas fee**, the number is calculated automatically, so we just click **Confirm** button.

![remix_deploy_success](assets/remix_deploy_success.png)

If the contract creation transaction is confirmed by testnet, you should be able to see the contract info under **Depolyed Contracts** section. Feel free to interact with the contract function and check the value.

If you have followed here, then congratulate you have deployed the first smart contract onto Elastos Ethereum sidechain.
