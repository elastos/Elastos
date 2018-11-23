package blockchain

type BlockVersion interface {
	GetVersion() uint32
}

type BlockVersionMain struct {

}

func (b *BlockVersionMain) GetVersion() uint32 {
	return 1
}

