package avm

import (
	"errors"

	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/params"
)

type CryptoECDsa struct {
}

func (c *CryptoECDsa) Hash168(data []byte) []byte {
	temp, err := params.ToCodeHash(data)
	if err != nil {
		return nil
	}
	return temp.Bytes()
}

func (c *CryptoECDsa) Hash256(data []byte) []byte {
	hash := common.Uint256(common.Sha256D(data))
	return hash.Bytes()
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
