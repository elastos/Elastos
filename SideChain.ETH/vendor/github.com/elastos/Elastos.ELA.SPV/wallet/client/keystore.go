package client

import (
	"bytes"
	"crypto/rand"
	"crypto/sha256"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	KeystoreVersion = "1.0"
)

type Keystore struct {
	sync.Mutex

	*KeystoreFile

	masterKey []byte

	accounts []*sdk.Account
}

func CreateKeystore(password []byte) (*Keystore, error) {
	keystoreFile, err := CreateKeystoreFile()
	if err != nil {
		return nil, err
	}

	keystore := &Keystore{
		KeystoreFile: keystoreFile,
	}

	iv := make([]byte, 16)
	_, err = rand.Read(iv)
	if err != nil {
		return nil, err
	}
	// Set IV
	keystoreFile.SetIV(iv)

	masterKey := make([]byte, 32)
	_, err = rand.Read(masterKey)
	if err != nil {
		return nil, err
	}

	passwordKey := crypto.ToAesKey(password)
	passwordHash := sha256.Sum256(passwordKey)
	// Set password hash
	keystoreFile.SetPasswordHash(passwordHash[:])

	masterKeyEncrypted, err := keystore.encryptMasterKey(passwordKey, masterKey)
	if err != nil {
		return nil, err
	}
	// Set master key encrypted
	keystoreFile.SetMasterKeyEncrypted(masterKeyEncrypted)

	// Generate new key pair
	privateKey, publicKey, err := crypto.GenerateKeyPair()
	if err != nil {
		return nil, err
	}

	privateKeyEncrypted, err := keystore.encryptPrivateKey(masterKey, passwordKey, privateKey, publicKey)
	// Set private key encrypted
	keystoreFile.SetPrivateKeyEncrypted(privateKeyEncrypted)

	// Init keystore parameters
	err = keystore.initAccounts(masterKey, privateKey, publicKey)
	if err != nil {
		return nil, err
	}

	err = keystoreFile.SaveToFile()
	if err != nil {
		return nil, err
	}

	return keystore, nil
}

func OpenKeystore(password []byte) (*Keystore, error) {
	keystoreFile, err := OpenKeystoreFile()
	if err != nil {
		return nil, err
	}
	keystore := new(Keystore)
	err = keystore.initKeystore(keystoreFile, password)
	return keystore, err
}

func (store *Keystore) initKeystore(keystoreFile *KeystoreFile, password []byte) error {
	store.KeystoreFile = keystoreFile
	err := store.verifyPassword(password)
	if err != nil {
		return err
	}

	passwordKey := crypto.ToAesKey(password)

	masterKey, err := store.decryptMasterKey(passwordKey)

	privateKey, publicKey, err := store.decryptPrivateKey(masterKey, passwordKey)
	if err != nil {
		return err
	}

	return store.initAccounts(masterKey, privateKey, publicKey)
}

func (store *Keystore) initAccounts(masterKey, privateKey []byte, publicKey *crypto.PublicKey) error {
	// initiate main account
	mainAccount, err := sdk.NewAccount(privateKey, publicKey)
	if err != nil {
		return err
	}

	// master key
	store.masterKey = masterKey

	// add main account to index 0
	store.accounts = append(store.accounts, mainAccount)

	// initiate sub accounts
	for i := 1; i <= store.SubAccountsCount; i++ {
		privateKey, publicKey, err := crypto.GenerateSubKeyPair(i, masterKey, privateKey)
		if err != nil {
			return err
		}
		childAccount, err := sdk.NewAccount(privateKey, publicKey)
		if err != nil {
			return err
		}
		store.accounts = append(store.accounts, childAccount)
	}

	return nil
}

func (store *Keystore) verifyPassword(password []byte) error {
	passwordKey := crypto.ToAesKey(password)
	passwordHash := sha256.Sum256(passwordKey)

	origin, err := store.GetPasswordHash()
	if err != nil {
		return err
	}
	if !bytes.Equal(origin, passwordHash[:]) {
		return errors.New("password wrong")
	}
	return nil
}

func (store *Keystore) ChangePassword(oldPassword, newPassword []byte) error {
	// Get old passwordKey
	oldPasswordKey := crypto.ToAesKey(oldPassword)

	masterKeyEncrypted, err := store.GetMasterKeyEncrypted()
	if err != nil {
		return err
	}
	defer common.ClearBytes(masterKeyEncrypted)

	masterKey, err := store.decryptMasterKey(oldPasswordKey)
	if err != nil {
		return err
	}

	// Decrypt private key
	privateKey, publicKey, err := store.decryptPrivateKey(masterKey, oldPasswordKey)
	if err != nil {
		return err
	}

	// Encrypt private key with new password
	newPasswordKey := crypto.ToAesKey(newPassword)
	newPasswordHash := sha256.Sum256(newPasswordKey)

	masterKeyEncrypted, err = store.encryptMasterKey(newPasswordKey, masterKey)
	if err != nil {
		return err
	}

	privateKeyEncrypted, err := store.encryptPrivateKey(masterKey, newPasswordKey, privateKey, publicKey)
	if err != nil {
		return err
	}

	store.SetPasswordHash(newPasswordHash[:])
	store.SetMasterKeyEncrypted(masterKeyEncrypted)
	store.SetPrivateKeyEncrypted(privateKeyEncrypted)

	err = store.SaveToFile()
	if err != nil {
		return err
	}

	return nil
}

