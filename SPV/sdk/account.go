package sdk

import (
	"bytes"

	"github.com/elastos/Elastos.ELA.SPV/crypto"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
)

type Account struct {
	privateKey   []byte
	publicKey    *crypto.PublicKey
	redeemScript []byte
	programHash  *Uint168
	address      string
}

func NewAccount(privateKey []byte, publicKey *crypto.PublicKey) (*Account, error) {
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

func (a *Account) PublicKey() *crypto.PublicKey {
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

func (a *Account) SignTx(txn *tx.Transaction) ([]byte, error) {
	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	return a.Sign(buf.Bytes())
}

func (a *Account) Sign(data []byte) ([]byte, error) {
	signature, err := crypto.Sign(a.privateKey, data)
	if err != nil {
		return nil, err
	}
	return signature, nil
}
