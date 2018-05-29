package core

import "io"

const RegisterIdentificationVersion = 0x00

type PayloadRegisterIdentification struct{}

func (a *PayloadRegisterIdentification) Data(version byte) []byte {
	//TODO: implement TransferAsset.Data()
	return []byte{0}
}

func (a *PayloadRegisterIdentification) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *PayloadRegisterIdentification) Deserialize(r io.Reader, version byte) error {
	return nil
}
