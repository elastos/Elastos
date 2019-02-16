package account

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"math/rand"
	"os"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/signal"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"
)

type Client interface {
	Sign(txn *types.Transaction) error

	ContainsAccount(pubKey *crypto.PublicKey) bool
	CreateAccount() (*Account, error)
	DeleteAccount(programHash common.Uint168) error
	GetAccount(pubKey *crypto.PublicKey) (*Account, error)
	GetDefaultAccount() (*Account, error)
	GetAccountByProgramHash(programHash common.Uint168) *Account
	GetAccounts() []*Account
}

type ClientImpl struct {
	mu sync.Mutex

	path      string
	iv        []byte
	masterKey []byte

	mainAccount common.Uint160
	accounts    map[common.Uint160]*Account

	FileStore
}

func Create(path string, password []byte) (*ClientImpl, error) {
	client := NewClient(path, password, true)
	if client == nil {
		return nil, errors.New("create account failed")
	}
	account, err := client.CreateAccount()
	if err != nil {
		return nil, err
	}

	client.mainAccount = account.ProgramHash.ToCodeHash()

	return client, nil
}

func Add(path string, password []byte) (*ClientImpl, error) {
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

func Open(path string, password []byte) (*ClientImpl, error) {
	client := NewClient(path, password, false)
	if client == nil {
		return nil, errors.New("open wallet failed")
	}
	if err := client.LoadAccounts(); err != nil {
		return nil, errors.New("load accounts failed")
	}

	return client, nil
}

func (cl *ClientImpl) Sign(txn *types.Transaction) (*types.Transaction, error) {
	// Get sign type
	signType, err := crypto.GetScriptType(txn.Programs[0].Code)
	if err != nil {
		return nil, err
	}
	// Look up transaction type
	if signType == vm.CHECKSIG {
		// Sign single transaction
		txn, err = cl.signStandardTransaction(txn)
		if err != nil {
			return nil, err
		}
	} else if signType == vm.CHECKMULTISIG {
		// Sign multi sign transaction
		txn, err = cl.signMultiSignTransaction(txn)
		if err != nil {
			return nil, err
		}
	}

	return txn, nil
}

func (cl *ClientImpl) signStandardTransaction(txn *types.Transaction) (*types.Transaction, error) {
	code := txn.Programs[0].Code
	acct := cl.GetAccountByCodeHash(*common.ToCodeHash(code))
	if acct == nil {
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
	// Add signature
	txn.Programs[0].Parameter = buf.Bytes()

	return txn, nil
}

func (cl *ClientImpl) signMultiSignTransaction(txn *types.Transaction) (*types.Transaction, error) {
	code := txn.Programs[0].Code
	param := txn.Programs[0].Parameter
	// Check if current user is a valid signer
	programHashes, err := GetSigners(code)
	if err != nil {
		return nil, err
	}
	var signerIndex = -1
	var acc *Account
	for i, hash := range programHashes {
		acc := cl.GetAccountByCodeHash(*hash)
		if acc != nil {
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
	txn.SerializeUnsigned(buf)
	txn.Programs[0].Parameter, err = crypto.AppendSignature(signerIndex, signature, buf.Bytes(), code, param)
	if err != nil {
		return nil, err
	}

	return txn, nil
}

func (cl *ClientImpl) GetDefaultAccount() (*Account, error) {
	return cl.GetAccountByCodeHash(cl.mainAccount), nil
}

func (cl *ClientImpl) GetAccount(pubKey *crypto.PublicKey) (*Account, error) {
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, errors.New("CreateStandardContract failed")
	}
	return cl.GetAccountByCodeHash(*signatureContract.ToCodeHash()), nil
}

func (cl *ClientImpl) GetAccountByCodeHash(codeHash common.Uint160) *Account {
	cl.mu.Lock()
	defer cl.mu.Unlock()
	if account, ok := cl.accounts[codeHash]; ok {
		return account
	}
	return nil
}

func NewClient(path string, password []byte, create bool) *ClientImpl {
	client := &ClientImpl{
		path:      path,
		accounts:  map[common.Uint160]*Account{},
		FileStore: FileStore{path: path},
	}

	go client.HandleInterrupt()

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
			log.Error(err)
			return nil
		}

		pwdhash := sha256.Sum256(passwordKey)
		if err := client.SaveStoredData("PasswordHash", pwdhash[:]); err != nil {
			log.Error(err)
			return nil
		}
		if err := client.SaveStoredData("IV", client.iv[:]); err != nil {
			log.Error(err)
			return nil
		}

		aesmk, err := crypto.AesEncrypt(client.masterKey[:], passwordKey, client.iv)
		if err != nil {
			log.Error(err)
			return nil
		}
		if err := client.SaveStoredData("MasterKey", aesmk); err != nil {
			log.Error(err)
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
func (cl *ClientImpl) CreateAccount() (*Account, error) {
	account, err := NewAccount()
	if err != nil {
		return nil, err
	}
	if err := cl.SaveAccount(account); err != nil {
		return nil, err
	}

	return account, nil
}

// SaveAccount saves a Account to memory and db
func (cl *ClientImpl) SaveAccount(ac *Account) error {
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
	for i := len(ac.PrivateKey) - 1; i >= 0; i-- {
		decryptedPrivateKey[96+i-len(ac.PrivateKey)] = ac.PrivateKey[i]
	}
	encryptedPrivateKey, err := cl.EncryptPrivateKey(decryptedPrivateKey)
	if err != nil {
		return err
	}
	common.ClearBytes(decryptedPrivateKey)

	// save Account keys to db
	err = cl.SaveAccountData(ac.ProgramHash.Bytes(), ac.RedeemScript, encryptedPrivateKey)
	if err != nil {
		return err
	}

	return nil
}

func (cl *ClientImpl) GetAccounts() []*Account {
	accounts := make([]*Account, 0, len(cl.accounts))
	for _, account := range cl.accounts {
		accounts = append(accounts, account)
	}

	// comment below to make sure the main account is at the top
	//sort.Slice(accounts, func(i, j int) bool {
	//	return bytes.Compare(accounts[i].ProgramHash[:],
	//		accounts[j].ProgramHash[:]) < 0
	//})

	return accounts
}

// LoadAccounts loads all accounts from db to memory
func (cl *ClientImpl) LoadAccounts() error {
	accounts := map[common.Uint160]*Account{}

	storeAddresses, err := cl.LoadAccountData()
	if err != nil {
		return err
	}
	for _, a := range storeAddresses {
		encryptedKeyPair, _ := common.HexStringToBytes(a.PrivateKeyEncrypted)
		keyPair, err := cl.DecryptPrivateKey(encryptedKeyPair)
		if err != nil {
			log.Error(err)
			continue
		}
		privateKey := keyPair[64:96]
		ac, err := NewAccountWithPrivateKey(privateKey)
		if err != nil {
			return err
		}
		accounts[ac.ProgramHash.ToCodeHash()] = ac

		if a.Type == MAINACCOUNT {
			cl.mainAccount = ac.ProgramHash.ToCodeHash()
		}
	}

	cl.accounts = accounts
	return nil
}

func (cl *ClientImpl) EncryptPrivateKey(prikey []byte) ([]byte, error) {
	enc, err := crypto.AesEncrypt(prikey, cl.masterKey, cl.iv)
	if err != nil {
		return nil, err
	}

	return enc, nil
}

func (cl *ClientImpl) DecryptPrivateKey(prikey []byte) ([]byte, error) {
	if prikey == nil {
		return nil, errors.New("The PriKey is nil")
	}
	if len(prikey) != 96 {
		return nil, errors.New("The len of PriKeyEnc is not 96bytes")
	}

	dec, err := crypto.AesDecrypt(prikey, cl.masterKey, cl.iv)
	if err != nil {
		return nil, err
	}

	return dec, nil
}

func (cl *ClientImpl) verifyPasswordKey(passwordKey []byte) bool {
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

func (cl *ClientImpl) HandleInterrupt() {
	interrupt := signal.NewInterrupt()
	select {
	case <-interrupt.C:
		// hold the mutex lock to prevent any wallet db changes
		cl.FileStore.Lock()
		os.Exit(0)
	}
}

func SignBySigner(txn *types.Transaction, acc *Account) ([]byte, error) {
	log.Debug()
	buf := new(bytes.Buffer)
	txn.SerializeUnsigned(buf)
	signature, err := crypto.Sign(acc.PrivKey(), buf.Bytes())
	if err != nil {
		return nil, errors.New("[Signature],SignBySigner failed.")
	}
	return signature, nil
}
