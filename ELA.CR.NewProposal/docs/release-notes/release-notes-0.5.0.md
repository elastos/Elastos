Elastos.ELA version 0.5.0 is now available from:

  <https://download.elastos.org/elastos-ela/elastos-ela-v0.5.0/>

This is a new major version release, support CR proposal. 

Please report bugs using the issue tracker at GitHub:

  <https://github.com/elastos/Elastos.ELA/issues>

How to Upgrade
==============

If you are running version release_v0.4.3 and before, you should shut it down and wait until
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

Support CR proposal.
Improve block processing efficiency.

0.5.0 change log
=================

### Rollback
- #1428 Fix issue that rollback block failed
- #1389 fix an issue that rollback tool cannot work normally
- #1372 fixed CR related transaction checker error
- #1368 add uint test for rollback multiple types of transactions
- #1367 Add rollback test case for crc multiple types of txs
- #1361 Add rollback test case for CRCImpeachment and CRCAppropriation tx
- #1351 add rollback uint test and change max size of block header
- #1350 add rollback uint test sample
- #1346 use history to handle rollback 

### CR and producer transaction
- #1499 Set default value of CRCommitteeStartHeight for testnet
- #1498 Returns an error when querying confirmation of one block but there is no confirmation in the block
- #1496 Serialize ProposalTrackingType before check signature of Secretary-General
- #1495 Set default value of CRCommitteeStartHeight for mainnet
- #1494 Fixed an issue that reverse proposal hash failed
- #1491 Reverse proposal hash in output payload of CR proposal
- #1490 Modify to not show status in transaction payload information
- #1486 Change the default value of max block size
- #1485 Modify to show recipient of CR proposal by address
- #1483 Add error return to function getDIDFromCode and getCode
- #1482 The budgets of Proposed proposal txns in the pool should be counted
- #1480 Fixed an issue that may lead to deadlock
- #1478 Fixed some issues of CR impeachment
- #1476 Separate blocks size check to header and context checks
- #1475 Modify to change from Canceled to Returned by CRDepositLockupBlocks
- #1473 Serialize CRCProposal payload by type of proposal
- #1472 The amount of the session needs to be equal to the CR Expenses
- #1471 Fixed an issue that voting period ended abnormally
- #1470 Set default SecretaryGeneral and CRCommitteeStartHeight for regnet
- #1469 Use the current block height to check whether the proposaled tx is legal
- #1468 Budgets of CR proposal need to less than the currently available amount
- #1467 Lock the snapshort procedure of txPoolCheckpoint
- #1466 Only record the locked amount of coinbase transaction
- #1465 Modify to record appropriation amount correctly
- #1464 Appropriation tx qutoa appropriation amount
- #1463 Modify the check of CategoryData of proposal
- #1462 Just use the UTXO of CRCAssetsAddress to create appropriation tx
- #1461 Fixed an issue that sometimes the node cannot be closed properly
- #1460 Limit Impeachment and Proposal voting to a maximum of 36 candidates
- #1459 Add CheckVoteCRCountHeight to params
- #1456 Panic when failed to read log files from the folder
- #1455 Add default value of PublicDPOSHeight for regnet
- #1453 Add default value of CRCommitteeStartHeight and SecretaryGeneral
- #1452 Modify to vote CR impeachment by CID
- #1451 Only unfinished budget need to be deducted from CRCCommitteeUsedAmount
- #1450 Modify to record proposals by session
- #1446 Proposals with Terminated status are allowed to make withdrawals 
- #1444 Modify to check vote CR candidates by CID
- #1443 Make some adjustments and fix some issues
- #1441 Optimize CRC proposal related field naming
- #1439 Modify to only allow to vote existing CR or producer
- #1438 CRCAppropriation tx can not send bug fix
- #1437 Fix issues about rollback and vote statistics
- #1436 Recently bug fix vote cr remove b.crCommittee.IsInVotingPeriod and so on
- #1435 Fix issue that conflict slot check failed
- #1434 Delete duplicate activateCandidateFromPending from process block
- #1433 Modify to allow finalizing proposal before all stage finished
- #1432 Adjust CRC related transaction structure
- #1430 Fix an issue where the penalty was miscalculated
- #1429 Fix some issues of CRC
- #1427 Process transactions with inputs of CRCImpeachment
- #1423 fix issue that failed to distribute dpos reward after CRCommitteeStartHeight
- #1422 Fixed configuration files MaxCommitteeProposalCount position
- #1420 Fix issue that check vote CRC proposal transaction failed
- #1417 modify the check of CR impeachment and appropriation transaction
- #1415 modify to check proposal not only by voting period
- #1414 fix issue that deposit coin is not correct
- #1413 cr impeachment change fromd did to cid
- #1408 fix issue that deposit amount is not correct
- #1407 fix issue that CRC reward of CR dpos node is not correct
- #1406 can't register proposal in voting period
- #1404 Check Proposal Withdraw transaction must be CRC committee address
- #1403 set default value of CRC assets address and CRC council expenses address
- #1400 fix issues with CR related
- #1399 Fix issue that the deposit amount of impeached CR is not correct
- #1398 Remove fee from payload CRCProposalWithdraw
- #1395 Types of Budgets and stage inspection
- #1388 Processing transactions not distinguishes voting period and election period
- #1386 fix issue that synchronization problem occurred after killing node
- #1384 a single CRC proposal can only use ten percent of total ELA of the current stage
- #1380 modify getPayloadInfo function to support CR related payload
- #1378 fix max peers check and create CRC proposal failed error 
- #1377 check public key when creating contract and redeem script
- #1363 Votes to eliminate validity checks on candidates
- #1362 add test cases of producer
- #1349 Adjust the proposal tracking logic 
- #1345 fix error that MaxTxPerBlock can't work in config file
- #1342 Adjust the proposal type to include a category data field
- #1335 reset next arbiter by crc ignore LastCommitteeHeight
- #1332 fix error that vote producer failed
- #1331 Fixed crSig error signature when key is empty.
- #1329 Add opinion hash on the transaction for crproposal
- #1327 The CRDepositLockupBlocks parameter ported to config file
- #1323 add crc_proposal_withdraw.lua script
- #1320 fix error of processing CRC impeachment transaction
- #1318 modify lock of committee and state
- #1313 Bugfix: add the change of the withdrawal address parameter of the sponsor
- #1308 Add opinion hash on the transaction of the proposal tracking
- #1301 adjust data structure of history member and check of CR related transaction
- #1300 Fixed unable to create special transactions
- #1299 Remove raw data in proposal and tracking proposal txs
- #1298 deal with the history of CR candidates and members
- #1293 Allow proposals to track other states that can be withdraw
- #1289 let multi ouput lua script support parameters with multi amounts 
- #1286 let CRC appropriation transaction pass the normal test
- #1285 modify CR reward distribution
- #1284 add verification of CRC proposal
- #1282 Complete CRC proposal appropriate transaction related logic
- #1280 Complete CRC proposal withdraw transaction related logic 
- #1279 add error code and interface to unite all errors underground
- #1276 add CR appropriation transaction
- #1272 add proposal data into CRC proposal and proposal tracking payload
- #1269 fix error that decrease CR proposal votes failed
- #1266 add CR proposal tracking transaction
- #1265 transaction indexer support
- #1264 fix error that create vote content failed
- #1260 Add cr case framework for ci 
- #1259 The number of committee proposals is limited to 128 
- #1248 rollback block node for bench test
- #1240 ditribute crc dpos reward to corresponding cr members
- #1237 Complete CRC proposal review transaction related logic
- #1236 output to the deposit address should be the address of a registered producer
- #1233 only CRs that have not been previously registered can be registered
- #1232 update testnet config to have testnet magic
- #1219 start voting period when CR members if not enough
- #1218 Cr vote proposal impeach 
- #1216 check related CR txs when adding txs to the tx pool
- #1213 add basic proposal status management logic 
- #1212 clean related transactions when the CR is unregistered
- #1211 support set keystore file and password for lua script
- #1209 add CRC proposal

