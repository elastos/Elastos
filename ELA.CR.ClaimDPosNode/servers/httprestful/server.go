package httprestful

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"io/ioutil"
	"net"
	"net/http"
	"strconv"
	"strings"
	"sync"

	. "github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/servers"
)

const (
	Api_Getconnectioncount  = "/api/v1/node/connectioncount"
	Api_GetNodeState        = "/api/v1/node/state"
	Api_GetblockTxsByHeight = "/api/v1/block/transactions/height/:height"
	Api_Getblockbyheight    = "/api/v1/block/details/height/:height"
	Api_Getblockbyhash      = "/api/v1/block/details/hash/:hash"
	Api_Getblockheight      = "/api/v1/block/height"
	Api_Getblockhash        = "/api/v1/block/hash/:height"
	Api_Gettransaction      = "/api/v1/transaction/:hash"
	Api_Getasset            = "/api/v1/asset/:hash"
	Api_GetBalanceByAddr    = "/api/v1/asset/balances/:addr"
	Api_GetBalancebyAsset   = "/api/v1/asset/balance/:addr/:assetid"
	Api_GetUTXObyAsset      = "/api/v1/asset/utxo/:addr/:assetid"
	Api_GetUTXObyAddr       = "/api/v1/asset/utxos/:addr"
	Api_SendRawTransaction  = "/api/v1/transaction"
	Api_GetTransactionPool  = "/api/v1/transactionpool"
	Api_Restart             = "/api/v1/restart"
)

type Action struct {
	sync.RWMutex
	name    string
	handler func(servers.Params) map[string]interface{}
}

type restServer struct {
	router   *Router
	listener net.Listener
	server   *http.Server
	postMap  map[string]Action
	getMap   map[string]Action
}

type ApiServer interface {
	Start()
	Stop()
}

func StartServer() {
	rest := InitRestServer()
	rest.Start()
}

func InitRestServer() ApiServer {
	rt := &restServer{}
	rt.router = &Router{}
	rt.initializeMethod()
	rt.initGetHandler()
	rt.initPostHandler()
	return rt
}

func (rt *restServer) Start() {
	if Parameters.HttpRestPort == 0 {
		log.Fatal("Not configure HttpRestPort port ")
	}

	if Parameters.HttpRestPort%1000 == servers.TlsPort {
		var err error
		rt.listener, err = rt.initTlsListen()
		if err != nil {
			log.Error("Https Cert: ", err.Error())
		}
	} else {
		var err error
		rt.listener, err = net.Listen("tcp", ":"+strconv.Itoa(Parameters.HttpRestPort))
		if err != nil {
			log.Fatal("net.Listen: ", err.Error())
		}
	}
	rt.server = &http.Server{Handler: rt.router}
	err := rt.server.Serve(rt.listener)

	if err != nil {
		log.Fatal("ListenAndServe: ", err.Error())
	}
}

func (rt *restServer) initializeMethod() {

	getMethodMap := map[string]Action{
		Api_Getconnectioncount:  {name: "getconnectioncount", handler: servers.GetConnectionCount},
		Api_GetNodeState:        {name: "getnodestate", handler: servers.GetNodeState},
		Api_GetblockTxsByHeight: {name: "getblocktransactionsbyheight", handler: servers.GetTransactionsByHeight},
		Api_Getblockbyheight:    {name: "getblockbyheight", handler: servers.GetBlockByHeight},
		Api_Getblockbyhash:      {name: "getblockbyhash", handler: servers.GetBlockByHash},
		Api_Getblockheight:      {name: "getblockheight", handler: servers.GetBlockHeight},
		Api_Getblockhash:        {name: "getblockhash", handler: servers.GetBlockHash},
		Api_GetTransactionPool:  {name: "gettransactionpool", handler: servers.GetTransactionPool},
		Api_Gettransaction:      {name: "gettransaction", handler: servers.GetTransactionByHash},
		Api_Getasset:            {name: "getasset", handler: servers.GetAssetByHash},
		Api_GetUTXObyAddr:       {name: "getutxobyaddr", handler: servers.GetUnspends},
		Api_GetUTXObyAsset:      {name: "getutxobyasset", handler: servers.GetUnspendOutput},
		Api_GetBalanceByAddr:    {name: "getbalancebyaddr", handler: servers.GetBalanceByAddr},
		Api_GetBalancebyAsset:   {name: "getbalancebyasset", handler: servers.GetBalanceByAsset},
		Api_Restart:             {name: "restart", handler: rt.Restart},
	}

	postMethodMap := map[string]Action{
		Api_SendRawTransaction: {name: "sendrawtransaction", handler: servers.SendRawTransaction},
	}
	rt.postMap = postMethodMap
	rt.getMap = getMethodMap
}

