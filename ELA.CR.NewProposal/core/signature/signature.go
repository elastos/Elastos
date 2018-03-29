package signature

import (
	"io"
	"bytes"
	"errors"
	"crypto/sha256"

	. "Elastos.ELA/common"
	"Elastos.ELA/core/contract/program"

	"github.com/golang/crypto/ripemd160"
)

const (
	STANDARD   = 0xAC
	MULTISIG   = 0xAE
	CROSSCHAIN = 0xAF

	PUSH1 = 0x51

	// signature length(0x40) || 64 bytes signature
	SignatureScriptLength = 65
)

//SignableData describe the data need be signed.
type SignableData interface {
	//Get the the SignableData's program hashes
	GetProgramHashes() ([]Uint168, error)

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

func ToProgramHash(code []byte) (Uint168, error) {
	temp := sha256.Sum256(code)
	md := ripemd160.New()
	io.WriteString(md, string(temp[:]))
	f := md.Sum(nil)

	signType := code[len(code)-1]
	if signType == STANDARD {
		f = append([]byte{33}, f...)
	} else if signType == MULTISIG {
		f = append([]byte{18}, f...)
	} else if signType == CROSSCHAIN {
		f = append([]byte{75}, f...)
	} else {
		return Uint168{}, errors.New("unknown code signature type")
	}

	return Uint168FromBytes(f)
}
