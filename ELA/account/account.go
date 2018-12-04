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

func NewAccount(accountType string) (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()

	var err error
	var ct *contract.Contract
	switch accountType {
	case "standard":
		ct, err = contract.CreateStandardContractByPubKey(pubKey)
		if err != nil {
			return nil, err
		}
	case "multisig":
		//ct, err := contract.CreateMultiSigContractByPubKey(pubKey)
		//if err != nil {
		//	return nil, err
		//}
	case "sidechian":
	case "pledge":
		ct, err = contract.CreatePledgeContractByPubKey(pubKey)
		if err != nil {
			return nil, err
		}
	default:
		return nil, errors.New("account type not found")
	}

	programHash, err := ct.ToProgramHash()
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
		Contract:    *ct,
		Address:     address,
	}, nil
}

func NewAccountWithPrivateKey(privateKey []byte, prefixType contract.PrefixType) (*Account, error) {
	privKeyLen := len(privateKey)

	if privKeyLen != 32 && privKeyLen != 96 && privKeyLen != 104 {
		return nil, errors.New("invalid private key")
	}

	pubKey := crypto.NewPubKey(privateKey)
	var ct *contract.Contract
	var err error
	switch prefixType {
	case contract.PrefixStandard:
		ct, err = contract.CreateStandardContractByPubKey(pubKey)
		if err != nil {
			return nil, err
		}
	case contract.PrefixMultiSig:
		// TODO: implement multi signature
		return nil, errors.New("multi signature need to be implemented")
	case contract.PrefixCrossChain:
		// TODO: implement cross chain
		return nil, errors.New("cross chain need to be implemented")
	case contract.PrefixPledge:
		ct, err = contract.CreatePledgeContractByPubKey(pubKey)
		if err != nil {
			return nil, err
		}
	default:
		return nil, errors.New("undefined contract prefix type")
	}
	programHash, err := ct.ToProgramHash()
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
		Contract:    *ct,
		Address:     address,
	}, nil
}

func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}
