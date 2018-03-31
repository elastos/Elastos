package payload

import (
	"errors"
	"io"
	"github.com/elastos/Elastos.ELA.SPV/common/serialization"
)

type TransferCrossChainAsset struct {
	// string: publickey; uint64: output index
	PublicKeys map[string]uint64
}

func (a *TransferCrossChainAsset) Data(version byte) []byte {
	//TODO: implement TransferCrossChainAsset.Data()
	return []byte{0}
}

func (a *TransferCrossChainAsset) Serialize(w io.Writer, version byte) error {
	if a.PublicKeys == nil {
		return errors.New("Invalid publickey map")
	}

	if err := serialization.WriteVarUint(w, uint64(len(a.PublicKeys))); err != nil {
		return errors.New("publicKey map's length serialize failed")
	}

	for k, v := range a.PublicKeys {
		if err := serialization.WriteVarString(w, k); err != nil {
			return errors.New("publicKey map's key serialize failed")
		}

		if err := serialization.WriteVarUint(w, v); err != nil {
			return errors.New("publicKey map's value serialize failed")
		}
	}

	return nil
}

func (a *TransferCrossChainAsset) Deserialize(r io.Reader, version byte) error {
	if a.PublicKeys == nil {
		return errors.New("Invalid public key map")
	}

	length, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("publicKey map's length deserialize failed")
	}

	a.PublicKeys = nil
	a.PublicKeys = make(map[string]uint64)
	for i := uint64(0); i < length; i++ {
		k, err := serialization.ReadVarString(r)
		if err != nil {
			return errors.New("publicKey map's key deserialize failed")
		}

		v, err := serialization.ReadVarUint(r, 0)
		if err != nil {
			return errors.New("publicKey map's value deserialize failed")
		}

		a.PublicKeys[k] = v
	}

	return nil
}
