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

	. "github.com/elastos/Elastos.ELA.SideChain/config"
	. "github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/servers"
)

type restServer struct {
	router   *Router
	listener net.Listener
	server   *http.Server
	postMap  map[string]servers.Action
	getMap   map[string]servers.Action
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
	getMethodMap := servers.HttpServers.RestFulGetFunctions
	getMethodMap[servers.Api_Restart] = servers.Action{Name: "restart", Handler: rt.Restart}
	postMethodMap := servers.HttpServers.RestFulPostFunctions

	rt.postMap = postMethodMap
	rt.getMap = getMethodMap
}

func (rt *restServer) getPath(url string) string {

	if strings.Contains(url, strings.TrimRight(servers.Api_GetblockTxsByHeight, ":height")) {
		return servers.Api_GetblockTxsByHeight
	} else if strings.Contains(url, strings.TrimRight(servers.Api_Getblockbyheight, ":height")) {
		return servers.Api_Getblockbyheight
	} else if strings.Contains(url, strings.TrimRight(servers.Api_Getblockbyhash, ":hash")) {
		return servers.Api_Getblockbyhash
	} else if strings.Contains(url, strings.TrimRight(servers.Api_Getblockhash, ":height")) {
		return servers.Api_Getblockhash
	} else if strings.Contains(url, strings.TrimRight(servers.Api_GetTotalIssued, ":assetid")) {
		return servers.Api_GetTotalIssued
	} else if strings.Contains(url, strings.TrimRight(servers.Api_Gettransaction, ":hash")) {
		return servers.Api_Gettransaction
	} else if strings.Contains(url, strings.TrimRight(servers.Api_GetBalanceByAddr, ":addr")) {
		return servers.Api_GetBalanceByAddr
	} else if strings.Contains(url, strings.TrimRight(servers.Api_GetBalancebyAsset, ":addr/:assetid")) {
		return servers.Api_GetBalancebyAsset
	} else if strings.Contains(url, strings.TrimRight(servers.Api_GetUTXObyAddr, ":addr")) {
		return servers.Api_GetUTXObyAddr
	} else if strings.Contains(url, strings.TrimRight(servers.Api_GetUTXObyAsset, ":addr/:assetid")) {
		return servers.Api_GetUTXObyAsset
	} else if strings.Contains(url, strings.TrimRight(servers.Api_Getasset, ":hash")) {
		return servers.Api_Getasset
	}
	return url
}

func (rt *restServer) getParams(r *http.Request, url string, req map[string]interface{}) map[string]interface{} {
	switch url {
	case servers.Api_Getconnectioncount:

	case servers.Api_GetblockTxsByHeight:
		req["height"] = getParam(r, "height")

	case servers.Api_Getblockbyheight:
		req["height"] = getParam(r, "height")

	case servers.Api_Getblockbyhash:
		req["hash"] = getParam(r, "hash")

	case servers.Api_Getblockheight:

	case servers.Api_GetTransactionPool:

	case servers.Api_Getblockhash:
		req["height"] = getParam(r, "height")

	case servers.Api_GetTotalIssued:
		req["assetid"] = getParam(r, "assetid")

	case servers.Api_Gettransaction:
		req["hash"] = getParam(r, "hash")

	case servers.Api_Getasset:
		req["hash"] = getParam(r, "hash")

	case servers.Api_GetBalancebyAsset:
		req["addr"] = getParam(r, "addr")
		req["assetid"] = getParam(r, "assetid")

	case servers.Api_GetBalanceByAddr:
		req["addr"] = getParam(r, "addr")

	case servers.Api_GetUTXObyAddr:
		req["addr"] = getParam(r, "addr")

	case servers.Api_GetUTXObyAsset:
		req["addr"] = getParam(r, "addr")
		req["assetid"] = getParam(r, "assetid")

	case servers.Api_Restart:

	case servers.Api_SendRawTransaction:

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
				resp = h.Handler(req)
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
					resp = h.Handler(req)
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
