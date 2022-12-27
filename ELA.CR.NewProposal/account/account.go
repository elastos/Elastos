// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package account

import (
	"encoding/json"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

/*
A ELA standard account is a set of private key, public key, redeem script, program hash and address data.
redeem script is (script content length)+(script content)+(script type),
program hash is the sha256 value of redeem script and converted to ripemd160 format with a (Type) prefix.
address is the base58 format of program hash, which is the string value show up on user interface as account address.
With account, you can get the transfer address or sign transaction etc.
*/
type Account struct {
	PrivateKey   []byte
	PublicKey    *crypto.PublicKey
	ProgramHash  common.Uint168
	RedeemScript []byte
	Address      string
}

// String format of Account
type AccountInfo struct {
	PrivateKey   string `json:"PrivateKey"`
	PublicKey    string `json:"PublicKey"`
	ProgramHash  string `json:"ProgramHash"`
	RedeemScript string `json:"RedeemScript"`
	Address      string `json:"Address"`
}

// Create an account instance with private key and public key
func NewAccount() (*Account, error) {
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, err
	}

	programHash := signatureContract.ToProgramHash()
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		PrivateKey:   priKey,
		PublicKey:    pubKey,
		ProgramHash:  *programHash,
		RedeemScript: signatureContract.Code,
		Address:      address,
	}, nil
}

func NewAccountWithPrivateKey(privateKey []byte) (*Account, error) {
	pubKey := crypto.NewPubKey(privateKey)
	signatureContract, err := contract.CreateStandardContract(pubKey)
	if err != nil {
		return nil, err
	}
	programHash := signatureContract.ToProgramHash()
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}
	return &Account{
		PrivateKey:   privateKey,
		PublicKey:    pubKey,
		ProgramHash:  *programHash,
		RedeemScript: signatureContract.Code,
		Address:      address,
	}, nil
}

func NewMultiSigAccount(m int, pubKeys []*crypto.PublicKey) (*Account, error) {
	multiSigContract, err := contract.CreateMultiSigContract(m, pubKeys)
	if err != nil {
		return nil, err
	}

	programHash := multiSigContract.ToProgramHash()
	address, err := programHash.ToAddress()
	if err != nil {
		return nil, err
	}

	return &Account{
		PrivateKey:   nil,
		PublicKey:    nil,
		ProgramHash:  *programHash,
		RedeemScript: multiSigContract.Code,
		Address:      address,
	}, nil
}

// Get account private key
func (ac *Account) PrivKey() []byte {
	return ac.PrivateKey
}

// Get account public key
func (ac *Account) PubKey() *crypto.PublicKey {
	return ac.PublicKey
}

// Sign data with account
func (ac *Account) Sign(data []byte) ([]byte, error) {
	return crypto.Sign(ac.PrivateKey, data)
}

// Convert account to JSON string
func (ac *Account) ToJson() (string, error) {
	pk, err := ac.PublicKey.EncodePoint(true)
	if err != nil {
		return "", err
	}

	info := AccountInfo{
		PrivateKey:   common.BytesToHexString(ac.PrivateKey),
		PublicKey:    common.BytesToHexString(pk),
		ProgramHash:  ac.ProgramHash.String(),
		RedeemScript: common.BytesToHexString(ac.RedeemScript),
		Address:      ac.Address,
	}
	data, err := json.Marshal(&info)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

// Create account from JSON string
func FromJson(data string) (*Account, error) {
	var info AccountInfo
	if err := json.Unmarshal([]byte(data), &info); err != nil {
		return nil, err
	}

	priKey, err := common.HexStringToBytes(info.PrivateKey)
	if err != nil {
		return nil, err
	}

	pubKeyBuf, err := common.HexStringToBytes(info.PublicKey)
	if err != nil {
		return nil, err
	}

	pubKey, err := crypto.DecodePoint(pubKeyBuf)
	if err != nil {
		return nil, err
	}

	hashBuf, err := common.HexStringToBytes(info.ProgramHash)
	if err != nil {
		return nil, err
	}

	hash, err := common.Uint168FromBytes(hashBuf)
	if err != nil {
		return nil, err
	}

	redeem, err := common.HexStringToBytes(info.RedeemScript)
	if err != nil {
		return nil, err
	}

	account := &Account{
		PrivateKey:   priKey,
		PublicKey:    pubKey,
		ProgramHash:  *hash,
		RedeemScript: redeem,
		Address:      info.Address,
	}
	return account, nil
}
