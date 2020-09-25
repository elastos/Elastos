Elastos.ELA.SideChain.ID version 0.1.5 is now available from:

  <https://download.elastos.org/elastos-did/elastos-did-v0.1.5/>

This is a new minor version release, add conflict slot of register did.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA.SideChain.ID/issues>

How to Upgrade
==============

If you are running version release_v0.1.4 and before, you should shut it down and wait until
 it has completely closed, then just copy over `did` (on Linux).

However, as usual, config, keystore and chaindata files are compatible.

Compatibility
==============

Elastos.ELA.SideChain.ID is supported and extensively tested on operating systems
using the Linux kernel. It is not recommended to use Elastos.ELA.SideChain.ID on
unsupported systems.

Elastos.ELA.SideChain.ID should also work on most other Unix-like systems but is not
as frequently tested on them.

As previously-supported CPU platforms, this release's pre-compiled
distribution provides binaries for the x86_64 platform.

Notable changes
===============

Solve the problem of repeatable register did

0.1.5 change log
=================

Detailed release notes follow
 
- #127 Use real DID not old CID
- #126 add conflict slot of register did