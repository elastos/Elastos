---
title: ETH Sidechain - Detailed Analysis
sidebar_label: ETH Sidechain - Detailed Analysis
---

### What's different between the Elastos ETH Sidechain and Ethereum?

The main difference is that the Elastos EVM has additional opcodes used by some native smart contracts to facilitate
the cross chain transfers of ELA between the Elastos mainchain and the ETH sidechain.

Since the Elastos EVM has a superset of opcodes, it's fully compatible with any existing Ethereum smart contracts and
there should be no noticeable difference to developers.

### Elastos ETH Sidechain Performance

The Elastos ETH Sidechain has the same 15 second blocks as Ethereum, but it has a much higher block gas limit.

- Ethereum block has limit = ~8,000,000 Gwei
- Elastos ETH Sidechain = 8,700,000,000 Gwei

This higher block has limit allows us to fit approximately 6,000 transactions in a 15 second block giving us about a
400 TPS performance metric.



