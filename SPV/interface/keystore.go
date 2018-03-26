package _interface

import (
	"SPVWallet/crypto"
)

type Keystore interface {
	Open(password string) (Keystore, error)
	ChangePassword(old, new string) error
	MainAccount() Account
	NewAccount() Account
	GetAccounts() []Account
}

type Account interface {
	Sign(data []byte) ([]byte, error)
	PublicKey() *crypto.PublicKey
}

func NewKeystore() Keystore {
	return &KeystoreImpl{}
}