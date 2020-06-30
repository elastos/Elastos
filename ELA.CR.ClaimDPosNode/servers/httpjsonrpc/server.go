// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package httpjsonrpc

import (
	"crypto/sha256"
	"crypto/subtle"
	"encoding/base64"
	"encoding/json"
	"io/ioutil"
	"mime"
	"net"
	"net/http"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/servers"
	elaErr "github.com/elastos/Elastos.ELA/servers/errors"
)

//an instance of the multiplexer
var mainMux map[string]func(Params) map[string]interface{}

const (
	// JSON-RPC protocol error codes.
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
	//-32000 to -32099	Server error, waiting for defining

	// IOTimeout is the maximum duration for JSON-RPC reading or writing
	// timeout.
	IOTimeout = 60 * time.Second

	// MaxRPCRead is the maximum buffer size for reading request.
	MaxRPCRead = 1024 * 1024 * 8
)

func StartRPCServer() {
	mainMux = make(map[string]func(Params) map[string]interface{})

	mainMux["setloglevel"] = SetLogLevel
	mainMux["getinfo"] = GetInfo
	mainMux["getblock"] = GetBlockByHash
	mainMux["getconfirmbyheight"] = GetConfirmByHeight
	mainMux["getconfirmbyhash"] = GetConfirmByHash
	mainMux["getcurrentheight"] = GetBlockHeight
	mainMux["getblockhash"] = GetBlockHash
	mainMux["getconnectioncount"] = GetConnectionCount
	mainMux["getrawmempool"] = GetTransactionPool
	mainMux["getrawtransaction"] = GetRawTransaction
	mainMux["getneighbors"] = GetNeighbors
	mainMux["getnodestate"] = GetNodeState
	mainMux["sendrawtransaction"] = SendRawTransaction
	mainMux["getarbitratorgroupbyheight"] = GetArbitratorGroupByHeight
	mainMux["getbestblockhash"] = GetBestBlockHash
	mainMux["getblockcount"] = GetBlockCount
	mainMux["getblockbyheight"] = GetBlockByHeight
	mainMux["getexistwithdrawtransactions"] = GetExistWithdrawTransactions
	mainMux["getreceivedbyaddress"] = GetReceivedByAddress
	// wallet interfaces
	mainMux["getamountbyinputs"] = GetAmountByInputs
	mainMux["getutxosbyamount"] = GetUTXOsByAmount
	mainMux["listunspent"] = ListUnspent
	mainMux["createrawtransaction"] = CreateRawTransaction
	mainMux["decoderawtransaction"] = DecodeRawTransaction
	mainMux["signrawtransactionwithkey"] = SignRawTransactionWithKey
	// aux interfaces
	mainMux["help"] = AuxHelp
	mainMux["submitauxblock"] = SubmitAuxBlock
	mainMux["createauxblock"] = CreateAuxBlock
	// mining interfaces
	mainMux["getmininginfo"] = GetMiningInfo
	mainMux["togglemining"] = ToggleMining
	mainMux["discretemining"] = DiscreteMining
	// cr interfaces
	mainMux["listcrcandidates"] = ListCRCandidates
	mainMux["listcurrentcrs"] = ListCurrentCRs
	mainMux["listcrproposalbasestate"] = ListCRProposalBaseState
	mainMux["getcrproposalstate"] = GetCRProposalState
	mainMux["getsecretarygeneral"] = GetSecretaryGeneral
	mainMux["getcrrelatedstage"] = GetCRRelatedStage
	// vote interfaces
	mainMux["listproducers"] = ListProducers
	mainMux["producerstatus"] = ProducerStatus
	mainMux["votestatus"] = VoteStatus
	// for cross-chain arbiter
	mainMux["submitsidechainillegaldata"] = SubmitSidechainIllegalData
	mainMux["getarbiterpeersinfo"] = GetArbiterPeersInfo
	mainMux["getdpospeersinfo"] = GetDPOSPeersInfo

	mainMux["estimatesmartfee"] = EstimateSmartFee
	mainMux["getdepositcoin"] = GetDepositCoin
	mainMux["getcrdepositcoin"] = GetCRDepositCoin
	mainMux["getarbitersinfo"] = GetArbitersInfo

	rpcServeMux := http.NewServeMux()
	server := http.Server{
		Handler:      rpcServeMux,
		ReadTimeout:  IOTimeout,
		WriteTimeout: IOTimeout,
	}
	rpcServeMux.HandleFunc("/", Handle)
	l, err := net.Listen("tcp4", ":"+strconv.Itoa(config.Parameters.HttpJsonPort))
	if err != nil {
		log.Fatal("Create listener error: ", err.Error())
		return
	}
	err = server.Serve(l)
	if err != nil {
		log.Fatal("ListenAndServe error: ", err.Error())
	}
}

