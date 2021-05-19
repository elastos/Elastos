Elastos.ELA version 0.4.2 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.4.2/>

This is a new minor version release, support registering CR by CID and DID. 

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.4.1 and before, you should shut it down and wait until
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

Support registering CR by CID and DID.

0.4.2 change log
=================

Detailed release notes follow

- #1393 modify RegisterCRByDIDHeight of mainNet
- #1381 modify to support registering CR by CID and DID