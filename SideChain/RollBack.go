package main

import (
	"bytes"
	"fmt"
	"os"
	"strconv"

	"github.com/elastos/Elastos.ELA.SideChain/common"
	"github.com/elastos/Elastos.ELA.SideChain/common/serialization"
	"github.com/elastos/Elastos.ELA.SideChain/core/store/ChainStore"
	"github.com/elastos/Elastos.ELA.SideChain/core/store/LevelDBStore"
)

func main() {
	wantHeight, _ := strconv.Atoi(os.Args[1])
	wantheightInt32 := uint32(wantHeight)

	st, err := LevelDBStore.NewLevelDBStore("Chain")
	if err != nil {
		fmt.Println("connect leveldb failed!")
		fmt.Println(err)
	}

	chain := &ChainStore.ChainStore{
		IStore: st,
	}

	key := bytes.NewBuffer(nil)
	key.WriteByte(byte(0x40))
	heightByte, _ := st.Get(key.Bytes())
	r := bytes.NewReader(heightByte)
	var blockHash common.Uint256
	blockHash.Deserialize(r)
	realHeight, _ := serialization.ReadUint32(r)

	if wantheightInt32 >= realHeight {
		fmt.Println("Current height of blockchain is %d, you can't do this, man.", realHeight)
		return
	}

	for i := realHeight; i > wantheightInt32; i-- {
		fmt.Println("i is", i)
		blockHashBefore, _ := chain.GetBlockHash(i)
		fmt.Println("blockhash before")
		fmt.Println(blockHashBefore)

		block, _ := chain.GetBlock(blockHashBefore)

		chain.BatchInit()
		chain.RollbackTrimemedBlock(block)
		chain.RollbackBlockHash(block)
		chain.RollbackTransactions(block)
		chain.RollbackUnspendUTXOs(block)
		chain.RollbackUnspend(block)
		chain.RollbackCurrentBlock(block)
		chain.BatchFinish()

		blockHashAfter, _ := chain.GetBlockHash(i)
		fmt.Println("blockhash after")
		fmt.Println(blockHashAfter)
	}
}
