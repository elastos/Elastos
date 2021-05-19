Elastos.ELA version 0.3.8 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.3.8/>

This is a new minor version release, fix rollback failed error, compatible with release_v0.3.7 version.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.3.7 and before, you should shut it down and wait until
 it has completely closed, then just copy over `ela` (on Linux).

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

Solved the rollback caused the block height does not grow.

0.3.8 change log
=================

Detailed release notes follow

- #1184 `2c62d92` fix error that zero amount of output may causes failed to rollback UTXO
- #1183 `f2ceac1` fix dpos state rollback error when there is a fork at the first block of dpos rounds
- #1179 `5ff823c` fix error that rollback may causes panic in processing votes