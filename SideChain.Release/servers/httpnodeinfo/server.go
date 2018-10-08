package httpnodeinfo

import (
	"fmt"
	"html/template"
	"net/http"
	"strconv"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"

	"github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

type Config struct {
	HttpRestPort uint16
	HttpJsonPort uint16
	NodePort     uint16
	Chain        *blockchain.BlockChain
	Server       server.IServer
}

type Info struct {
	NodeVersion  string
	BlockHeight  uint32
	NeighborCnt  int
	Neighbors    []NgbNodeInfo
	HttpRestPort uint16
	HttpJsonPort uint16
	NodePort     uint16
	NodeId       string
}

type NgbNodeInfo struct {
	NgbId   string
	NbrAddr string
}

var templates = template.Must(template.New("info").Parse(page))

type infoserver struct {
	nodePort  uint16
	jsonPort  uint16
	resetPort uint16
	chain     *blockchain.BlockChain
	server    server.IServer
}

func (s *infoserver) view(w http.ResponseWriter, r *http.Request) {
	var nodeInfos []NgbNodeInfo
	peers := s.server.ConnectedPeers()

	for _, node := range peers {
		nodeInfos = append(nodeInfos, NgbNodeInfo{
			NgbId:   fmt.Sprintf("0x%x", node.ToPeer().ID()),
			NbrAddr: node.ToPeer().String(),
		})
	}

	pageInfo := &Info{
		BlockHeight:  s.chain.GetBestHeight(),
		NeighborCnt:  len(peers),
		Neighbors:    nodeInfos,
		HttpRestPort: s.resetPort,
		HttpJsonPort: s.jsonPort,
		NodePort:     s.nodePort,
		NodeId:       fmt.Sprintf("0x%x", 0),
	}

	err := templates.ExecuteTemplate(w, "info", pageInfo)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func (s *infoserver) Start() {
	http.HandleFunc("/info", s.view)
	http.ListenAndServe(":"+strconv.Itoa(int(config.Parameters.HttpInfoPort)), nil)
}

func New(cfg *Config) *infoserver {
	return &infoserver{
		nodePort:  cfg.NodePort,
		jsonPort:  cfg.HttpJsonPort,
		resetPort: cfg.HttpRestPort,
		chain:     cfg.Chain,
		server:    cfg.Server,
	}
}