### P2P
- #1373 Limit the maximum number of connections for per host
- #1365 check address list every two hours
- #1364 filter unwanted addresses when storing them into peers.json
- #1278 add timeout to sync peer 

### Wallet
- #1230 move logic of wallet from common.lua to other lua script
- #1299 open account creating and custom wallet path 

### RPC
- #1426 Modify to show vote result by string
- #1425 Removes the VoteStartHeight field from the ListCRProposalBaseState interface
- #1401 fix issues that information of RPC display incomplete
- #1360 add config about RPC service level and node profile strategy
- #1339 Rpc add listsecretary and fixed member refundable status
- #1306 put deposit related amount into struct DepositInfo
- #1297 fix error of blocking synchronization block
- #1268 RPC: add listcrproposalbasestate and getcrproposalstate interface 
- #1262 rename return data structs about RPC 

### Database and Cache
- #1405 disable the checkpoint for unspent tx cache
- #1385 set tx reference map value to value instead of pointer
- #1382 start store confirm from CRCOnlyDPOSHeight
- #1379 add memory strategy for txn cache
- #1374 prevent checkpoint saving when initializing chain
- #1371 give a max count limit for tx cache
- #1369 add tool to count inputs within a block
- #1359 add check point mechanism for unspent txn cache
- #1343 separate the IStore interface from the chainstore
- #1341 remove unused dpos store and improve benchmark profile
- #1340 remove the logic of old chain store
- #1338 use the utxo cache
- #1334 cache the transaction which contains utxo
- #1333 improve performance by modifying the common read and write operation
- #1330 Seperate blocks size check to header and context checks
- #1328 fix benchmark sync error when chain data is huge
- #1326 migrate old ffldb database
- #1317 delete dpos leveldb store
- #1312 add benchmark about sync a node from scratch to best height
- #1311 add benchmark about run to specified height
- #1310 add benchmark about process single block
- #1309 utxo storage optimization
- #1307 add data generation tool
- #1303 do not save block node before save block 
- #1291 store block confirmation into ffldb 
- #1288 Database profile 

### Transaction Pool
- #1366 enable tx pool saving check point
- #1357 Refactor tx pool and eliminate lower fee rate tx when tx pool is full 

### Miscellaneous
- #1458 Use the actions ci status badge
- #1449 Cleanup test ci file
- #1431 Upgrade actions version
- #1421 Add Check PR for github actions
- #1419 Add Artifacts
- #1416 Add github actions
- #1397 Update main license date
- #1387 Update license date
- #1376 add print level to params
- #1283 add new logger generator that only output to file
- #1252 Fixed block timestamp is not after expected in docker
- #1214 Removes duplicate license 

### Glide and Go mod
- #1358 Use go mod dependency management system 
- #1319 Remove glide 

