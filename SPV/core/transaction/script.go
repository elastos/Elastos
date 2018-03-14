package transaction

import (
	"io"
	"sort"
	"bytes"
	"errors"
	"crypto/sha256"

	"SPVWallet/crypto"
	. "SPVWallet/core"

	"golang.org/x/crypto/ripemd160"
)

type OpCode byte

func ToProgramHash(code []byte) (*Uint168, error) {
	temp := sha256.Sum256(code)
	md := ripemd160.New()
	io.WriteString(md, string(temp[:]))
	f := md.Sum(nil)

	signType := code[len(code)-1]
	if signType == STANDARD {
		f = append([]byte{33}, f...)
	} else if signType == MULTISIG {
		f = append([]byte{18}, f...)
	}

	return Uint168FromBytes(f)
}

func CreateStandardRedeemScript(publicKey *crypto.PublicKey) ([]byte, error) {
	content, err := publicKey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("create standard redeem script, encode public key failed")
	}
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(content)))
	buf.Write(content)
	buf.WriteByte(byte(STANDARD))

	return buf.Bytes(), nil
}

func CreateMultiSignRedeemScript(M int, publicKeys []*crypto.PublicKey) ([]byte, error) {
	// Write M
	opCode := OpCode(byte(PUSH1) + byte(M) - 1)
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(opCode))

	//sort pubkey
	sort.Sort(crypto.PubKeySlice(publicKeys))

	// Write public keys
	for _, pubkey := range publicKeys {
		content, err := pubkey.EncodePoint(true)
		if err != nil {
			return nil, errors.New("create multi sign redeem script, encode public key failed")
		}
		buf.WriteByte(byte(len(content)))
		buf.Write(content)
	}

	// Write N
	N := len(publicKeys)
	opCode = OpCode(byte(PUSH1) + byte(N) - 1)
	buf.WriteByte(byte(opCode))
	buf.WriteByte(MULTISIG)

	return buf.Bytes(), nil
}
