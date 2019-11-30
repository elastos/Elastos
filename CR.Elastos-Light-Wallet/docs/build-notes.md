# elastos-light-wallet

### to check if your node is working run:

url http://localhost:21333 -d '{"method":"getnodestate"}' -H "Content-Type: application/json"

### if the usb device cannot be detected, try adding the udev rules:
https://support.ledger.com/hc/en-us/articles/115005165269-Connection-issues-with-Windows-or-Linux

```
wget -q -O - https://raw.githubusercontent.com/LedgerHQ/udev-rules/master/add_udev_rules.sh | sudo bash
```


### requirements

```
nodejs v10.11.0 or higher.
npm 6.4.1 or higher.
python 2.7 (for multiple versions, run "npm config set python ${path-to-python2.7}")
```

if windows give you an error "cannot find vcbuild.exe"

```
npm install -g --production windows-build-tools
```

if mac gives you error 'Library not loaded: libintl.8.dylib' when creating the packaged exeutables try:

```
sudo port install gettext;

ln -s /opt/local/lib/libintl.8.dylib /usr/local/opt/gettext/lib/libintl.8.dylib;
```

macOS:
openssl dgst -sha256 <App.dmg>

Windows:
certUtil -hashfile "<App.exe>" SHA256

Linux AppImage:
sha256sum <App.AppImage>


```
Accessories

* npm plus (It is installed inside atom in the packages section)
* python
* microsoft visual studio 2005

To install python it is necessary to run windows CMD as administrator and execute the command [npm install --global --production windows-build-tools]

And run there [npm install node-gyp] to verify if you have installed C ++ if not installed

If you have a problem with visual C ++ you can use this command in the same Windows CMD [npm install mongoose --msvs_version = 2012]
```

To use:
```
Node.js v8.x:
# Using Ubuntu
sudo update-alternatives --remove-all nodejs;
curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -;
sudo apt-get install -y nodejs;

npm install;
npm rebuild usb --update-binary;
npm rebuild node-hid --update-binary;
npm start;
```

To test:
```
npm test;
```

To build a release:
```
npm run dist-mac;
npm run dist-win;
npm run dist-linux;
```

### helpful hash tools:
Hash of a hex message:
echo -n "<hex>" | shasum -a 256

Hash of the binary code inside a hex message:
perl -e 'print pack("H*","<hex>")' | shasum -a 256

### helpful to find stray packages

npm ls node-gyp
