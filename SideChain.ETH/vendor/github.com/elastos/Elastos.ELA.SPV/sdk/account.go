package sdk

import (
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
	privateKey   []byte
	publicKey    *crypto.PublicKey
	redeemScript []byte
	programHash  *common.Uint168
	address      string
}

// Create an account instance with private key and public key
func NewAccount(privateKey []byte, publicKey *crypto.PublicKey) (*Account, error) {
	contract, err := contract.CreateStandardContract(publicKey)
	if err != nil {
		return nil, err
	}

	programHash := contract.ToProgramHash()

	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		privateKey:   privateKey,
		publicKey:    publicKey,
		redeemScript: contract.Code,
		programHash:  programHash,
		address:      address,
	}, nil
}

// Get account private key
func (a *Account) PrivateKey() []byte {
	return a.privateKey
}

// Get account public key
func (a *Account) PublicKey() *crypto.PublicKey {
	return a.publicKey
}

// Get account redeem script
func (a *Account) RedeemScript() []byte {
	return a.redeemScript
}

// Get account program hash
func (a *Account) ProgramHash() *common.Uint168 {
	return a.programHash
}

// Get account address
func (a *Account) Address() string {
	return a.address
}

// Sign data with account
func (a *Account) Sign(data []byte) ([]byte, error) {
	signature, err := crypto.Sign(a.privateKey, data)
	if err != nil {
		return nil, err
	}
	return signature, nil
}
