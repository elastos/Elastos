package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/wallet/client"
)

type keystore struct {
	keystore *client.Keystore
}

// This method will open or create a keystore with the given password
func (impl *keystore) Open(password string) (Keystore, error) {
	var err error
	// Try to open keystore first
	impl.keystore, err = client.OpenKeystore([]byte(password))
	if err == nil {
		return impl, nil
	}

	// Try to create a keystore
	impl.keystore, err = client.CreateKeystore([]byte(password))
	if err != nil {
		return nil, err
	}

	return impl, nil
}

func (impl *keystore) ChangePassword(old, new string) error {
	return impl.keystore.ChangePassword([]byte(old), []byte(new))
}

func (impl *keystore) MainAccount() Account {
	return impl.keystore.MainAccount()
}

func (impl *keystore) NewAccount() Account {
	return impl.keystore.NewAccount()
}

func (impl *keystore) GetAccounts() []Account {
	var accounts []Account
	for _, account := range impl.keystore.GetAccounts() {
		accounts = append(accounts, account)
	}
	return accounts
}

func (impl *keystore) Json() (string, error) {
	return impl.keystore.Json()
}

func (impl *keystore) FromJson(str string, password string) error {
	impl.keystore = new(client.Keystore)
	return impl.keystore.FromJson(str, password)
}
