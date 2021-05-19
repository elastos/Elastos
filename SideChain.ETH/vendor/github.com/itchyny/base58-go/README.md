# base58-go [![Travis Build Status](https://travis-ci.org/itchyny/base58-go.svg?branch=master)](https://travis-ci.org/itchyny/base58-go) [![Go Report Card](https://goreportcard.com/badge/github.com/itchyny/base58-go)](https://goreportcard.com/report/github.com/itchyny/base58-go) [![MIT License](http://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/itchyny/base58-go/blob/master/LICENSE) [![GoDoc](https://godoc.org/github.com/itchyny/base58-go?status.svg)](https://godoc.org/github.com/itchyny/base58-go)

### Base58 encoding/decoding package in Go
This is a Go language package for encoding/decoding base58 strings.
This package supports multiple encodings.

[tv42/base58](https://github.com/tv42/base58) is an existing implementation.
But I do not like what the package provides; the interface of the package,
the command-line interface, the pull request of configuring the encoding
is not merged for months, leading zeros and empty string are not encoded
correctly. So I created a new package and command line interface.

## Package Usage
```go
package main

import (
	"fmt"
	"os"

	"github.com/itchyny/base58-go"
)

func main() {
	encoding := base58.FlickrEncoding // or RippleEncoding or BitcoinEncoding

	encoded, err := encoding.Encode([]byte("100"))
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	fmt.Println(string(encoded))

	decoded, err := encoding.Decode(encoded)
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	fmt.Println(string(decoded))
}
```

## base58 command
### Homebrew
```sh
brew install itchyny/tap/base58
```

### Build from source
```bash
go get -u github.com/itchyny/base58-go/cmd/base58
```

### Basic usage
```sh
 $ base58
100
2J
100000000
9QwvW
79228162514264337593543950336
5QchsBFApWPVxyp9C
^D
 $ base58 --decode
2J
100
9QwvW
100000000
5QchsBFApWPVxyp9C
79228162514264337593543950336
^D
 $ echo 100000000 | base58
9QwvW
 $ echo 9QwvW | base58 --decode
100000000
 $ echo 100000000 | base58 --encoding=bitcoin
9qXWw
```

## Bug Tracker
Report bug at [Issues・itchyny/base58-go - GitHub](https://github.com/itchyny/base58-go/issues).

## Author
itchyny (https://github.com/itchyny)

## License
This software is released under the MIT License, see LICENSE.
