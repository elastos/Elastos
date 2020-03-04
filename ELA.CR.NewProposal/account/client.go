// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package account

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"math/rand"
	"os"
	"sort"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils"
	"github.com/elastos/Elastos.ELA/utils/signal"
	"github.com/elastos/Elastos.ELA/vm"
)

type Client struct {
	mu sync.Mutex

	path      string
	iv        []byte
	masterKey []byte

	mainAccount common.Uint160
	accounts    map[common.Uint160]*Account

	FileStore
}

func Create(path string, password []byte) (*Client, error) {
	return createClient(path, password, func(cl *Client) (*Account, error) {
		return cl.CreateAccount()
	})
}

func CreateFromAccount(path string, password []byte,
	account *Account) (*Client, error) {
	return createClient(path, password, func(cl *Client) (*Account, error) {
		if err := cl.SaveAccount(account); err != nil {
			return nil, err
		}
		return account, nil
	})
}

func createClient(path string, password []byte,
	accountOp func(*Client) (*Account, error)) (*Client, error) {
	client := NewClient(path, password, true)
	if client == nil {
		return nil, errors.New("create account failed")
	}
	account, err := accountOp(client)
	if err != nil {
		return nil, err
	}
	client.mainAccount = account.ProgramHash.ToCodeHash()

	return client, nil
}

func Add(path string, password []byte) (*Client, error) {
	client := NewClient(path, password, false)
	if client == nil {
		return nil, errors.New("add account failed")
	}
	_, err := client.CreateAccount()
	if err != nil {
		return nil, err
	}

	return client, nil
}

func AddMultiSig(path string, password []byte, m int, pubKeys []*crypto.PublicKey) (*Account, error) {
	exist := utils.FileExisted(path)
	client := NewClient(path, password, !exist)
	if client == nil {
		return nil, errors.New("add multi-signature account failed")
	}

	return client.CreateMultiSigAccount(m, pubKeys)
}

func Open(path string, password []byte) (*Client, error) {
	client := NewClient(path, password, false)
	if client == nil {
		return nil, errors.New("open wallet failed")
	}
	if err := client.LoadAccounts(); err != nil {
		return nil, err
	}

	return client, nil
}

func (cl *Client) Sign(txn *types.Transaction) (*types.Transaction, error) {
	var signedPrograms []*pg.Program
	for _, program := range txn.Programs {
		// Get sign type
		signType, err := crypto.GetScriptType(program.Code)
		if err != nil {
			return nil, err
		}
		// Look up transaction type
		if signType == vm.CHECKSIG {
			// Sign single transaction
			signedProgram, err := SignStandardTransaction(txn, program, cl.accounts)
			if err != nil {
				return nil, err
			}
			signedPrograms = append(signedPrograms, signedProgram)
		} else if signType == vm.CHECKMULTISIG {
			// Sign multi sign transaction
			signedProgram, err := SignMultiSignTransaction(txn, program, cl.accounts)
			if err != nil {
				return nil, err
			}
			signedPrograms = append(signedPrograms, signedProgram)
		}
	}
	txn.Programs = signedPrograms

	return txn, nil
}

func (cl *Client) GetMainAccount() *Account {
	return cl.GetAccountByCodeHash(cl.mainAccount)
}

func (cl *Client) GetAccount(pubKey *crypto.PublicKey) (*Account, error) {
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, errors.New("CreateStandardContract failed")
	}
	return cl.GetAccountByCodeHash(*signatureContract.ToCodeHash()), nil
}

func (cl *Client) GetAccountByCodeHash(codeHash common.Uint160) *Account {
	cl.mu.Lock()
	defer cl.mu.Unlock()
	if account, ok := cl.accounts[codeHash]; ok {
		return account
	}
	return nil
}

