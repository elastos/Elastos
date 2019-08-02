Elastos.ELA version 0.3.5 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-0.3.5/>

This release provides a minor bug fix for 0.3.4.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has
completely shut down, then just copy over `ela` (on Linux).

Note that the node will check the block's context more strictly in
version `0.3.5`. Upgrading directly from 0.3.x before the block height 436812.

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

An issue was solved which could cause the block context check to fail.

0.3.5 change log
=================

### Block and transaction handling

- #1152 Fix reward processing error
