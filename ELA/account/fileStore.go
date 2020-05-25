// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package account

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
)

type AccountData struct {
	Address             string
	ProgramHash         string
	RedeemScript        string
	PrivateKeyEncrypted string
	Type                string
}

type FileData struct {
	Version      string
	PasswordHash string
	IV           string
	MasterKey    string
	Account      []AccountData
}

type FileStore struct {
	// this lock could be hold by readDB, writeDB and interrupt signals.
	sync.Mutex

	data FileData
	file *os.File
	path string
}

// Caller holds the lock and reads bytes from DB, then close the DB and release the lock
func (cs *FileStore) readDB() ([]byte, error) {
	cs.Lock()
	defer cs.Unlock()
	defer cs.closeDB()

	var err error
	cs.file, err = os.OpenFile(cs.path, os.O_RDONLY, 0400)
	if err != nil {
		return nil, err
	}

	if cs.file != nil {
		data, err := ioutil.ReadAll(cs.file)
		if err != nil {
			return nil, err
		}
		return data, nil
	} else {
		return nil, errors.New("[readDB] file handle is nil")
	}
}

// Caller holds the lock and writes bytes to DB, then close the DB and release the lock
func (cs *FileStore) writeDB(data []byte) error {
	cs.Lock()
	defer cs.Unlock()
	defer cs.closeDB()

	var err error
	cs.file, err = os.OpenFile(cs.path, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return err
	}

	if cs.file != nil {
		cs.file.Write(data)
	}

	return nil
}

func (cs *FileStore) closeDB() {
	if cs.file != nil {
		cs.file.Close()
		cs.file = nil
	}
}

func (cs *FileStore) BuildDatabase(path string) {
	if _, err := os.Stat(path); err == nil || os.IsExist(err) {
		fmt.Println(path + " file already exist")
		os.Exit(1)
	}
	jsonBlob, err := json.Marshal(cs.data)
	if err != nil {
		fmt.Println("Build DataBase Error")
		os.Exit(1)
	}
	cs.writeDB(jsonBlob)
}

func (cs *FileStore) SaveAccountData(programHash *common.Uint168, redeemScript []byte,
	encryptedPrivateKey []byte) error {
	JSONData, err := cs.readDB()
	if err != nil {
		return errors.New("error: reading db")
	}
	if err := json.Unmarshal(JSONData, &cs.data); err != nil {
		return errors.New("error: unmarshal db")
	}

	var accountType string
	if len(cs.data.Account) == 0 {
		accountType = MAINACCOUNT
	} else {
		accountType = SUBACCOUNT
	}

	addr, err := programHash.ToAddress()
	if err != nil {
		return errors.New("invalid address")
	}
	a := AccountData{
		Address:             addr,
		ProgramHash:         common.BytesToHexString(programHash.Bytes()),
		RedeemScript:        common.BytesToHexString(redeemScript),
		PrivateKeyEncrypted: common.BytesToHexString(encryptedPrivateKey),
		Type:                accountType,
	}

	for _, v := range cs.data.Account {
		if a.ProgramHash == v.ProgramHash {
			return errors.New("account already exists")
		}
	}
	cs.data.Account = append(cs.data.Account, a)

	JSONBlob, err := json.Marshal(cs.data)
	if err != nil {
		return errors.New("error: marshal db")
	}
	cs.writeDB(JSONBlob)

	return nil
}

func (cs *FileStore) DeleteAccountData(address string) error {
	JSONData, err := cs.readDB()
	if err != nil {
		return errors.New("error: reading db")
	}
	if err := json.Unmarshal(JSONData, &cs.data); err != nil {
		return errors.New("error: unmarshal db")
	}

	for i, v := range cs.data.Account {
		if address == v.Address {
			if v.Type == MAINACCOUNT {
				return errors.New("can't remove main account")
			}
			cs.data.Account = append(cs.data.Account[:i], cs.data.Account[i+1:]...)
		}
	}

	JSONBlob, err := json.Marshal(cs.data)
	if err != nil {
		return errors.New("error: marshal db")
	}
	cs.writeDB(JSONBlob)

	return nil
}

func (cs *FileStore) LoadAccountData() ([]AccountData, error) {
	JSONData, err := cs.readDB()
	if err != nil {
		return nil, errors.New("error: reading db")
	}
	if err := json.Unmarshal(JSONData, &cs.data); err != nil {
		return nil, errors.New("error: unmarshal db")
	}
	return cs.data.Account, nil
}

func (cs *FileStore) SaveStoredData(name string, value []byte) error {
	JSONData, err := cs.readDB()
	if err != nil {
		return errors.New("error: reading db")
	}
	if err := json.Unmarshal(JSONData, &cs.data); err != nil {
		return errors.New("error: unmarshal db")
	}

	hexValue := common.BytesToHexString(value)
	switch name {
	case "Version":
		cs.data.Version = string(value)
	case "IV":
		cs.data.IV = hexValue
	case "MasterKey":
		cs.data.MasterKey = hexValue
	case "PasswordHash":
		cs.data.PasswordHash = hexValue

	}
	JSONBlob, err := json.Marshal(cs.data)
	if err != nil {
		return errors.New("error: marshal db")
	}
	cs.writeDB(JSONBlob)

	return nil
}

func (cs *FileStore) LoadStoredData(name string) ([]byte, error) {
	JSONData, err := cs.readDB()
	if err != nil {
		return nil, errors.New("error: reading db")
	}
	if err := json.Unmarshal(JSONData, &cs.data); err != nil {
		return nil, errors.New("error: unmarshal db")
	}
	switch name {
	case "Version":
		return []byte(cs.data.Version), nil
	case "IV":
		return common.HexStringToBytes(cs.data.IV)
	case "MasterKey":
		return common.HexStringToBytes(cs.data.MasterKey)
	case "PasswordHash":
		return common.HexStringToBytes(cs.data.PasswordHash)
	}

	return nil, errors.New("can't find the key: " + name)
}

func (cs *FileStore) SetPath(path string) {
	cs.Lock()
	defer cs.Unlock()

	cs.path = path
}

func GetWalletAccountData(walletPath string) ([]AccountData, error) {
	var fileStore FileStore
	fileStore.SetPath(walletPath)
	storeAccounts, err := fileStore.LoadAccountData()
	if err != nil {
		return nil, err
	}

	return storeAccounts, nil
}

func GetWalletMainAccountData(walletPath string) (*AccountData, error) {
	storeAccounts, err := GetWalletAccountData(walletPath)
	if err != nil {
		return nil, err
	}
	for _, a := range storeAccounts {
		if a.Type == MAINACCOUNT {
			return &a, nil
		}
	}

	return nil, errors.New("no main account found")
}
