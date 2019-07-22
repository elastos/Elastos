// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package wallet

import (
	"errors"
	"path/filepath"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils"
)

var (
	addressBook = make(map[string]*AddressInfo, 0)
)

type AddressInfo struct {
	address string
	code    []byte
}

type Wallet struct {
	*CoinsCheckPoint
	FileStore
}

func (w *Wallet) LoadAddresses() error {
	addrBook := make(map[string]*AddressInfo, 0)
	storeAddresses, err := w.LoadAddressData()
	if err != nil {
		return err
	}
	for _, addressData := range storeAddresses {
		code, err := common.HexStringToBytes(addressData.Code)
		if err != nil {
			return err
		}
		addressInfo := &AddressInfo{
			address: addressData.Address,
			code:    code,
		}
		addrBook[addressData.Address] = addressInfo
	}
	addressBook = addrBook

	return nil
}

func (w *Wallet) ImportPubkey(pubKey []byte, enableUtxoDB bool) error {
	pk, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return errors.New("invalid public key")
	}
	sc, err := contract.CreateStandardContract(pk)
	if err != nil {
		return err
	}
	address, err := sc.ToProgramHash().ToAddress()
	if err != nil {
		return err
	}

	if err := w.SaveAddressData(address, sc.Code); err != nil {
		return err
	}

	addressBook[address] = &AddressInfo{
		address: address,
		code:    sc.Code,
	}

	if enableUtxoDB {
		return nil
	}

	return w.RescanWallet()
}

func (w *Wallet) ImportAddress(address string, enableUtxoDB bool) error {
	_, err := common.Uint168FromAddress(address)
	if err != nil {
		return errors.New("invalid address")
	}

	if err := w.SaveAddressData(address, nil); err != nil {
		return err
	}

	addressBook[address] = &AddressInfo{
		address: address,
		code:    nil,
	}

	if enableUtxoDB {
		return nil
	}

	return w.RescanWallet()
}

func (w *Wallet) ListUnspent(address string, enableUtxoDB bool) (map[common.Uint256][]*blockchain.UTXO,
	error) {
	if enableUtxoDB {
		programHash, err := common.Uint168FromAddress(address)
		if err != nil {
			return nil, err
		}
		unspents, err := Store.GetUnspentsFromProgramHash(*programHash)
		if err != nil {
			return nil, err
		}

		return unspents, nil
	}

	coins := w.ListCoins(address)
	utxos := make([]*blockchain.UTXO, 0)
	for op, coin := range coins {
		utxos = append(utxos, &blockchain.UTXO{
			TxID:  op.TxID,
			Index: uint32(op.Index),
			Value: coin.Output.Value,
		})
	}
	unspents := make(map[common.Uint256][]*blockchain.UTXO, 0)
	unspents[*account.SystemAssetID] = utxos

	return unspents, nil
}

func (w *Wallet) RescanWallet() error {
	bestHeight := Store.GetHeight()
	for i := uint32(0); i <= bestHeight; i++ {
		hash, err := Store.GetBlockHash(i)
		if err != nil {
			return err
		}
		block, err := Store.GetBlock(hash)
		if err != nil {
			return err
		}
		w.OnBlockSaved(&types.DposBlock{
			Block: block,
		})
	}

	return nil
}

func New(dataDir string) *Wallet {
	walletPath := filepath.Join(dataDir, "wallet.dat")
	wallet := Wallet{
		FileStore:       FileStore{path: walletPath},
		CoinsCheckPoint: NewCoinCheckPoint(),
	}

	exist := utils.FileExisted(walletPath)
	if !exist {
		if err := wallet.BuildDatabase(walletPath); err != nil {
			log.Warn("Build wallet failed, " + err.Error())
		}
		if err := wallet.SaveStoredData("Version", []byte(WalletVersion)); err != nil {
			log.Warn("Save version field failed, " + err.Error())
		}
		if err := wallet.SaveStoredData("Height", []byte("0")); err != nil {
			log.Warn("Save height field failed, " + err.Error())
		}
	} else {
		if err := wallet.LoadAddresses(); err != nil {
			log.Warn("Build wallet failed" + err.Error())
		}
	}

	return &wallet
}