func NewClient(path string, password []byte, create bool) *Client {
	client := &Client{
		path:      path,
		accounts:  map[common.Uint160]*Account{},
		FileStore: FileStore{path: path},
	}

	passwordKey := crypto.ToAesKey(password)
	if create {
		//create new client
		client.iv = make([]byte, 16)
		client.masterKey = make([]byte, 32)

		//generate random number for iv/masterkey
		r := rand.New(rand.NewSource(time.Now().UnixNano()))
		for i := 0; i < 16; i++ {
			client.iv[i] = byte(r.Intn(256))
		}
		for i := 0; i < 32; i++ {
			client.masterKey[i] = byte(r.Intn(256))
		}

		//new client store (build DB)
		client.BuildDatabase(path)

		if err := client.SaveStoredData("Version", []byte(KeystoreVersion)); err != nil {
			fmt.Println("error: failed to save Version")
			return nil
		}

		pwdhash := sha256.Sum256(passwordKey)
		if err := client.SaveStoredData("PasswordHash", pwdhash[:]); err != nil {
			fmt.Println("error: failed to save PasswordHash")
			return nil
		}
		if err := client.SaveStoredData("IV", client.iv[:]); err != nil {
			fmt.Println("error: failed to save IV")
			return nil
		}

		aesmk, err := crypto.AesEncrypt(client.masterKey[:], passwordKey, client.iv)
		if err != nil {
			fmt.Println("error: failed to encrypt the passoword", err.Error())
			return nil
		}
		if err := client.SaveStoredData("MasterKey", aesmk); err != nil {
			fmt.Println("error: failed to save MasterKey")
			return nil
		}

	} else {
		if ok := client.verifyPasswordKey(passwordKey); !ok {
			return nil
		}
		var err error
		client.iv, err = client.LoadStoredData("IV")
		if err != nil {
			fmt.Println("error: failed to load iv")
			return nil
		}
		encryptedMasterKey, err := client.LoadStoredData("MasterKey")
		if err != nil {
			fmt.Println("error: failed to load master key")
			return nil
		}
		client.masterKey, err = crypto.AesDecrypt(encryptedMasterKey, passwordKey, client.iv)
		if err != nil {
			fmt.Println("error: failed to decrypt master key")
			return nil
		}
	}
	common.ClearBytes(passwordKey)

	return client
}

// CreateAccount create a new Account then save it
func (cl *Client) CreateAccount() (*Account, error) {
	account, err := NewAccount()
	if err != nil {
		return nil, err
	}
	if err := cl.SaveAccount(account); err != nil {
		return nil, err
	}

	return account, nil
}

func (cl *Client) CreateMultiSigAccount(m int, pubKeys []*crypto.PublicKey) (*Account, error) {
	account, err := NewMultiSigAccount(m, pubKeys)
	if err != nil {
		return nil, err
	}
	if err = cl.SaveAccountData(&account.ProgramHash, account.RedeemScript, nil); err != nil {
		return nil, err
	}

	return account, nil
}

// SaveAccount saves a Account to memory and db
func (cl *Client) SaveAccount(ac *Account) error {
	cl.mu.Lock()
	defer cl.mu.Unlock()

	// save Account to memory
	cl.accounts[ac.ProgramHash.ToCodeHash()] = ac

	decryptedPrivateKey := make([]byte, 96)
	temp, err := ac.PublicKey.EncodePoint(false)
	if err != nil {
		return err
	}
	for i := 1; i <= 64; i++ {
		decryptedPrivateKey[i-1] = temp[i]
	}
	for i := len(ac.PrivKey()) - 1; i >= 0; i-- {
		decryptedPrivateKey[96+i-len(ac.PrivKey())] = ac.PrivKey()[i]
	}
	encryptedPrivateKey, err := cl.EncryptPrivateKey(decryptedPrivateKey)
	if err != nil {
		return err
	}
	common.ClearBytes(decryptedPrivateKey)

	// save Account keys to db
	err = cl.SaveAccountData(&ac.ProgramHash, ac.RedeemScript, encryptedPrivateKey)
	if err != nil {
		return err
	}

	return nil
}

func (cl *Client) GetAccounts() []*Account {
	accounts := make([]*Account, 0, len(cl.accounts))
	for _, account := range cl.accounts {
		accounts = append(accounts, account)
	}

	sort.Slice(accounts, func(i, j int) bool {
		if cl.mainAccount == accounts[i].ProgramHash.ToCodeHash() || accounts[i].Address < accounts[j].Address {
			return true
		}
		return false
	})

	return accounts
}

// LoadAccounts loads all accounts from db to memory
func (cl *Client) LoadAccounts() error {
	accounts := map[common.Uint160]*Account{}

	storeAccounts, err := cl.LoadAccountData()
	if err != nil {
		return err
	}
	for _, a := range storeAccounts {
		phBytes, err := common.HexStringToBytes(a.ProgramHash)
		programHash, err := common.Uint168FromBytes(phBytes)
		if err != nil {
			return err
		}
		prefixType := contract.GetPrefixType(*programHash)
		if prefixType == contract.PrefixStandard {
			if a.PrivateKeyEncrypted != "" {
				encryptedKeyPair, _ := common.HexStringToBytes(a.PrivateKeyEncrypted)
				keyPair, err := cl.DecryptPrivateKey(encryptedKeyPair)
				if err != nil {
					return err
				}
				privateKey := keyPair[64:96]
				ac, err := NewAccountWithPrivateKey(privateKey)
				if err != nil {
					return err
				}
				accounts[programHash.ToCodeHash()] = ac
			} else {
				rs, _ := common.HexStringToBytes(a.RedeemScript)
				ac := &Account{
					PrivateKey:   nil,
					PublicKey:    nil,
					ProgramHash:  *programHash,
					RedeemScript: rs,
					Address:      a.Address,
				}
				accounts[programHash.ToCodeHash()] = ac
			}
		} else if prefixType == contract.PrefixMultiSig {
			rs, _ := common.HexStringToBytes(a.RedeemScript)
			ac := &Account{
				PrivateKey:   nil,
				PublicKey:    nil,
				ProgramHash:  *programHash,
				RedeemScript: rs,
				Address:      a.Address,
			}
			accounts[programHash.ToCodeHash()] = ac
		}

		if a.Type == MAINACCOUNT {
			cl.mainAccount = programHash.ToCodeHash()
		}
	}

	cl.accounts = accounts
	return nil
}

