package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"

	"github.com/elastos/Elastos.ELA.Utility/common"
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
