package httprestful

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"strings"

	"github.com/elastos/Elastos.ELA.SideChain/servers"
)

const (
	ApiGetConnectionCount  = "/api/v1/node/connectioncount"
	ApiGetBlockTxsByHeight = "/api/v1/block/transactions/height"
	ApiGetBlockByHeight    = "/api/v1/block/details/height"
	ApiGetBlockByHash      = "/api/v1/block/details/hash"
	ApiGetBlockHeight      = "/api/v1/block/height"
	ApiGetBlockHash        = "/api/v1/block/hash"
	ApiGetTotalIssued      = "/api/v1/totalissued"
	ApiGetTransaction      = "/api/v1/transaction"
	ApiGetAsset            = "/api/v1/asset"
	ApiGetBalanceByAddr    = "/api/v1/asset/balances"
	ApiGetBalanceByAsset   = "/api/v1/asset/balance"
	ApiGetUTXOByAsset      = "/api/v1/asset/utxo"
	ApiGetUTXOByAddr       = "/api/v1/asset/utxos"
	ApiSendRawTransaction  = "/api/v1/transaction"
	ApiGetTransactionPool  = "/api/v1/transactionpool"
	ApiRestart             = "/api/v1/restart"
)

type restserver struct {
	port     uint16
	certFile string
	keyFile  string
	router   router
	server   *http.Server
}

func New(port uint16, service *servers.HttpService, certFile, keyFile string) *restserver {
	s := restserver{
		port:     port,
		certFile: certFile,
		keyFile:  keyFile,
	}

	s.RegisterAction("GET", ApiGetConnectionCount, service.GetConnectionCount)
	s.RegisterAction("GET", ApiGetBlockTxsByHeight, service.GetTransactionsByHeight, "height")
	s.RegisterAction("GET", ApiGetBlockByHeight, service.GetBlockByHeight, "height")
	s.RegisterAction("GET", ApiGetBlockByHash, service.GetBlockByHash, "hash")
	s.RegisterAction("GET", ApiGetBlockHeight, service.GetBlockHeight)
	s.RegisterAction("GET", ApiGetBlockHash, service.GetBlockHash, "height")
	s.RegisterAction("GET", ApiGetTransactionPool, service.GetTransactionPool)
	s.RegisterAction("GET", ApiGetTransaction, service.GetTransactionByHash, "hash")
	s.RegisterAction("GET", ApiGetAsset, service.GetAssetByHash, "hash")
	s.RegisterAction("GET", ApiGetUTXOByAddr, service.GetUnspendsByAddr, "addr")
	s.RegisterAction("GET", ApiGetUTXOByAsset, service.GetUnspendsByAsset, "addr", "asset")
	s.RegisterAction("GET", ApiGetBalanceByAddr, service.GetBalanceByAddr, "addr")
	s.RegisterAction("GET", ApiGetBalanceByAsset, service.GetBalanceByAsset, "addr", "asset")
	s.RegisterAction("GET", ApiRestart, s.Restart)

	s.RegisterAction("POST", ApiSendRawTransaction, service.SendRawTransaction)

	return &s
}

func (s *restserver) write(w http.ResponseWriter, data []byte) {
	w.Header().Add("Access-Control-Allow-Headers", "Content-Type")
	w.Header().Set("content-type", "application/json;charset=utf-8")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Write(data)
}

func (s *restserver) response(w http.ResponseWriter, resp map[string]interface{}) {
	resp["Desc"] = resp["Error"].(servers.ErrorCode).String()
	data, err := json.Marshal(resp)
	if err != nil {
		log.Fatal("HTTP Handle - json.Marshal: %v", err)
		return
	}
	s.write(w, data)
}

func (s *restserver) RegisterAction(method, path string, handler servers.Handler, params ...string) {
	// build full path first
	path = strings.TrimRight(path, "/")
	for _, param := range params {
		path += "/:" + param
	}

	// register action to router
	switch strings.ToUpper(method) {
	case "GET":
		s.router.Get(path, func(w http.ResponseWriter, r *http.Request) {

			var req = make(map[string]interface{})
			for _, param := range params {
				req[param] = getParam(r, param)
			}

			s.response(w, handler(req))
		})

	case "POST":
		s.router.Post(path, func(w http.ResponseWriter, r *http.Request) {

			body, _ := ioutil.ReadAll(r.Body)
			defer r.Body.Close()

			var req = make(map[string]interface{})
			var resp map[string]interface{}
			for _, param := range params {
				req[param] = getParam(r, param)
			}

			if err := json.Unmarshal(body, &req); err == nil {
				resp = handler(req)
			} else {
				resp = servers.ResponsePack(servers.IllegalDataFormat, "")
			}

			s.response(w, resp)
		})

		s.router.Options(path, func(w http.ResponseWriter, r *http.Request) {
			s.write(w, []byte{})
		})
	}
}

func (s *restserver) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	handler, params, err := s.router.serve(req.URL.Path, req.Method)
	if err != nil {
		http.NotFound(w, req)
		return
	}
	ctx := context.WithValue(req.Context(), RouteParams, params)
	handler(w, req.WithContext(ctx))
}

func (s *restserver) Start() {
	if s.port == 0 {
		log.Fatal("Not configure HttpRestPort port ")
	}

	var err error
	var listener net.Listener

	if s.port%1000 == servers.TlsPort {
		var err error
		listener, err = s.initTlsListen()
		if err != nil {
			log.Error("Https Cert: ", err.Error())
		}
	} else {
		var err error
		listener, err = net.Listen("tcp", fmt.Sprint(":", s.port))
		if err != nil {
			log.Fatal("net.Listen: ", err.Error())
		}
	}
	s.server = &http.Server{Handler: s}
	err = s.server.Serve(listener)
	if err != nil {
		log.Fatal("ListenAndServe: ", err.Error())
	}
}

func (s *restserver) Stop() {
	if s.server != nil {
		s.server.Shutdown(context.Background())
		log.Error("Close restful ")
	}
}

func (s *restserver) Restart(cmd servers.Params) map[string]interface{} {
	go func() {
		s.Stop()
		s.Start()
	}()

	return servers.ResponsePack(servers.Success, "")
}

func (s *restserver) initTlsListen() (net.Listener, error) {
	// load cert
	cert, err := tls.LoadX509KeyPair(s.certFile, s.keyFile)
	if err != nil {
		log.Error("load keys fail", err)
		return nil, err
	}

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	log.Infof("TLS listen port is %d", s.port)
	listener, err := tls.Listen("tcp", fmt.Sprint(":", s.port), tlsConfig)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return listener, nil
}
