Elastos.ELA version 0.3.6 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.3.6/>

This release fix an error that DPOS node can not confirm block.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running an older version, you should shut it down and wait until
 it has completely closed, then just copy over `ela` (on Linux).

Note that the DPOS node will have no timeout when change view occurred
in version `0.3.6`.

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

An issue was solved which DPOS node can not confirm block.

0.3.6 change log
=================

### Block and transaction handling

- #1164 Change dpos inactive strategy
