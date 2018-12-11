package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
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
	DefaultPort   uint16 // node port for public peers to provide services.
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

	c.SeedList = normalizeAddresses(c.SeedList, fmt.Sprint(c.DefaultPort))

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

// removeDuplicateAddresses returns a new slice with all duplicate entries in
// addrs removed.
func removeDuplicateAddresses(addrs []string) []string {
	result := make([]string, 0, len(addrs))
	seen := map[string]struct{}{}
	for _, val := range addrs {
		if _, ok := seen[val]; !ok {
			result = append(result, val)
			seen[val] = struct{}{}
		}
	}
	return result
}

// normalizeAddress returns addr with the passed default port appended if
// there is not already a port specified.
func normalizeAddress(addr, defaultPort string) string {
	_, _, err := net.SplitHostPort(addr)
	if err != nil {
		return net.JoinHostPort(addr, defaultPort)
	}
	return addr
}

// normalizeAddresses returns a new slice with all the passed peer addresses
// normalized with the given default port, and all duplicates removed.
func normalizeAddresses(addrs []string, defaultPort string) []string {
	for i, addr := range addrs {
		addrs[i] = normalizeAddress(addr, defaultPort)
	}

	return removeDuplicateAddresses(addrs)
}
