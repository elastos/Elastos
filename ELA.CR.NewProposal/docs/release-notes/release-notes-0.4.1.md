Elastos.ELA version 0.4.1 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.4.1/>

This is a new minor version release, fix some nodes out of sync. 

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.4.0 and before, you should shut it down and wait until
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

Solved the problem that some nodes are not synchronized.

0.4.1 change log
=================

Detailed release notes follow

- #1353 `4ea94e5` display height when querying CR candidate information
- #1347 `1592d35` display as address when acquiring output payload information of CR
- #1344 `0616e5d` fixed block synchronization sometimes got stuck error