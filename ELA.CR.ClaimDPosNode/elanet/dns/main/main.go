package main

import (
	"flag"
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/elanet/dns"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/elastos/Elastos.ELA/utils/elalog"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

const (
	// dataDir defines the default data directory.
	dataDir = "elastos/dns/"

	// help message print on startup.
	help = "Go to the 'elanet/dns/README.md' file for more details.\n" +
		"OPTIONS:\n" +
		"	-p      specify a port for the DNS service\n" +
		"	-net    specify a network for the DNS service\n" +
		"	-debug  enable debug mode\n"
)

var (
	// Build version generated when build program.
	Version string

	// The go source code version at build.
	GoVersion string

	// Set default active net params.
	params = &config.DefaultParams
)

func main() {
	// Print help message on startup.
	fmt.Println(help)

	interrupt := signal.NewInterrupt()

	// Resolve the parameters if have.
	var port uint
	var net string
	var debug bool
	flag.UintVar(&port, "p", 0, "port")
	flag.StringVar(&net, "net", "", "active net")
	flag.BoolVar(&debug, "debug", false, "turn on debug log")
	flag.Parse()

	switch strings.ToLower(net) {
	case "testnet", "test":
		params = config.DefaultParams.TestNet()
	case "regnet", "reg":
		params = config.DefaultParams.RegNet()
	}

	// If no port parameter, use default value.
	if port == 0 {
		port = uint(params.DefaultPort)
	}

	// Create the DNS instance.
	dnsService, err := dns.New(dataDir, params.Magic, uint16(port))
	if err != nil {
		fmt.Fprint(os.Stderr, err.Error())
		os.Exit(1)
	}

	// Initiate log.
	logLevel := elalog.LevelInfo
	if debug {
		logLevel = elalog.LevelDebug
	}
	fileWriter := elalog.NewFileWriter(dataDir, 0, 0)
	backend := elalog.NewBackend(io.MultiWriter(fileWriter, os.Stdout),
		elalog.Llongfile)
	peerlog := backend.Logger("PEER", logLevel)
	srvrlog := backend.Logger("SRVR", logLevel)
	dnsslog := backend.Logger("DNSS", logLevel)
	peer.UseLogger(peerlog)
	server.UseLogger(srvrlog)
	dns.UseLogger(dnsslog)

	dnsslog.Infof("Node version: %s", Version)
	dnsslog.Info(GoVersion)
	dnsService.Start()

	<-interrupt.C
	dnsService.Stop()
}
