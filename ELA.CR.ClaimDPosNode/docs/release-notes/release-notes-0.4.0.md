Elastos.ELA version 0.4.0 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.4.0/>

This is a new major version release, include a CR transaction，UTXO cache optimization.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.3.9 and before, you should shut it down and wait until
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

- New CR related transactions, inclouding register,update, unregister and return transcations.
- The first upgrade node carries out the old data migration, and the block stored by the leveldb is transferred to the ffldb.
- Faster processing of transaction.
- Faster processing of block.

0.4.0 change log
=================

### Rollback
- #1273 fix the rollback exception
- #1243 fix rollback error and set default value of EnableUtxoDB

### CR and Prodcuer transcation
- #1314 add duplicate check to ReturnDepositCoin transaction
- #1302 modify to allow URL field of producer and CR be empty
- #1275 add votes output cache for CR state
- #1270 unlimite max vote count of CR type votes
- #1263 fix bugs of release_v0.4.0 about vote_cr_tx.lua and duplicate check of nicknames 
- #1250 process deposit amount when return deposit coin
- #1245 fix bugs of Release v0.4.0 about CRC committee
- #1244 fix check return deposit coin transaction failed error
- #1236 output to the deposit address should be the address of a registered p…
- #1235 change cr voting key from code to did
- #1233 only CRs that have not been previously registered can be registered
- #1227 check duplicate nickname in txpool
- #1212 clean related transactions when the CR is unregistered
- #1207 add unit test for insertReference
- #1185 cr vote add time check and vote equal sort by code hash
- #1178 CR: Fixed candidates and candidateVotes number inconsistent
- #1176 CR: Fixed duplicate records for cr re-registration
- #1162 public key of CR should not exist in crc list
- #1160 check CR with producer public key before append to tx pool
- #1157 fix check if cr code confilict with producer keys failed
- #1261 fix error that get deposit coin failed

### P2P
- #1266 add write and read deadline for peer

### Wallet
- #1258 cli: add utxo lock flags support
- #1251 build the activate txn with default account
- #1156 use the keystore file as a wallet 

### RPC
- #1254 fix error that start rpc server may sometimes panic
- #1241 add rpc interface to get CR deposit coin
- #1226 remove the check of the address in VoteStatus rpc interface

### Database
- #1277 fetch confirmed block from database and block pool
- #1253 fixed database closure can sometimes cause blocking problems 
- #1190 add benchmark test for fflDB

### Block
- #1249 check block exist according to block nodes
- #1239 add status for block node
- #1217 Comment out the logic of recover from checkpoint

### Miscellaneous
- #1222 modify time source and default params
- #1206 fix misjudge the reflect result with nil value
- #1189 Add license
- #1196 add command line args about port and pow related
- #1188 add command args for node app
- #1171 fix error that BadNetWork information may sometimes always print
- #1161 calculate reward when force changed in a new way
- #1158 add flags for FilterLoad message

### Glide
- #1203 Fix Makefile version bug