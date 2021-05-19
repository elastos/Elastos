package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
)

const (
	// dataDir defines the folder where to put the database files.
	dataDir = "./data"

	// defaultDebugLevel indicates the default debug level string.
	defaultDebugLevel = "info"

	// configFilename defines the configuration file name for the ELA node.
	configFilename = "./config.json"
)

var (
	// defaultConfig defines the default parameters to running a SPV client.
	defaultConfig = configParams{
		RPCPort:    20346,
		DebugLevel: defaultDebugLevel,
	}

	cfg = loadConfig()
)

type configParams struct {
	Network        string
	PermanentPeers []string
	RPCPort        uint16
	DebugLevel     string
}

func loadConfig() *configParams {
	data, err := ioutil.ReadFile(configFilename)
	if err != nil {
		fmt.Printf("Read config file error %s", err)
		os.Exit(-1)
	}
	// Remove the UTF-8 Byte Order Mark
	data = bytes.TrimPrefix(data, []byte("\xef\xbb\xbf"))

	// We have put the default configuration into config file, it's not mater
	// whether unmarshall success or not.
	json.Unmarshal(data, &defaultConfig)

	return &defaultConfig
}
