package _interface

import (
	"github.com/elastos/Elastos.ELA/crypto"
)

/*
Keystore is a file based storage to save the account information,
including `Password` `MasterKey` `PrivateKey` etc. in AES encrypted format.
Keystore interface is a help to create a keystore file storage and master the accounts within it.
*/
type Keystore interface {
	// Create or open a keystore file
	Open(password string) (Keystore, error)

	// Change the password of this keystore
	ChangePassword(old, new string) error

	// Get the main account
	MainAccount() Account

	// Create a new sub account
	NewAccount() Account

	// Get main account and all sub accounts
	GetAccounts() []Account

	Json() (string, error)

	FromJson(json string, password string) error
}

type Account interface {
	// Create a signature of the given data with this account
	Sign(data []byte) ([]byte, error)

	// Get the public key of this account
	PublicKey() *crypto.PublicKey
}

func NewKeystore() Keystore {
	return &keystore{}
}
