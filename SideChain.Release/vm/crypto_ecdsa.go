package vm

import (
	"errors"

	"github.com/elastos/Elastos.ELA/crypto"
)

type CryptoECDsa struct {
}

func (c *CryptoECDsa) Hash168(data []byte) []byte {
	return []byte{}
}

func (c *CryptoECDsa) Hash256(data []byte) []byte {
	return []byte{}
}

func (c *CryptoECDsa) VerifySignature(data []byte, signature []byte, pubkey []byte) error {

	pk, err := crypto.DecodePoint(pubkey)
	if err != nil {
		return errors.New("[CryptoECDsa], crypto.DecodePoint failed.")
	}

	err = crypto.Verify(*pk, data, signature)
	if err != nil {
		return errors.New("[CryptoECDsa], VerifySignature failed.")
	}
	return nil
}