func (cl *Client) EncryptPrivateKey(prikey []byte) ([]byte, error) {
	enc, err := crypto.AesEncrypt(prikey, cl.masterKey, cl.iv)
	if err != nil {
		return nil, err
	}

	return enc, nil
}

func (cl *Client) DecryptPrivateKey(prikey []byte) ([]byte, error) {
	if prikey == nil {
		return nil, errors.New("the private key is nil")
	}
	if len(prikey) != 96 {
		return nil, errors.New("the len of private key is not 96bytes")
	}

	dec, err := crypto.AesDecrypt(prikey, cl.masterKey, cl.iv)
	if err != nil {
		return nil, err
	}

	return dec, nil
}

func (cl *Client) verifyPasswordKey(passwordKey []byte) bool {
	savedPasswordHash, err := cl.LoadStoredData("PasswordHash")
	if err != nil {
		fmt.Println("error: failed to load password hash")
		return false
	}
	if savedPasswordHash == nil {
		fmt.Println("error: saved password hash is nil")
		return false
	}
	passwordHash := sha256.Sum256(passwordKey)
	///ClearBytes(passwordKey, len(passwordKey))
	if !bytes.Equal(savedPasswordHash, passwordHash[:]) {
		fmt.Println("error: password wrong")
		return false
	}
	return true
}

func (cl *Client) HandleInterrupt() {
	interrupt := signal.NewInterrupt()
	select {
	case <-interrupt.C:
		// hold the mutex lock to prevent any wallet db changes
		cl.FileStore.Lock()
		os.Exit(0)
	}
}

func SignBySigner(txn *types.Transaction, acc *Account) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := txn.SerializeUnsigned(buf); err != nil {
		return nil, err
	}
	signature, err := crypto.Sign(acc.PrivKey(), buf.Bytes())
	if err != nil {
		return nil, errors.New("[Signature],SignBySigner failed")
	}
	return signature, nil
}

func SignStandardTransaction(txn *types.Transaction, program *pg.Program,
	accounts map[common.Uint160]*Account) (*pg.Program, error) {
	code := program.Code
	acct, ok := accounts[*common.ToCodeHash(code)]
	if !ok {
		return nil, errors.New("no available account in wallet to do single-sign")
	}

	// Sign transaction
	signature, err := SignBySigner(txn, acct)
	if err != nil {
		return nil, err
	}
	// Add verify program for transaction
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)

	signedProgram := &pg.Program{
		Code:      code,
		Parameter: buf.Bytes(),
	}

	return signedProgram, nil
}

func SignMultiSignTransaction(txn *types.Transaction, program *pg.Program,
	accounts map[common.Uint160]*Account) (*pg.Program, error) {
	code := program.Code
	param := program.Parameter
	// Check if current user is a valid signer
	codeHashes, err := GetSigners(code)
	if err != nil {
		return nil, err
	}
	var signerIndex = -1
	var acc *Account
	for i, hash := range codeHashes {
		var ok bool
		acc, ok = accounts[*hash]
		if ok {
			signerIndex = i
			break
		}
	}
	if signerIndex == -1 {
		return nil, errors.New("no available account detected")
	}
	// Sign transaction
	signature, err := SignBySigner(txn, acc)
	if err != nil {
		return nil, err
	}

	// Append signature
	buf := new(bytes.Buffer)
	if err := txn.SerializeUnsigned(buf); err != nil {
		return nil, err
	}
	parameter, err := crypto.AppendSignature(signerIndex, signature, buf.Bytes(), code, param)
	if err != nil {
		return nil, err
	}

	signedProgram := &pg.Program{
		Code:      code,
		Parameter: parameter,
	}

	return signedProgram, nil
}
