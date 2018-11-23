package blockhistory

import "github.com/elastos/Elastos.ELA/blockchain"

type BlockVersionV0 struct {
	blockchain.BlockVersionMain
}

func (b *BlockVersionV0) GetVersion() uint32 {
	return 0
}
