# elastos-light-wallet

## using the app

### if the ledger device cannot be detected,try adding the udev rules:
https://support.ledger.com/hc/en-us/articles/115005165269-Connection-issues-with-Windows-or-Linux

```
wget -q -O - https://raw.githubusercontent.com/LedgerHQ/udev-rules/master/add_udev_rules.sh | sudo bash
```

## verifying the executable.

to check the sha256 hashes of the apps run the following commands:

macOS:
openssl sha -sha256 <App.dmg>

Windows:
certUtil -hashfile "<App.exe>" SHA256

Linux AppImage:
sha256sum <App.AppImage>

## building a release

If the usb build fails, it may be because building from source is broken, try
```
npm install node@8.15.0
```

To run in dev mode:
```
npm start;
```

To test:
```
npm test;
```

To build a release:
```
npm run dist-all;
```

then run:
```
openssl sha -sha256 dist/elastos-light-wallet\ Setup\ 1.0.0.exe;
openssl sha -sha256 dist/elastos-light-wallet-1.0.0-x86_64.AppImage;
openssl sha -sha256 dist/elastos-light-wallet-1.0.0.dmg;
```


## setting up the node.

don't forget testnet and mainnet config files are in github at: https://github.com/elastos/Elastos.ELA/blob/master/docs/

## todo:
1. add RSS feed.
    elastos rss feed        
        https://news.elastos.org/feed/

2. add price feed
    price feed
        https://api.coingecko.com/api/v3/simple/price?ids=elastos&vs_currencies=usd
