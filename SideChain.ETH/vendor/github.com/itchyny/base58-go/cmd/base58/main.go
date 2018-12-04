package main

import (
	"bufio"
	"fmt"
	"io"
	"os"

	"github.com/itchyny/base58-go"
	"github.com/jessevdk/go-flags"
)

type flagopts struct {
	Decode   bool     `short:"D" long:"decode" description:"decodes input"`
	Encoding string   `short:"e" long:"encoding" default:"flickr" description:"encoding (flickr, ripple or bitcoin)"`
	Input    []string `short:"i" long:"input" default:"-" description:"input file"`
	Output   string   `short:"o" long:"output" default:"-" description:"output file"`
}

func (opt *flagopts) encoding() *base58.Encoding {
	var encoding *base58.Encoding
	switch opt.Encoding {
	case "flickr":
		encoding = base58.FlickrEncoding
	case "ripple":
		encoding = base58.RippleEncoding
	case "bitcoin":
		encoding = base58.BitcoinEncoding
	default:
		fmt.Fprintf(os.Stderr, "Unknown encoding: %s.\n", opt.Encoding)
		os.Exit(1)
	}
	return encoding
}

type option struct {
	decode   bool
	encoding *base58.Encoding
}

func main() {
	var opts flagopts
	args, err := flags.Parse(&opts)
	if err != nil {
		os.Exit(1)
	}
	var inputFiles []string
	for _, name := range append(opts.Input, args...) {
		if name != "" && name != "-" {
			inputFiles = append(inputFiles, name)
		}
	}
	var outFile io.Writer
	if opts.Output == "-" {
		outFile = os.Stdout
	} else {
		file, err := os.Create(opts.Output)
		if err != nil {
			fmt.Fprintln(os.Stderr, err.Error())
			os.Exit(1)
		}
		defer file.Close()
		outFile = file
	}
	var status int
	opt := &option{decode: opts.Decode, encoding: opts.encoding()}
	if len(inputFiles) == 0 {
		status = run(opt, os.Stdin, outFile, os.Stderr)
	}
	for _, name := range inputFiles {
		file, err := os.Open(name)
		if err != nil {
			fmt.Fprintln(os.Stderr, err.Error())
			continue
		}
		defer file.Close()
		if s := run(opt, file, outFile, os.Stderr); status < s {
			status = s
		}
	}
	os.Exit(status)
}

func run(opt *option, in io.Reader, out io.Writer, outerr io.Writer) int {
	scanner := bufio.NewScanner(in)
	var status int
	var result []byte
	var err error
	for scanner.Scan() {
		src := scanner.Bytes()
		if opt.decode {
			result, err = opt.encoding.Decode(src)
		} else {
			result, err = opt.encoding.Encode(src)
		}
		if err != nil {
			fmt.Fprintln(outerr, err.Error())
			status = 1
			continue
		}
		out.Write(result)
		out.Write([]byte{0x0a})
	}
	return status
}
