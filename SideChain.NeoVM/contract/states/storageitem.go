package states

import (
	"io"
	"errors"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
)

type StorageItem struct {
	StateBase
	Value []byte
}

func NewStorageItem(value []byte) *StorageItem {
	var storageItem StorageItem
	storageItem.Value = value
	return &storageItem
}

func (storageItem *StorageItem) Serialize(w io.Writer) error {
	storageItem.StateBase.Serialize(w)
	common.WriteVarBytes(w, storageItem.Value)
	return nil
}

func (storageItem *StorageItem) Deserialize(r io.Reader) error {
	stateBase := new(StateBase)
	err := stateBase.Deserialize(r)
	if err != nil {
		return err
	}
	storageItem.StateBase = *stateBase
	value, err := common.ReadVarBytes(r, avm.MaxItemSize, "StorageItem Deserialize value")
	if err != nil {
		return errors.New("StorageItem Code Deserialize fail.")
	}
	storageItem.Value = value
	return nil
}

func (storageItem *StorageItem) Bytes() []byte {
	b := new(bytes.Buffer)
	storageItem.Serialize(b)
	return b.Bytes()
}
