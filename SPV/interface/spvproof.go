package _interface

import (
	tx "SPVWallet/core/transaction"
	"SPVWallet/p2p/msg"
)

const DefaultConfirmations = 6

type SPVProof struct {
	msg.MerkleBlock
	txs           []tx.Transaction
	confirmHeight uint32
}

func NewSPVProof(block msg.MerkleBlock, txs []tx.Transaction) *SPVProof {
	return &SPVProof{
		MerkleBlock:   block,
		txs:           txs,
		confirmHeight: getConfirmHeight(block, txs),
	}
}

func getConfirmHeight(block msg.MerkleBlock, txs []tx.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	return block.BlockHeader.Height + DefaultConfirmations
}
