package service

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type StorageContext struct {
	codeHash *common.Uint168
	IsReadOnly bool
}

func NewStorageContext(codeHash *common.Uint168) *StorageContext {
	var storageContext StorageContext
	storageContext.codeHash = codeHash
	storageContext.IsReadOnly = false
	return &storageContext
}

func (sc *StorageContext) Bytes() []byte {
	return sc.codeHash.Bytes()
}

func (sc *StorageContext) Serialize(w io.Writer) error {
	err := common.WriteVarBytes(w, sc.codeHash.Bytes())
	if err != nil {
		return err
	}
	var b uint8 = 0
	if sc.IsReadOnly {
		b = 1
	}
	err = common.WriteUint8(w, b)
	if err != nil {
		return err
	}
	return nil
}
func (sc *StorageContext) Deserialize(r io.Reader) error {
	data, err := common.ReadVarBytes(r, 21, "StorageContext Deserialize")
	if err != nil {
		return err
	}
	hash, err := common.Uint168FromBytes(data)
	if err != nil {
		return nil
	}
	sc.codeHash = hash

	b, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	sc.IsReadOnly = false
	if b == 1 {
		sc.IsReadOnly = true
	}

	return nil
}