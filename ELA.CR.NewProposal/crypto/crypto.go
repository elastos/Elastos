package crypto

import (
	"Elastos.ELA/common/serialization"
	"errors"
	"fmt"
	"io"
	"math/big"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/sha256"
)

const (
	SIGNRLEN     = 32
	SIGNATURELEN = 64
	NEGBIGNUMLEN = 33
)

type CryptoAlgSet struct {
	EccParams elliptic.CurveParams
	Curve     elliptic.Curve
}

var algSet CryptoAlgSet

type PubKey struct {
	X, Y *big.Int
}

func init() {
	algSet.Curve = elliptic.P256()
	algSet.EccParams = *(algSet.Curve.Params())
}

func Verify(publicKey PubKey, data []byte, signature []byte) error {
	len := len(signature)
	if len != SIGNATURELEN {
		fmt.Printf("Unknown signature length %d\n", len)
		return errors.New("Unknown signature length")
	}

	r := new(big.Int).SetBytes(signature[:len/2])
	s := new(big.Int).SetBytes(signature[len/2:])

	digest := sha256.Sum256(data)

	pub := new(ecdsa.PublicKey)

	pub.Curve = algSet.Curve
	pub.X = new(big.Int).Set(publicKey.X)
	pub.Y = new(big.Int).Set(publicKey.Y)

	if ecdsa.Verify(pub, digest[:], r, s) {
		return nil
	} else {
		return errors.New("[Validation], Verify failed.")
	}

}

func (e *PubKey) Serialize(w io.Writer) error {
	bufX := []byte{}
	if e.X.Sign() == -1 {
		// prefix 0x00 means the big number X is negative
		bufX = append(bufX, 0x00)
	}
	bufX = append(bufX, e.X.Bytes()...)

	if err := serialization.WriteVarBytes(w, bufX); err != nil {
		return err
	}

	bufY := []byte{}
	if e.Y.Sign() == -1 {
		// prefix 0x00 means the big number Y is negative
		bufY = append(bufY, 0x00)
	}
	bufY = append(bufY, e.Y.Bytes()...)
	if err := serialization.WriteVarBytes(w, bufY); err != nil {
		return err
	}
	return nil
}

func (e *PubKey) Deserialize(r io.Reader) error {
	bufX, err := serialization.ReadVarBytes(r)
	if err != nil {
		return err
	}
	e.X = big.NewInt(0)
	e.X = e.X.SetBytes(bufX)
	if len(bufX) == NEGBIGNUMLEN {
		e.X.Neg(e.X)
	}
	bufY, err := serialization.ReadVarBytes(r)
	if err != nil {
		return err
	}
	e.Y = big.NewInt(0)
	e.Y = e.Y.SetBytes(bufY)
	if len(bufY) == NEGBIGNUMLEN {
		e.Y.Neg(e.Y)
	}
	return nil
}
