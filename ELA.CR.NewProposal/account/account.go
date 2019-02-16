package account

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

type Account struct {
	PrivateKey   []byte
	PublicKey    *crypto.PublicKey
	ProgramHash  common.Uint168
	RedeemScript []byte
	Address      string
}

func NewAccount() (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, err
	}

	programHash, err := signatureContract.ToProgramHash()
	if err != nil {
		return nil, err
	}
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		PrivateKey:   priKey,
		PublicKey:    pubKey,
		ProgramHash:  *programHash,
		RedeemScript: signatureContract.Code,
		Address:      address,
	}, nil
}

func NewAccountWithPrivateKey(privateKey []byte) (*Account, error) {
	priKeyLen := len(privateKey)

	if priKeyLen != 32 && priKeyLen != 96 && priKeyLen != 104 {
		return nil, errors.New("invalid private key")
	}

	pubKey := crypto.NewPubKey(privateKey)
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, err
	}
	programHash, err := signatureContract.ToProgramHash()
	if err != nil {
		return nil, err
	}
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}
	return &Account{
		PrivateKey:   privateKey,
		PublicKey:    pubKey,
		ProgramHash:  *programHash,
		RedeemScript: signatureContract.Code,
		Address:      address,
	}, nil
}

func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}
