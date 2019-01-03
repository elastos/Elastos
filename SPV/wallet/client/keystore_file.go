package client

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"os"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	KeystoreFilename = "keystore.dat"
)

type KeystoreFile struct {
	sync.Mutex

	Version string

	IV                  string
	PasswordHash        string
	MasterKeyEncrypted  string
	PrivateKeyEncrypted string

	SubAccountsCount int
}

func CreateKeystoreFile() (*KeystoreFile, error) {
	if info, err := os.Stat(KeystoreFilename); info != nil || os.IsExist(err) {
		return nil, errors.New("key store file already exist")
	}

	file := &KeystoreFile{
		Version: KeystoreVersion,
	}

	return file, nil
}

func OpenKeystoreFile() (*KeystoreFile, error) {

	file := &KeystoreFile{}

	err := file.LoadFromFile()
	if err != nil {
		return nil, err
	}

	return file, nil
}

func (store *KeystoreFile) SetIV(iv []byte) {
	store.IV = common.BytesToHexString(iv)
}

func (store *KeystoreFile) SetPasswordHash(passwordHash []byte) {
	store.PasswordHash = common.BytesToHexString(passwordHash)
}

func (store *KeystoreFile) SetMasterKeyEncrypted(masterKeyEncrypted []byte) {
	store.MasterKeyEncrypted = common.BytesToHexString(masterKeyEncrypted)
}

func (store *KeystoreFile) SetPrivateKeyEncrypted(privateKeyEncrypted []byte) {
	store.PrivateKeyEncrypted = common.BytesToHexString(privateKeyEncrypted)
}

func (store *KeystoreFile) GetIV() ([]byte, error) {

	iv, err := common.HexStringToBytes(store.IV)
	if err != nil {
		return nil, err
	}

	return iv, nil
}

func (store *KeystoreFile) GetPasswordHash() ([]byte, error) {

	passwordHash, err := common.HexStringToBytes(store.PasswordHash)
	if err != nil {
		return nil, err
	}

	return passwordHash, nil
}

func (store *KeystoreFile) GetMasterKeyEncrypted() ([]byte, error) {

	masterKeyEncrypted, err := common.HexStringToBytes(store.MasterKeyEncrypted)
	if err != nil {
		return nil, err
	}

	return masterKeyEncrypted, nil
}

func (store *KeystoreFile) GetPrivetKeyEncrypted() ([]byte, error) {

	privateKeyEncrypted, err := common.HexStringToBytes(store.PrivateKeyEncrypted)
	if err != nil {
		return nil, err
	}

	return privateKeyEncrypted, nil
}

func (store *KeystoreFile) LoadFromFile() error {
	store.Lock()
	defer store.Unlock()

	if _, err := os.Stat(KeystoreFilename); err != nil {
		return errors.New("keystore file not exist")
	}

	file, err := os.OpenFile(KeystoreFilename, os.O_RDONLY, 0666)
	if err != nil {
		return err
	}

	data, err := ioutil.ReadAll(file)
	if err != nil {
		return err
	}

	err = json.Unmarshal(data, store)
	if err != nil {
		return err
	}

	return nil
}

func (store *KeystoreFile) SaveToFile() error {
	store.Lock()
	defer store.Unlock()

	file, err := os.OpenFile(KeystoreFilename, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0666)
	if err != nil {
		return err
	}

	data, err := json.Marshal(*store)
	if err != nil {
		return err
	}

	_, err = file.Write(data)
	if err != nil {
		return err
	}

	return nil
}

func (store *KeystoreFile) Json() (string, error) {
	store.Lock()
	defer store.Unlock()

	data, err := json.Marshal(*store)
	if err != nil {
		return "", err
	}

	return string(data), nil
}

func (store *KeystoreFile) FromJson(str string) error {
	store.Lock()
	defer store.Unlock()

	return json.Unmarshal([]byte(str), store)
}
