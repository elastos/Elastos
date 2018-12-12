package main

import (
	"bytes"
	"fmt"
	"os"
	"strconv"

	"github.com/elastos/Elastos.ELA/blockchain"

	"github.com/elastos/Elastos.ELA/common"
)

func main() {
	wantHeightString, _ := strconv.Atoi(os.Args[1])
	targetHeight := uint32(wantHeightString)

	st, err := blockchain.NewLevelDB("Chain")
	if err != nil {
		fmt.Println("connect leveldb failed! Please check wether there is already a ela process running.", err)
	}
	chain := blockchain.ChainStore{IStore: st}

	data, _ := st.Get([]byte{byte(blockchain.SYSCurrentBlock)})
	// we drop 0-32 bytes of data, because it is current block hash.
	currentHeight, _ := common.ReadUint32(bytes.NewReader(data[32:]))

	if targetHeight >= currentHeight {
		fmt.Printf("Current height of blockchain is %d, you can't do this, man.", currentHeight)
		return
	}

	for i := currentHeight; i > targetHeight; i-- {
		fmt.Println("current height is", i)
		blockHash, _ := chain.GetBlockHash(i)
		block, _ := chain.GetBlock(blockHash)
		fmt.Println("blockhash before rollback:", blockHash.String())

		chain.NewBatch()
		chain.RollbackTrimmedBlock(block)
		chain.RollbackBlockHash(block)
		chain.RollbackTransactions(block)
		chain.RollbackUnspendUTXOs(block)
		chain.RollbackUnspend(block)
		chain.RollbackCurrentBlock(block)
		chain.BatchCommit()

		blockHashAfter, _ := chain.GetBlockHash(i)
		fmt.Println("blockhash after rollback:", blockHashAfter.String())
	}
}
