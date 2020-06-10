// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/elanet/dns"
	"github.com/elastos/Elastos.ELA/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

const (
	// help message print on startup.
	help = "Go to the 'elanet/dns/README.md' file for more details.\n" +
		"OPTIONS:\n" +
		"	-net    specify a active net for the DNS service.\n" +
		"	-magic  specify a magic number of the DNS service.\n" +
		"	-port   specify a port number for the DNS service.\n" +
		"	-newversionheight   specify a new version message height for the DNS service.\n" +
		"	-debug  enable debug mode."
)

var (
	// Build version generated when build program.
	Version string

	// The go source code version at build.
	GoVersion string

	// Set default active net params.
	params = &config.DefaultParams

	// dataDir defines the default data directory.
	dataDir = filepath.Join("elastos", "data", "dns")

	// logDir defines the directory to put log files.
	logDir = filepath.Join("elastos", "logs", "dns")

	// logLevel represents the log print level.
	logLevel = elalog.LevelInfo
)

func main() {
	// Print help message on startup.
	fmt.Println(help)

	interrupt := signal.NewInterrupt()

	// Resolve the parameters if have.
	var net string
	var magic uint
	var port uint
	var debug bool
	var newversionheight uint
	flag.StringVar(&net, "net", "main", "specify a active net for the DNS service")
	flag.UintVar(&magic, "magic", 0, "specify a magic number for the DNS service")
	flag.UintVar(&port, "port", 0, "specify a port number for the DNS service")
	flag.BoolVar(&debug, "debug", false, "turn on debug log")
	flag.UintVar(&newversionheight, "newversionheight", 0, "specify a new version message height for the DNS service")
	flag.Parse()

	// Use the specified active net parameters.
	switch strings.ToLower(net) {
	case "testnet", "test":
		params = config.DefaultParams.TestNet()
	case "regnet", "reg":
		params = config.DefaultParams.RegNet()
	}

	// If magic parameter specified use the given magic number.
	if magic != 0 {
		params.Magic = uint32(magic)
	}

	// If port parameter specified use the given port number.
	if port != 0 {
		params.DefaultPort = uint16(port)
	}

	// If port parameter specified use the given port number.
	if newversionheight != 0 {
		params.NewVersionHeight = uint64(newversionheight)
	}

	// Create the DNS instance.
	dnsService, err := dns.New(dataDir, params.Magic, params.DefaultPort, params.NewVersionHeight, Version)
	if err != nil {
		fmt.Fprint(os.Stderr, err.Error())
		os.Exit(1)
	}

	// Change log level to debug if debug parameter added.
	if debug {
		logLevel = elalog.LevelDebug
	}

	// Initiate log printer.
	fileWriter := elalog.NewFileWriter(logDir, 0, 0)
	backend := elalog.NewBackend(io.MultiWriter(fileWriter, os.Stdout),
		elalog.Llongfile)
	addrlog := backend.Logger("AMGR", logLevel)
	peerlog := backend.Logger("PEER", logLevel)
	srvrlog := backend.Logger("SRVR", logLevel)
	dnsslog := backend.Logger("DNSS", logLevel)
	addrmgr.UseLogger(addrlog)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	dns.UseLogger(dnsslog)

	dnsslog.Infof("Node version: %s", Version)
	dnsslog.Info(GoVersion)
	dnsService.Start()

	<-interrupt.C
	dnsService.Stop()
}
