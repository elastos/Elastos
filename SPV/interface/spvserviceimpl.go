package _interface

import (
	"bytes"
	"errors"
	"os"
	"os/signal"

	"SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
	"SPVWallet/msg"
	"SPVWallet/spv"
	"SPVWallet/log"
)

type SPVServiceImpl struct {
	*spv.SPV
	clientId uint64
	accounts []*core.Uint168
	callback func(db.Proof, tx.Transaction)
}

func (service *SPVServiceImpl) RegisterAccount(address string) error {
	account, err := core.Uint168FromAddress(address)
	if err != nil {
		return errors.New("Invalid address format")
	}
	service.accounts = append(service.accounts, account)
	return nil
}

func (service *SPVServiceImpl) OnTransactionConfirmed(callback func(db.Proof, tx.Transaction)) {
	service.callback = callback
}

func (service *SPVServiceImpl) SubmitTransactionReceipt(txHash core.Uint256) error {
	return service.BlockChain().Queue().Delete(&txHash)
}

func (service *SPVServiceImpl) VerifyTransaction(proof db.Proof, tx tx.Transaction) error {
	if service.SPV == nil {
		return errors.New("SPV service not started")
	}

	// Check if block is on the main chain
	if !service.BlockChain().IsKnownBlock(proof.BlockHash) {
		return errors.New("can not find block on main chain")
	}

	// Get Header from main chain
	header, err := service.BlockChain().GetHeader(&proof.BlockHash)
	if err != nil {
		return errors.New("can not get block from main chain")
	}

	// Check if merkleroot is match
	merkleBlock := msg.MerkleBlock{
		BlockHeader:  *header,
		Transactions: proof.Transactions,
		Hashes:       proof.Hashes,
		Flags:        proof.Flags,
	}
	txIds, err := spv.CheckMerkleBlock(merkleBlock)
	if err != nil {
		return errors.New("check merkle branch failed, " + err.Error())
	}
	if len(txIds) == 0 {
		return errors.New("invalid transaction proof, no transactions found")
	}

	// Check if transaction hash is match
	match := false
	for _, txId := range txIds {
		if *txId == *tx.Hash() {
			match = true
			break
		}
	}
	if !match {
		return errors.New("transaction hash not match proof")
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

	// Register accounts
	if len(service.accounts) == 0 {
		return errors.New("No account registered")
	}
	for _, account := range service.accounts {
		service.BlockChain().Addrs().Put(account, RegisteredAccountScript, db.TypeNotify)
	}

	// Set callback
	service.SPV.SetOnBlockCommitListener(service.onBlockCommit)

	// Handle interrupt signal
	stop := make(chan int, 1)
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, os.Interrupt)
	go func() {
		for range signals {
			log.Trace("SPV service shutting down...")
			service.Stop()
			stop <- 1
		}
	}()

	// Start SPV service
	service.SPV.Start()

	<-stop

	return nil
}

func (service *SPVServiceImpl) onBlockCommit(header db.Header, proof db.Proof, txs []tx.Transaction) {
	// If no transactions return
	if len(txs) == 0 {
		return
	}

	// If no transaction match registered account return
	var matchedTxs []tx.Transaction
	for _, tx := range txs {
		for _, output := range tx.Outputs {
			if service.BlockChain().Addrs().GetAddrFilter().ContainAddress(output.ProgramHash) {
				matchedTxs = append(matchedTxs, tx)
			}
		}
	}
	if len(matchedTxs) == 0 {
		return
	}

	// Queue transactions
	for _, tx := range txs {
		item := &db.QueueItem{
			TxHash:        *tx.Hash(),
			BlockHash:     *header.Hash(),
			Height:        header.Height,
			ConfirmHeight: getConfirmHeight(header, tx),
		}

		// Save to queue db
		service.BlockChain().Queue().Put(item)
	}

	// Look up for confirmed transactions
	confirmedItems, err := service.BlockChain().Queue().GetConfirmed(header.Height)
	if err != nil {
		return
	}
	for _, item := range confirmedItems {
		//	Get proof from db
		proof, err := service.BlockChain().Proofs().Get(&item.BlockHash)
		if err != nil {
			log.Error("Query merkle proof failed, block hash:", item.BlockHash.String())
			return
		}
		//	Get transaction from db
		txn, err := service.BlockChain().TXNs().Get(&item.TxHash)
		if err != nil {
			log.Error("Query transaction failed, tx hash:", item.TxHash.String())
			return
		}
		proof = getTransactionProof(proof, txn.TxId)

		var tx tx.Transaction
		err = tx.DeserializeUnsigned(bytes.NewReader(txn.RawData))
		if err != nil {
			log.Error("Deserialize stord transaction failed, hash: ", txn.TxId.String())
			return
		}

		// Trigger callback
		service.callback(*proof, tx)
	}
}

func getConfirmHeight(header db.Header, tx tx.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	return header.Height + DefaultConfirmations
}

func getTransactionProof(proof *db.Proof, txHash core.Uint256) *db.Proof {
	// TODO Pick out the merkle proof of the transaction
	return proof
}
