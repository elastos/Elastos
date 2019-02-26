

# ELA Client - `SHA 287e7aef`

?> You can find this at - [https://github.com/elastos/Elastos.ELA.Client](https://github.com/elastos/Elastos.ELA.Client)

The `README.md` works well, just remember to call `glide cc` before you call `glide update && glide install` to clear the cache in case you had an old build.


### How to Use It

The `wallet.db` stores the addresses and public keys, whereas the `keystore-*.dat` files hold the private keys.
The two function independently from each other, in that importing a private key does not automatically add the wallet to the `wallet.db`.

**So when you type `./ela-cli wallet -l` it doesn't actually list anything initially.**

When you import a wallet via the private key with the `--import privateKey` command, all this does is it encrypts the private key into the keystore file,
which by default is `keystore.dat`. Each private key will need a new key store so ensure you name them properly.

- To add the wallet address to the `wallet.db` you have to use `--addaccount value` command with the public key.

- Similarly use `--delaccount value` to remove an address

- When signing transactions, you can use a keystore by using the `-n` flag and entering your password for that keystore.

The rest of the info in the `README.md` should be self-explanatory.
