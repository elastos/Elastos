Elastos.ELA version 0.4.3 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.4.3/>

This is a new minor version release, including bugfixes. 

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.4.2 and before, you should shut it down and wait until
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

Fixed an issue that block message size validation does not match block size validation, which could cause some node is not synchronized.

0.4.3 change log
=================

Detailed release notes follow

- #1476 Separate blocks size check to header and context checks