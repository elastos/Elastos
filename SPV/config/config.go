package config

import (
	"bytes"
	"io/ioutil"
	"encoding/json"
	"fmt"
)

const (
	ConfigFilename = "./config.json"
)

var config *Configuration // The single instance of config

type Configuration struct {
	Magic    uint32
	Port     uint16
	SeedList []string
}

func (config *Configuration) readConfigFile() error {
	data, err := ioutil.ReadFile(ConfigFilename)
	if err != nil {
		return err
	}
	// Remove the UTF-8 Byte Order Mark
	data = bytes.TrimPrefix(data, []byte("\xef\xbb\xbf"))

	err = json.Unmarshal(data, config)
	if err != nil {
		return err
	}
	return nil
}

func Config() *Configuration {
	if config == nil {
		config = new(Configuration)
		err := config.readConfigFile()
		if err != nil {
			fmt.Println("Read config file error:", err)
		}
	}
	return config
}
