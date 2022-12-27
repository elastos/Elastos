### testnet and mainnet locations.

  they are both on elastos.coranos.io

### to build:

  cd ~/dev/src/github.com/elastos/Elastos.ELA;  
  export GOPATH=$HOME/dev;
  export GOBIN=$GOPATH/bin;
  export PATH=$GOROOT/bin:$PATH;
  export PATH=$GOBIN:$PATH;
  go version;
  glide --version;
  git pull;
  glide update;
  glide install;
  make;
  cp ela ../mainnet/ela-mainnet;
  cp ela ../testnet/ela-testnet;

### to smoke test:

  verify block height is the same:
  <http://coranos.cc:20334/api/v1/node/state>
  vs
  <https://blockchain.elastos.org/blocks>

  verify "neighbors" exists and has values:
  <http://coranos.cc:20334/api/v1/node/state>

### to start:

  cd ~/dev/src/github.com/elastos/mainnet
  screen -dmS ela-mainnet ./ela-mainnet

  cd ~/dev/src/github.com/elastos/testnet/
  screen -dmS ela-testnet ./ela-testnet
