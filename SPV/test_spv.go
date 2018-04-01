package main

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"os"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	i "github.com/elastos/Elastos.ELA.SPV/interface"
)

var spv i.SPVService

func main() {
	log.Init()

	var id = make([]byte, 8)
	var clientId uint64
	var err error
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, clientId)
	spv = i.NewSPVService(clientId)

	// Register account
	err = spv.RegisterAccount("ETBBrgotZy3993o9bH75KxjLDgQxBCib6u")
	err = spv.RegisterAccount("EUyNwnAh5SzzTtAPV1HkXzjUEbw2YqKsUM")
	if err != nil {
		log.Error("Register account error: ", err)
		os.Exit(0)
	}

	// Set on transaction confirmed callback
	spv.OnTransactionConfirmed(onTransactionConfirmed)

	// Start spv service
	spv.Start()
}

func onTransactionConfirmed(proof db.Proof, tx tx.Transaction) {
	log.Debug("Receive transaction confirm, hash:", tx.Hash().String())
	err := spv.VerifyTransaction(proof, tx)
	if err != nil {
		log.Error("Verify transaction error: ", err)
		return
	}

	// Submit transaction receipt
	spv.SubmitTransactionReceipt(*tx.Hash())
}