func (store *Keystore) MainAccount() *sdk.Account {
	return store.GetAccountByIndex(0)
}

func (store *Keystore) NewAccount() *sdk.Account {
	// create sub account
	privateKey, publicKey, err := crypto.GenerateSubKeyPair(
		store.SubAccountsCount+1, store.masterKey, store.accounts[0].PrivateKey())
	if err != nil {
		panic(fmt.Sprint("New sub account failed,", err))
	}

	account, err := sdk.NewAccount(privateKey, publicKey)
	if err != nil {
		panic(fmt.Sprint("New sub account failed,", err))
	}

	store.accounts = append(store.accounts, account)

	store.SubAccountsCount += 1
	err = store.SaveToFile()
	if err != nil {
		panic(fmt.Sprint("New sub account failed,", err))
	}

	return account
}

func (store *Keystore) GetAccounts() []*sdk.Account {
	return store.accounts
}

func (store *Keystore) GetAccountByIndex(index int) *sdk.Account {
	if index < 0 || index > len(store.accounts)-1 {
		return nil
	}
	return store.accounts[index]
}

func (store *Keystore) GetAccountByProgramHash(programHash *common.Uint168) *sdk.Account {
	if programHash == nil {
		return nil
	}
	for _, account := range store.accounts {
		if *account.ProgramHash() == *programHash {
			return account
		}
	}
	return nil
}

func (store *Keystore) encryptMasterKey(passwordKey, masterKey []byte) ([]byte, error) {
	iv, err := store.GetIV()
	if err != nil {
		return nil, err
	}

	masterKeyEncrypted, err := crypto.AesEncrypt(masterKey, passwordKey, iv)
	if err != nil {
		return nil, err
	}

	return masterKeyEncrypted, nil
}

func (store *Keystore) decryptMasterKey(passwordKey []byte) (masterKey []byte, err error) {
	iv, err := store.GetIV()
	if err != nil {
		return nil, err
	}

	masterKeyEncrypted, err := store.GetMasterKeyEncrypted()
	if err != nil {
		return nil, err
	}

	masterKey, err = crypto.AesDecrypt(masterKeyEncrypted, passwordKey, iv)
	if err != nil {
		return nil, err
	}

	return masterKey, nil
}

func (store *Keystore) encryptPrivateKey(masterKey, passwordKey, privateKey []byte, publicKey *crypto.PublicKey) ([]byte, error) {
	decryptedPrivateKey := make([]byte, 96)

	publicKeyBytes, err := publicKey.EncodePoint(false)
	if err != nil {
		return nil, err
	}
	for i := 1; i <= 64; i++ {
		decryptedPrivateKey[i-1] = publicKeyBytes[i]
	}
	for i := len(privateKey) - 1; i >= 0; i-- {
		decryptedPrivateKey[96+i-len(privateKey)] = privateKey[i]
	}

	iv, err := store.GetIV()
	if err != nil {
		return nil, err
	}

	encryptedPrivateKey, err := crypto.AesEncrypt(decryptedPrivateKey, masterKey, iv)
	if err != nil {
		return nil, err
	}
	return encryptedPrivateKey, nil
}

func (store *Keystore) decryptPrivateKey(masterKey, passwordKey []byte) ([]byte, *crypto.PublicKey, error) {
	privateKeyEncrypted, err := store.GetPrivetKeyEncrypted()
	if err != nil {
		return nil, nil, err
	}
	if len(privateKeyEncrypted) != 96 {
		return nil, nil, errors.New("invalid encrypted private key")
	}

	iv, err := store.GetIV()
	if err != nil {
		return nil, nil, err
	}

	keyPair, err := crypto.AesDecrypt(privateKeyEncrypted, masterKey, iv)
	if err != nil {
		return nil, nil, err
	}
	privateKey := keyPair[64:96]

	return privateKey, crypto.NewPubKey(privateKey), nil
}

func (store *Keystore) FromJson(str string, password string) error {
	file := new(KeystoreFile)
	file.FromJson(str)
	return store.initKeystore(file, []byte(password))
}

func (store *Keystore) Json() (string, error) {
	return store.KeystoreFile.Json()
}
