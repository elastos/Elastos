Elastos.ELA version 0.3.9 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.3.9/>

This is a new minor version release, fix rollback failed error, compatible with release_v0.3.8 version.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.3.8 and before, you should shut it down and wait until
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

0.3.9 change log
=================

Detailed release notes follow

- #1290 `9ff97b1` fix vote statistics error
- #1292 `a0a0221` add write and read deadline for peer