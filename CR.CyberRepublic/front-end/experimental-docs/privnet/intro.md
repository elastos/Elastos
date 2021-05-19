
# Getting Started

Your personal private net is a locally running Core Elastos Ecosystem.

If you don't have access to private Elastos repos this won't work for you, in that case you will
have to rely on the docker container.


## Setting Up Your PrivateNet (PrivNet)

**This includes:**

- ELA Mainchain - cache is `elastos` folder
- ELA Sidechain (DID) - cache is `elastos_did` folder
- ELA Arbiter Nodes (dPoS) - cache is `elastos_arbiter` folder
- IPFS
- PeerPad

!> **PLEASE NOTE:** If you make a mistake, change a magic number or other genesis parameter, you may need to delete the cache folders listed above and start all over again.


## Local Environment or Docker Container

?> You can either set everything up locally or download our docker image which has everything up and running already.

### Setting Up Your Own Environment (OSX)

##### Required Dependencies

- Go@1.9 - `brew install go@1.9`

*For reference this guide is using `go1.9.7 darwin/amd64`*

##### Directory Structure

We would like to keep everything in one root folder, this will all be relative to your `$HOME` variable
- This is aliased as `~`.

Most of Elastos components assumed everything is at `~/dev/src/github.com/elastos`, they've stopped hardcoding that
but you can never be too sure, so I suggest you follow the convention.

These should be set in your `~/.bash_profile`, don't forget to call this after the edits `source ~/.bash_profile`

```
export GOROOT=/usr/local/opt/go@1.9/libexec
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```


?> <span style="font-size: 1.5em;">[Continue to the Setup Steps](/privnet/setup.md)</span>
