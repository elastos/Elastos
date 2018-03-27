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

var config *Config // The single instance of config

type Config struct {
	PrintLevel uint8
	Magic      uint32
	SeedList   []string
}

func (config *Config) readConfigFile() error {
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

func Values() *Config {
	if config == nil {
		config = new(Config)
		err := config.readConfigFile()
		if err != nil {
			fmt.Println("Read config file error:", err)
		}
	}
	return config
}