//this is the function that should be called in order to answer an rpc call
//should be registered like "http.AddMethod("/", httpjsonrpc.Handle)"
func Handle(w http.ResponseWriter, r *http.Request) {
	isClientAllowed := clientAllowed(r)
	if !isClientAllowed {
		log.Warn("Client ip is not allowed")
		RPCError(w, http.StatusForbidden, InternalError, "Client ip is not allowed")
		return
	}

	// JSON RPC commands should be POSTs
	if r.Method != "POST" {
		log.Warn("JSON-RPC Handle - Method!=\"POST\"")
		RPCError(w, http.StatusMethodNotAllowed, InternalError, "JSON-RPC protocol only allows POST method")
		return
	}
	contentType, _, _ := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if contentType != "application/json" {
		RPCError(w, http.StatusUnsupportedMediaType, InternalError, "JSON-RPC need content type to be application/json")
		return
	}

	isCheckAuthOk := checkAuth(r)
	if !isCheckAuthOk {
		log.Warn("Client authenticate failed")
		RPCError(w, http.StatusUnauthorized, InternalError, "Client authenticate failed")
		return
	}

	//read the body of the request
	body, err := ioutil.ReadAll(http.MaxBytesReader(w, r.Body, MaxRPCRead))
	if err != nil {
		RPCError(w, http.StatusBadRequest, InvalidRequest, "JSON-RPC request reading error:"+err.Error())
		return
	}

	request := make(map[string]interface{})
	err = json.Unmarshal(body, &request)
	if err != nil {
		log.Error("JSON-RPC request parsing error: ", err)
		RPCError(w, http.StatusBadRequest, ParseError, "JSON-RPC request parsing error:"+err.Error())
		return
	}
	//get the corresponding function
	requestMethod, ok := request["method"].(string)
	if !ok {
		RPCError(w, http.StatusBadRequest, InvalidRequest, "JSON-RPC need a method")
		return
	}
	method, ok := mainMux[requestMethod]
	if !ok {
		RPCError(w, http.StatusNotFound, MethodNotFound, "JSON-RPC method "+requestMethod+" not found")
		return
	}

	requestParams := request["params"]
	// Json rpc 1.0 support positional parameters while json rpc 2.0 support named parameters.
	// positional parameters: { "requestParams":[1, 2, 3....] }
	// named parameters: { "requestParams":{ "a":1, "b":2, "c":3 } }
	// Here we support both of them.
	var params Params
	switch requestParams := requestParams.(type) {
	case nil:
	case []interface{}:
		params = convertParams(requestMethod, requestParams)
	case map[string]interface{}:
		params = Params(requestParams)
	default:
		RPCError(w, http.StatusBadRequest, InvalidRequest, "params format error, must be an array or a map")
		return
	}
	log.Debug("RPC method:", requestMethod)

	response := method(params)
	var data []byte
	if response["Error"] != elaErr.ServerErrCode(0) {
		data, _ = json.Marshal(map[string]interface{}{
			"jsonrpc": "2.0",
			"result":  nil,
			"error": map[string]interface{}{
				"code":    response["Error"],
				"message": response["Result"],
				"id":      request["id"],
			},
		})

	} else {
		data, _ = json.Marshal(map[string]interface{}{
			"jsonrpc": "2.0",
			"result":  response["Result"],
			"id":      request["id"],
			"error":   nil,
		})
	}
	w.Header().Set("Content-type", "application/json")
	w.Write(data)
}

