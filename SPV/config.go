package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
	"io/ioutil"
	"os"
)

const (
	ConfigFilename = "./config.json"
)

var config = loadConfig()

type Config struct {
	Magic         uint32
	SeedList      []string
	NodePort      uint16 // node port for public peers to provide services.
	Foundation    string
	PrintLevel    int
	MaxLogsSize   int64
	MaxPerLogSize int64
	JsonRpcPort   uint16

	foundation *common.Uint168
}

func loadConfig() *Config {
	data, err := ioutil.ReadFile(ConfigFilename)
	if err != nil {
		fmt.Printf("Read config file error %s", err)
		os.Exit(-1)
	}
	// Remove the UTF-8 Byte Order Mark
	data = bytes.TrimPrefix(data, []byte("\xef\xbb\xbf"))

	c := Config{}
	err = json.Unmarshal(data, &c)
	if err != nil {
		fmt.Printf("Read config file error %s", err)
		os.Exit(-1)
	}

	if c.Foundation == "" {
		c.Foundation = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	}

	c.foundation, err = common.Uint168FromAddress(c.Foundation)
	if err != nil {
		fmt.Printf("Parse foundation address error %s", err)
		os.Exit(-1)
	}

	return &c
}

type blockHeader struct {
	*core.Header
}

func (h *blockHeader) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *blockHeader) Bits() uint32 {
	return h.Header.Bits
}

func (h *blockHeader) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *blockHeader) PowHash() common.Uint256 {
	return h.AuxPow.ParBlockHeader.Hash()
}

func newBlockHeader() util.BlockHeader {
	return &blockHeader{Header: &core.Header{}}
}
