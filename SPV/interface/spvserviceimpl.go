package _interface

import (
	"errors"

	"SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
	"SPVWallet/p2p/msg"
	"SPVWallet/spv"
)

type SPVServiceImpl struct {
	*spv.SPV
	clientId   uint64
	addrFilter *db.AddrFilter
	spvProofs  map[core.Uint256]*SPVProof
	callback   func(msg.MerkleBlock, []tx.Transaction)
}

func (service *SPVServiceImpl) RegisterAccount(address string) error {
	account, err := core.Uint168FromAddress(address)
	if err != nil {
		return errors.New("Invalid address format")
	}
	if service.addrFilter == nil {
		service.addrFilter = db.NewAddrFilter(nil)
	}
	service.addrFilter.AddAddr(db.NewAddr(account, nil))
	return nil
}

func (service *SPVServiceImpl) OnTransactionConfirmed(callback func(msg.MerkleBlock, []tx.Transaction)) {
	service.callback = callback
}

func (service *SPVServiceImpl) VerifyTransaction(block msg.MerkleBlock, txs []tx.Transaction) error {
	if service.SPV == nil {
		return errors.New("SPV service not started")
	}
	blockHash := block.BlockHeader.Hash()

	// Check if block is on the main chain
	if !service.BlockChain().IsKnownBlock(*blockHash) {
		return errors.New("can not find block on main chain")
	}

	// Check if merkleroot is match
	txIds, err := spv.CheckMerkleBlock(&block)
	if err != nil {
		return errors.New("check merkle branch failed, " + err.Error())
	}
	if len(txIds) == 0 {
		return errors.New("no transactions found")
	}

	// Check if transactions hashes are match
	matches := 0
	for _, txId := range txIds {
		for _, tx := range txs {
			if *txId == *tx.Hash() {
				matches++
			}
		}
	}
	if matches != len(txs) {
		return errors.New("transaction hash not match")
	}

	// Check if transactions hits the registered accounts
	if !service.addrFilter.IsLoaded() {
		return errors.New("no account registered")
	}
	for _, tx := range txs {
		match := false
		for _, output := range tx.Outputs {
			if service.addrFilter.ContainAddress(output.ProgramHash) {
				match = true
			}
		}
		if !match {
			return errors.New("transaction not related with registered account")
		}
	}
	return nil
}

func (service *SPVServiceImpl) SendTransaction(tx tx.Transaction) error {
	if service.SPV == nil {
		return errors.New("SPV service not started")
	}

	return service.SPV.SendTransaction(tx)
}

func (service *SPVServiceImpl) Start() error {
	if service.SPV != nil {
		return errors.New("SPV service already started")
	}

	var err error
	service.SPV, err = spv.InitSPV(service.clientId)
	if err != nil {
		return err
	}
	service.SPV.SetOnBlockCommitListener(service.onBlockCommit)
	service.SPV.Start()

	return nil
}

func (service *SPVServiceImpl) onBlockCommit(block msg.MerkleBlock, txs []tx.Transaction) {
	// Save SPV proof
	hash := *block.BlockHeader.Hash()
	height := block.BlockHeader.Height
	proof := NewSPVProof(block, txs)
	service.spvProofs[hash] = proof

	// Look up for confirmed blocks
	for _, proof := range service.spvProofs {
		if height > proof.confirmHeight {
			service.callback(proof.MerkleBlock, proof.txs)
		}
	}
}
