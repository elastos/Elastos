package blockchain

import (
	"math/big"

	"github.com/elastos/Elastos.ELA.SideChain/types"
)

func AssertBlock(value interface{}) *types.Block {
	switch block := value.(type) {
	case *types.Block:
		return block
	default:
		panic("parameter is not block")
	}
}

func AssertBigInt(value interface{}) *big.Int {
	switch bigInt := value.(type) {
	case *big.Int:
		return bigInt
	default:
		panic("parameter is not big.Int")
	}
}

func AssertMedianTimeSource(value interface{}) MedianTimeSource {
	switch timeSource := value.(type) {
	case MedianTimeSource:
		return timeSource
	default:
		panic("parameter is not MedianTimeSource")
	}
}

func AssertBlockNode(value interface{}) *BlockNode {
	switch node := value.(type) {
	case *BlockNode:
		return node
	default:
		panic("parameter is not BlockNode")
	}
}

func AssertHeader(value interface{}) *types.Header {
	switch header := value.(type) {
	case *types.Header:
		return header
	default:
		panic("parameter is not Header")
	}
}

func AssertTransaction(value interface{}) *types.Transaction {
	switch tx := value.(type) {
	case *types.Transaction:
		return tx
	default:
		panic("parameter is not Transaction")
	}
}

func AssertUint32(value interface{}) uint32 {
	switch v := value.(type) {
	case uint32:
		return v
	default:
		panic("parameter is not uint32")
	}
}

func AssertBlockHeight(value interface{}) uint32 {
	return AssertUint32(value)
}
