// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package httpnodeinfo

import (
	"fmt"
	"html/template"
	"net/http"
	"strconv"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/servers"
)

type Info struct {
	NodeVersion   string
	BlockHeight   uint32
	NeighborCnt   int
	Neighbors     []NgbNodeInfo
	HttpRestPort  int
	HttpWsPort    int
	HttpJsonPort  int
	HttpLocalPort int
	NodePort      uint16
}

type NgbNodeInfo struct {
	NgbID   string
	NbrAddr string
}

var templates = template.Must(template.New("info").Parse(page))

func viewHandler(w http.ResponseWriter, r *http.Request) {
	var ngbrNodersInfo []NgbNodeInfo

	peers := servers.Server.ConnectedPeers()

	for _, ip := range peers {
		p := ip.ToPeer()
		ngbrNodersInfo = append(ngbrNodersInfo, NgbNodeInfo{
			NgbID:   fmt.Sprintf("0x%x", p.ID()),
			NbrAddr: p.Addr(),
		})
	}

	pageInfo := &Info{
		BlockHeight:  chain.DefaultLedger.Blockchain.GetHeight(),
		NeighborCnt:  len(peers),
		Neighbors:    ngbrNodersInfo,
		HttpRestPort: config.Parameters.HttpRestPort,
		HttpWsPort:   config.Parameters.HttpWsPort,
		HttpJsonPort: config.Parameters.HttpJsonPort,
		NodePort:     config.Parameters.NodePort,
	}

	err := templates.ExecuteTemplate(w, "info", pageInfo)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func StartServer() {
	http.HandleFunc("/info", viewHandler)
	http.ListenAndServe(":"+strconv.Itoa(int(config.Parameters.HttpInfoPort)), nil)
}
