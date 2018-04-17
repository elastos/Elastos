package crypto

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"io"
	"sort"

	. "Elastos.ELA/common"

	"github.com/golang/crypto/ripemd160"
)

const (
	// Address types
	STANDARD   = 0xAC
	MULTISIG   = 0xAE
	CROSSCHAIN = 0xAF

	PUSH1 = 0x51

	// encoded public key length 0x21 || encoded public key (33 bytes) || OP_CHECKSIG(0xac)
	PublicKeyScriptLength = 35

	// signature length(0x40) || 64 bytes signature
	SignatureScriptLength = 65

	// 1byte m || 3 encoded public keys with leading 0x40 (34 bytes * 3) ||
	// 1byte n + 1byte OP_CHECKMULTISIG
	MinMultiSignCodeLength = 71
)

func MakeMerkleParent(left *Uint256, right *Uint256) (*Uint256, error) {
	// dupes can screw things up; CVE-2012-2459. check for them
	if left != nil && right != nil && left.IsEqual(*right) {
		return nil, errors.New("DUP HASH CRASH")
	}
	// if left child is nil, output nil.  Need this for hard mode.
	if left == nil {
		return nil, errors.New("Left child is nil")
	}
	// if right is nil, hash left with itself
	if right == nil {
		right = left
	}

	// Concatenate the left and right nodes
	var sha [64]byte
	copy(sha[:32], left[:])
	copy(sha[32:], right[:])

	parent := Uint256(Sha256D(sha[:]))
	return &parent, nil
}

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
	} else if signType == CROSSCHAIN {
		f = append([]byte{75}, f...)
	}

	return Uint168FromBytes(f)
}

func CreateStandardRedeemScript(publicKey *PublicKey) ([]byte, error) {
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

func CreateMultiSignRedeemScript(M uint, publicKeys []*PublicKey) ([]byte, error) {
	// Write M
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(PUSH1 + M - 1))

	//sort pubkey
	sort.Sort(PubKeySlice(publicKeys))

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
	buf.WriteByte(byte(PUSH1 + N - 1))
	buf.WriteByte(MULTISIG)

	return buf.Bytes(), nil
}

func ParseMultisigScript(code []byte) ([][]byte, error) {
	if len(code) < MinMultiSignCodeLength || code[len(code)-1] != MULTISIG {
		return nil, errors.New("not a valid multi sign transaction code, length not enough")
	}
	// remove last byte MULTISIG
	code = code[:len(code)-1]
	// remove m
	code = code[1:]
	// remove n
	code = code[:len(code)-1]
	if len(code)%(PublicKeyScriptLength-1) != 0 {
		return nil, errors.New("not a valid multi sign transaction code, length not match")
	}

	var publicKeys [][]byte
	i := 0
	for i < len(code) {
		script := make([]byte, PublicKeyScriptLength-1)
		copy(script, code[i:i+PublicKeyScriptLength-1])
		i += PublicKeyScriptLength - 1
		publicKeys = append(publicKeys, script)
	}
	return publicKeys, nil
}

func GetScriptType(script []byte) (byte, error) {
	if len(script) != PublicKeyScriptLength && len(script) < MinMultiSignCodeLength {
		return 0, errors.New("invalid transaction type, redeem script not a standard or multi sign type")
	}
	return script[len(script)-1], nil
}

func GetSigner(code []byte) (*Uint168, error) {
	if len(code) != PublicKeyScriptLength || code[len(code)-1] != STANDARD {
		return nil, errors.New("not a valid standard transaction code, length not match")
	}
	// remove last byte STANDARD
	code = code[:len(code)-1]
	script := make([]byte, PublicKeyScriptLength)
	copy(script, code[:PublicKeyScriptLength])

	return ToProgramHash(script)
}

func GetSigners(code []byte) ([]*Uint168, error) {
	scripts, err := ParseMultisigScript(code)
	if err != nil {
		return nil, err
	}

	var signers []*Uint168
	for _, script := range scripts {
		script = append(script, STANDARD)
		hash, _ := ToProgramHash(script)
		signers = append(signers, hash)
	}

	return signers, nil
}

func GetM(code []byte) (uint, error) {
	scriptType, err := GetScriptType(code)
	if err != nil {
		return 0, err
	}
	if scriptType != MULTISIG {
		return 0, errors.New("not a multisig script")
	}
	return getM(code), nil
}

func getM(code []byte) uint {
	return uint(code[0] - PUSH1 + 1)
}

func GetSignStatus(code, param []byte) (haveSign, needSign int, err error) {
	scriptType, err := GetScriptType(code)
	if err != nil {
		return -1, -1, err
	}

	if scriptType == STANDARD {
		signed := len(param) / SignatureScriptLength
		return signed, 1, nil

	} else if scriptType == MULTISIG {

		haveSign = len(param) / SignatureScriptLength

		return haveSign, int(getM(code)), nil
	}

	return -1, -1, errors.New("invalid redeem script type")
}

func AppendSignature(signerIndex int, signature, data, code, param []byte) ([]byte, error) {
	// Check if singer already signed
	if param != nil {
		publicKeys, err := ParseMultisigScript(code)
		if err != nil {
			return nil, err
		}
		for i := 0; i < len(param); i += SignatureScriptLength {
			// Remove length byte
			sign := param[i: i+SignatureScriptLength][1:]
			publicKey := publicKeys[signerIndex][1:]
			pubKey, err := DecodePoint(publicKey)
			if err != nil {
				return nil, err
			}
			err = Verify(*pubKey, data, sign)
			if err == nil {
				return nil, errors.New("signer already signed")
			}
		}
	}

	// Append new signature
	param = append(param, byte(len(signature)))
	return append(param, signature...), nil
}
