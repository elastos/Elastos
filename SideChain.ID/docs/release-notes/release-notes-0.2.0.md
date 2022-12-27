Elastos.ELA.SideChain.ID version 0.2.0 is now available from:

  <https://download.elastos.org/elastos-did/elastos-did-v0.2.0/>

This is a new minor version release, add conflict slot of register did.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA.SideChain.ID/issues>

How to Upgrade
==============

If you are running version release_v0.1.5 and before, you should shut it down and wait until
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

Support CR council member claim DPOS node.

0.2.0 change log
=================

Detailed release notes follow
 
- #158 Modify to return error message of function SideAuxPowCheck
- #156 Change reward in coinbase by the height of main chain
- #154 Only reward miner after CRClaimDPOSNodeStartHeight
- #152 Add prefix of did node version
- #148 Add NewP2PProtocolVersionHeight and pass NodeVersion to SPV module
- #147 Pass node version to side chain
- #146 Add CRClaimDPOSNodeStartHeight into config
- #136 Transactions of one block are commited in one transaction
- #131 Update go.mod & register func for spv block listener
- #130 Use go mod dependency management system and FTNexTTurnDPOSInfo instead of FTBloom 
