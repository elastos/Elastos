# SPV Implement
## Summary
### This is a upgrade version of the Elastos digital currency node program, which implemented SPV (Simplified Payment Verification) service.
## Changes
1. Add folder `bloom` which is the source code of an implement of bloom filter.
2. Add `SPVService` parameter to config file, this is a boolean value to set SPV service on/off
3. Modify Handle() method in `datarequest` to support Transaction data request.
4. Add new message type `filterload` under `net/message/` folder, this is the message to make service node load bloom filter.
5. Add `Type` parameter to `InvPayload`, make inventory message can take `BLOCK` and `TRANSACTION` two different types.
6. Add new message type `merkleblock` under `net/message/` folder, this is a special message type only used in SPV communication.
7. Add `spvfilter` file under `net/message/` folder, which is a helper to filter SPV messages from SPVPort.
8. Add `listenSPVPort` method to `net/node/link.go`, SPV service are provided by the specific SPVPort.
9. Add `spvlink` to `net/node/` folder, which is the implement of SPVPort listen method.
10. Update `PROTOCOLVERSION` from 0 to 1
11. Add `SPVPort` and `SPVService` constant to `net/protocol/protocol.go`
12. Add `LoadFilter(*bloom.Filter)` and `GetFilter() *bloom.Filter` to `Noder` interface, and implement these methods in `node`.
12. Merge `Xmit()` and `Relay()` into one method `Relay()`.

## Usage
1. Set `SPVService` parameter to `true` in `config.json`.
2. Run `./node` in the console.