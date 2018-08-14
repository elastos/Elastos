package node

import (
	"time"
	"fmt"
	"strings"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/config"
)

const (
	DumpNodeStateInterval = time.Minute * 5
	NodeTitleFormat       = "| %-5s | %-16s | %-8s | %-16s | %-5s | %5s | %5s | %5s | %5s | %13s |\n"
	NodeLineFormat        = "| %-5d | %-16x | %-8d | %-16d | %-5t | %5d | %5d | %5d | %5d | %13s |\n"
	NeighborTitleFormat   = "| %-5s | %-16s | %-8s | %-16s | %-5s | %-5s | %-10s | %24s |\n"
	NeighborLineFormat    = "| %-5d | %-16x | %-8d | %-16d | %-5t | %-5t | %-10s | %24s |\n"
)

func monitorNodeState() {
	ticker := time.NewTicker(DumpNodeStateInterval)
	defer ticker.Stop()
	for range ticker.C {
		// Dump local node state information
		dumpNodeState()
	}
}

func dumpNodeState() {
	state := " ***** DUMP NODE STATE INFORMATION ***** \n"
	neighbors := LocalNode.GetNeighborNodes()

	// Dump node information
	state += separateLine()
	state += fmt.Sprintf(NodeTitleFormat,
		"NBRS",
		"NODE ID",
		"HEIGHT",
		"SERVICES",
		"RELAY",
		"P2P",
		"REST",
		"RPC",
		"WS",
		"OPEN SERVICE",
	)
	state += separateLine()

	var openService string
	if config.Parameters.OpenService {
		openService = fmt.Sprintf("%5t / %5d", true, config.Parameters.NodeOpenPort)
	} else {
		openService = fmt.Sprintf("%5t / %5s", false, "NONE")
	}
	state += fmt.Sprintf(NodeLineFormat,
		len(neighbors),
		LocalNode.ID(),
		chain.DefaultLedger.Blockchain.BlockHeight,
		LocalNode.services,
		LocalNode.IsRelay(),
		LocalNode.Port(),
		config.Parameters.HttpRestPort,
		config.Parameters.HttpJsonPort,
		config.Parameters.HttpWsPort,
		openService,
	)
	state += separateLine()

	// Dump neighbors information
	state += neighborTitleLine()
	state += separateLine()

	for index, neighbor := range neighbors {
		state += dumpNeighborLine(index, neighbor.(*node))
		state += separateLine()
	}

	log.Info(state)
}

func separateLine() string {
	return fmt.Sprintf(" %s\n", strings.Repeat("-", 112))
}

func neighborTitleLine() string {
	return fmt.Sprintf(NeighborTitleFormat,
		"INDEX",
		"NEIGHBOR ID",
		"HEIGHT",
		"SERVICES",
		"RELAY",
		"EXTRA",
		"STATUS",
		"IP ADDRESS",
	)
}

func dumpNeighborLine(index int, n *node) string {
	return fmt.Sprintf(NeighborLineFormat,
		index+1,
		n.ID(),
		n.Height(),
		n.Services(),
		n.IsRelay(),
		n.IsFromExtraNet(),
		n.PeerState.String(),
		n.NetAddress().String(),
	)
}
