package account

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

/*
A ELA standard account is a set of private key, public key, redeem script, program hash and address data.
redeem script is (script content length)+(script content)+(script type),
program hash is the sha256 value of redeem script and converted to ripemd160 format with a (Type) prefix.
address is the base58 format of program hash, which is the string value show up on user interface as account address.
With account, you can get the transfer address or sign transaction etc.
*/
type Account struct {
	PrivateKey   []byte
	PublicKey    *crypto.PublicKey
	ProgramHash  common.Uint168
	RedeemScript []byte
	Address      string
}

// Create an account instance with private key and public key
func NewAccount() (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, err
	}

	programHash := signatureContract.ToProgramHash()
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
	programHash := signatureContract.ToProgramHash()
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

func NewMultiSigAccount(m int, pubKeys []*crypto.PublicKey) (*Account, error) {
	multiSigContract, err := contract.CreateMultiSigContract(m, pubKeys)
	if err != nil {
		return nil, err
	}

	programHash := multiSigContract.ToProgramHash()
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		PrivateKey:   nil,
		PublicKey:    nil,
		ProgramHash:  *programHash,
		RedeemScript: multiSigContract.Code,
		Address:      address,
	}, nil
}

// Get account private key
func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

// Get account public key
func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}

// Sign data with account
func (ac *Account) Sign(data []byte) ([]byte, error) {
	signature, err := crypto.Sign(ac.PrivateKey, data)
	if err != nil {
		return nil, err
	}
	return signature, nil
}
