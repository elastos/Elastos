package account

import (
	"errors"

	"github.com/elastos/Elastos.ELA/core/contract"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

type Account struct {
	PrivateKey  []byte
	PublicKey   *crypto.PublicKey
	ProgramHash common.Uint168
}

func NewAccount() (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	signatureRedeemScript, err := contract.CreateSignatureRedeemScript(pubKey)
	if err != nil {
		return nil, err
	}
	programHash, err := crypto.ToProgramHash(signatureRedeemScript)
	if err != nil {
		return nil, err
	}

	return &Account{
		PrivateKey:  priKey,
		PublicKey:   pubKey,
		ProgramHash: *programHash,
	}, nil
}

func NewAccountWithPrivateKey(privateKey []byte) (*Account, error) {
	privKeyLen := len(privateKey)

	if privKeyLen != 32 && privKeyLen != 96 && privKeyLen != 104 {
		return nil, errors.New("invalid private key")
	}

	pubKey := crypto.NewPubKey(privateKey)
	signatureRedeemScript, err := contract.CreateSignatureRedeemScript(pubKey)
	if err != nil {
		return nil, err
	}
	programHash, err := crypto.ToProgramHash(signatureRedeemScript)
	if err != nil {
		return nil, err
	}
	return &Account{
		PrivateKey:  privateKey,
		PublicKey:   pubKey,
		ProgramHash: *programHash,
	}, nil
}

func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}
