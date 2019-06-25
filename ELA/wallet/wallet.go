package wallet

import (
	"errors"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/utils"
	"path/filepath"

	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	WalletVersion = "0.0.1"
)

type AddressInfo struct {
	address string
	code    []byte
}

type Wallet struct {
	addressBook map[string]*AddressInfo
	FileStore
}

func (w *Wallet) LoadAddresses() error {
	addressBook := make(map[string]*AddressInfo, 0)
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
		addressBook[addressData.Address] = addressInfo
	}
	w.addressBook = addressBook

	return nil
}

func (w *Wallet) ImportPubkey(pubKey []byte) error {
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

	return w.SaveAddressData(address, sc.Code)
}

func (w *Wallet) ImportAddress(address string) error {
	_, err := common.Uint168FromAddress(address)
	if err != nil {
		return errors.New("invalid address")
	}

	return w.SaveAddressData(address, nil)
}

func New(dataDir string) *Wallet {
	walletPath := filepath.Join(dataDir, "wallet.dat")
	wallet := Wallet{
		addressBook: make(map[string]*AddressInfo, 0),
		FileStore:   FileStore{path: walletPath},
	}

	exist := utils.FileExisted(walletPath)
	if !exist {
		wallet.BuildDatabase(walletPath)
		wallet.SaveStoredData("Version", []byte(WalletVersion))
		wallet.SaveStoredData("Height", []byte("0"))
	} else {
		wallet.LoadAddresses()
	}

	return &wallet
}
