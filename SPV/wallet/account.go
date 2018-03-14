package wallet

import (
	"bytes"

	. "SPVWallet/crypto"
	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
)

type Account struct {
	privateKey   []byte
	publicKey    *PublicKey
	redeemScript []byte
	programHash  *Uint168
	address      string
}

func NewAccount(privateKey []byte, publicKey *PublicKey) (*Account, error) {
	signatureRedeemScript, err := tx.CreateStandardRedeemScript(publicKey)
	if err != nil {
		return nil, err
	}

	programHash, err := tx.ToProgramHash(signatureRedeemScript)
	if err != nil {
		return nil, err
	}

	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		privateKey:   privateKey,
		publicKey:    publicKey,
		redeemScript: signatureRedeemScript,
		programHash:  programHash,
		address:      address,
	}, nil
}

func (a *Account) PrivateKey() []byte {
	return a.privateKey
}

func (a *Account) PublicKey() *PublicKey {
	return a.publicKey
}

func (a *Account) RedeemScript() []byte {
	return a.redeemScript
}

func (a *Account) ProgramHash() *Uint168 {
	return a.programHash
}

func (a *Account) Address() string {
	return a.address
}

func (a *Account) Sign(txn *tx.Transaction) ([]byte, error) {
	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	signedData, err := Sign(a.privateKey, buf.Bytes())
	if err != nil {
		return nil, err
	}

	return signedData, nil
}
