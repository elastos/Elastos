package core

import "io"

type PayloadTransferAsset struct{}

func (a *PayloadTransferAsset) Data(version byte) []byte {
	//TODO: implement TransferAsset.Data()
	return []byte{0}
}

func (a *PayloadTransferAsset) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *PayloadTransferAsset) Deserialize(r io.Reader, version byte) error {
	return nil
}
