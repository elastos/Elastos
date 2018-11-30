package account

import (
	"errors"

	"github.com/elastos/Elastos.ELA/core/contract"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type Account struct {
	PrivateKey  []byte
	PublicKey   *crypto.PublicKey
	ProgramHash common.Uint168
	Contract    contract.Contract
	Address     string
}

func NewAccount() (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	signatureContract, err := contract.CreateStandardContractByPubKey(pubKey)
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
		PrivateKey:  priKey,
		PublicKey:   pubKey,
		ProgramHash: *programHash,
		Contract:    *signatureContract,
		Address:     address,
	}, nil
}

func NewAccountWithPrivateKey(privateKey []byte) (*Account, error) {
	privKeyLen := len(privateKey)

	if privKeyLen != 32 && privKeyLen != 96 && privKeyLen != 104 {
		return nil, errors.New("invalid private key")
	}

	pubKey := crypto.NewPubKey(privateKey)
	signatureContract, err := contract.CreateStandardContractByPubKey(pubKey)
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
		PrivateKey:  privateKey,
		PublicKey:   pubKey,
		ProgramHash: *programHash,
		Contract:    *signatureContract,
		Address:     address,
	}, nil
}

func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}