func (rt *restServer) getPath(url string) string {

	if strings.Contains(url, strings.TrimRight(Api_GetblockTxsByHeight, ":height")) {
		return Api_GetblockTxsByHeight
	} else if strings.Contains(url, strings.TrimRight(Api_Getblockbyheight, ":height")) {
		return Api_Getblockbyheight
	} else if strings.Contains(url, strings.TrimRight(Api_Getblockbyhash, ":hash")) {
		return Api_Getblockbyhash
	} else if strings.Contains(url, strings.TrimRight(Api_Getblockhash, ":height")) {
		return Api_Getblockhash
	} else if strings.Contains(url, strings.TrimRight(Api_Gettransaction, ":hash")) {
		return Api_Gettransaction
	} else if strings.Contains(url, strings.TrimRight(Api_GetBalanceByAddr, ":addr")) {
		return Api_GetBalanceByAddr
	} else if strings.Contains(url, strings.TrimRight(Api_GetBalancebyAsset, ":addr/:assetid")) {
		return Api_GetBalancebyAsset
	} else if strings.Contains(url, strings.TrimRight(Api_GetUTXObyAddr, ":addr")) {
		return Api_GetUTXObyAddr
	} else if strings.Contains(url, strings.TrimRight(Api_GetUTXObyAsset, ":addr/:assetid")) {
		return Api_GetUTXObyAsset
	} else if strings.Contains(url, strings.TrimRight(Api_Getasset, ":hash")) {
		return Api_Getasset
	}
	return url
}

func (rt *restServer) getParams(r *http.Request, url string, req map[string]interface{}) map[string]interface{} {
	switch url {
	case Api_Getconnectioncount:

	case Api_GetNodeState:

	case Api_GetblockTxsByHeight:
		req["height"] = getParam(r, "height")

	case Api_Getblockbyheight:
		req["height"] = getParam(r, "height")

	case Api_Getblockbyhash:
		req["blockhash"] = getParam(r, "hash")

	case Api_Getblockheight:

	case Api_GetTransactionPool:

	case Api_Getblockhash:
		req["height"] = getParam(r, "height")

	case Api_Gettransaction:
		req["hash"] = getParam(r, "hash")

	case Api_Getasset:
		req["hash"] = getParam(r, "hash")

	case Api_GetBalancebyAsset:
		req["addr"] = getParam(r, "addr")
		req["assetid"] = getParam(r, "assetid")

	case Api_GetBalanceByAddr:
		req["addr"] = getParam(r, "addr")

	case Api_GetUTXObyAddr:
		req["addr"] = getParam(r, "addr")

	case Api_GetUTXObyAsset:
		req["addr"] = getParam(r, "addr")
		req["assetid"] = getParam(r, "assetid")

	case Api_Restart:

	case Api_SendRawTransaction:

	}
	return req
}

func (rt *restServer) initGetHandler() {

	for k, _ := range rt.getMap {
		rt.router.Get(k, func(w http.ResponseWriter, r *http.Request) {

			var req = make(map[string]interface{})
			var resp map[string]interface{}

			url := rt.getPath(r.URL.Path)

			if h, ok := rt.getMap[url]; ok {
				req = rt.getParams(r, url, req)
				resp = h.handler(req)
			} else {
				resp = servers.ResponsePack(InvalidMethod, "")
			}
			rt.response(w, resp)
		})
	}
}

func (rt *restServer) initPostHandler() {
	for k, _ := range rt.postMap {
		rt.router.Post(k, func(w http.ResponseWriter, r *http.Request) {

			body, _ := ioutil.ReadAll(r.Body)
			defer r.Body.Close()

			var req = make(map[string]interface{})
			var resp map[string]interface{}

			url := rt.getPath(r.URL.Path)
			if h, ok := rt.postMap[url]; ok {
				if err := json.Unmarshal(body, &req); err == nil {
					req = rt.getParams(r, url, req)
					resp = h.handler(req)
				} else {
					resp = servers.ResponsePack(IllegalDataFormat, "")
				}
			} else {
				resp = servers.ResponsePack(InvalidMethod, "")
			}
			rt.response(w, resp)
		})
	}
	//Options
	for k, _ := range rt.postMap {
		rt.router.Options(k, func(w http.ResponseWriter, r *http.Request) {
			rt.write(w, []byte{})
		})
	}

}

func (rt *restServer) write(w http.ResponseWriter, data []byte) {
	w.Header().Add("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("content-type", "application/json;charset=utf-8")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Write(data)
}

func (rt *restServer) response(w http.ResponseWriter, resp map[string]interface{}) {
	resp["Desc"] = ErrMap[resp["Error"].(ErrCode)]
	data, err := json.Marshal(resp)
	if err != nil {
		log.Fatal("HTTP Handle - json.Marshal: %v", err)
		return
	}
	rt.write(w, data)
}

func (rt *restServer) Stop() {
	if rt.server != nil {
		rt.server.Shutdown(context.Background())
		log.Error("Close restful ")
	}
}

func (rt *restServer) Restart(cmd servers.Params) map[string]interface{} {
	go func() {
		rt.Stop()
		rt.Start()
	}()

	return servers.ResponsePack(Success, "")
}

func (rt *restServer) initTlsListen() (net.Listener, error) {

	CertPath := Parameters.RestCertPath
	KeyPath := Parameters.RestKeyPath

	// load cert
	cert, err := tls.LoadX509KeyPair(CertPath, KeyPath)
	if err != nil {
		log.Error("load keys fail", err)
		return nil, err
	}

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	log.Info("TLS listen port is ", strconv.Itoa(Parameters.HttpRestPort))
	listener, err := tls.Listen("tcp", ":"+strconv.Itoa(Parameters.HttpRestPort), tlsConfig)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return listener, nil
}
