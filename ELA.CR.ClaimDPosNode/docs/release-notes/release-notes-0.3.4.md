Elastos.ELA version 0.3.4 is now available from:

  <https://www.elastos.org/bin/elastos-ela-0.3.4/>

This is a new minor version release, including new features, various bug
fixes and performance improvements.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has
completely shut down, then just copy over `ela` (on Linux).

Note that the illegal node can activate the node by sending an active
transaction in version 0.3.4. Upgrading directly from 0.3.x before the
block height 439000.

However, as usual, config, keystore and chaindata files are compatible.

Compatibility
==============

Elastos.ELA is supported and extensively tested on operating systems
using the Linux kernel. It is not recommended to use Elastos.ELA on
unsupported systems.

Elastos.ELA should also work on most other Unix-like systems but is not
as frequently tested on them.

As previously-supported CPU platforms, this release's pre-compiled
distribution provides binaries for the x86_64 platform.

Notable changes
===============

Documentation
-------------

- The [JSON-RPC documentation](https://github.com/elastos/Elastos.ELA/blob/master/docs/jsonrpc_apis.md)
  documentation has been updated with information about new RPC
  `getmininginfo` for getting extra information related to mining.

New RPCs
--------

- `getmininginfo` returns the extra information related to mining, such as
  difficulty, hash rate, etc.

0.3.4 change log
=================

### Block and transaction handling

- #1076 Check block context before appending to blockpool
- #1083 Change illegal producer related strategy
- #1111 Fix error that arbiter may send accept and reject vote of same proposal

### RPC and other APIs

- #1084 Add `getmininginfo`

### Tests and QA

- #1085 Use goroutine to deal with dpos network message
- #1126 Fix lock in ForceChange function may lead to dead lock error

### Miscellaneous

- #1077 Add log to show block infomation when check failed
- #1112 Change Build Status to travis.com
- #1105 Update glide.yaml
- #1101 Travis: remove auto-deployment code