func clientAllowed(r *http.Request) bool {
	//this ipAbbr  may be  ::1 when request is localhost
	ipAbbr, _, err := net.SplitHostPort(r.RemoteAddr)
	if err != nil {
		log.Errorf("RemoteAddr clientAllowed SplitHostPort failure %s \n", r.RemoteAddr)
		return false

	}
	//after ParseIP ::1 chg to 0:0:0:0:0:0:0:1 the true ip
	remoteIp := net.ParseIP(ipAbbr)

	if remoteIp == nil {
		log.Errorf("clientAllowed ParseIP ipAbbr %s failure  \n", ipAbbr)
		return false
	}

	if remoteIp.IsLoopback() {
		return true
	}

	for _, cfgIp := range config.Parameters.RpcConfiguration.WhiteIPList {
		//WhiteIPList have 0.0.0.0  allow all ip in
		if cfgIp == "0.0.0.0" {
			return true
		}
		if cfgIp == remoteIp.String() {
			return true
		}

	}
	return false
}

func checkAuth(r *http.Request) bool {
	if (config.Parameters.RpcConfiguration.User == config.Parameters.RpcConfiguration.Pass) &&
		(len(config.Parameters.RpcConfiguration.User) == 0) {
		return true
	}
	authHeader := r.Header["Authorization"]
	if len(authHeader) <= 0 {
		return false
	}

	authSha256 := sha256.Sum256([]byte(authHeader[0]))

	login := config.Parameters.RpcConfiguration.User + ":" + config.Parameters.RpcConfiguration.Pass
	auth := "Basic " + base64.StdEncoding.EncodeToString([]byte(login))
	cfgAuthSha256 := sha256.Sum256([]byte(auth))

	resultCmp := subtle.ConstantTimeCompare(authSha256[:], cfgAuthSha256[:])
	if resultCmp == 1 {
		return true
	}

	// Request's auth doesn't match  user
	return false
}

func RPCError(w http.ResponseWriter, httpStatus int, code elaErr.ServerErrCode, message string) {
	data, _ := json.Marshal(map[string]interface{}{
		"jsonrpc": "2.0",
		"result":  nil,
		"error": map[string]interface{}{
			"code":    code,
			"message": message,
			"id":      nil,
		},
	})
	w.Header().Set("Content-type", "application/json")
	w.WriteHeader(httpStatus)
	w.Write(data)
}

func convertParams(method string, params []interface{}) Params {
	switch method {
	case "createauxblock":
		return FromArray(params, "paytoaddress")
	case "submitauxblock":
		return FromArray(params, "blockhash", "auxpow")
	case "getblockhash":
		return FromArray(params, "height")
	case "getblock":
		return FromArray(params, "blockhash", "verbosity")
	case "setloglevel":
		return FromArray(params, "level")
	case "getrawtransaction":
		return FromArray(params, "txid", "verbose")
	case "getarbitratorgroupbyheight":
		return FromArray(params, "height")
	case "togglemining":
		return FromArray(params, "mining")
	case "discretemining":
		return FromArray(params, "count")
	case "sendrawtransaction":
		return FromArray(params, "data")
	case "listunspent":
		return FromArray(params, "addresses")
	case "getreceivedbyaddress":
		return FromArray(params, "address")
	case "getblockbyheight":
		return FromArray(params, "height")
	case "estimatesmartfee":
		return FromArray(params, "confirmations")
	default:
		return Params{}
	}
}
