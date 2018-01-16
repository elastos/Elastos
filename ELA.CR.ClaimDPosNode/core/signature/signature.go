package signature

import (
	"io"
	"bytes"

	"Elastos.ELA/common"
	"Elastos.ELA/core/contract/program"
)

//SignableData describe the data need be signed.
type SignableData interface {
	//Get the the SignableData's program hashes
	GetProgramHashes() ([]common.Uint168, error)

	SetPrograms([]*program.Program)

	GetPrograms() []*program.Program

	SerializeUnsigned(io.Writer) error

	GetDataContent() []byte
}

func GetDataContent(data SignableData) []byte {
	buf := new(bytes.Buffer)
	data.SerializeUnsigned(buf)
	return buf.Bytes()
}
