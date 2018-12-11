package jsonrpc

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

const (
	// JSON-RPC protocol version.
	Version = "2.0"
	// JSON-RPC protocol error codes.
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
	//-32000 to -32099	Server error, waiting for defining
)

// Handler is the registered method to handle a http request.
type Handler func(util.Params) (interface{}, error)

// Request represent the standard JSON-RPC request data structure.
type Request struct {
	Id      interface{} `json:"id"`
	Version string      `json:"jsonrpc"`
	Method  string      `json:"method"`
	Params  interface{} `json:"params"`
}

// Response represent the standard JSON-RPC Response data structure.
type Response struct {
	Id      interface{} `json:"id"`
	Version string      `json:"jsonrpc"`
	Result  interface{} `json:"result"`
	Error   *util.Error `json:"error"`
}

// error returns an error response to the http client.
func (r *Response) error(w http.ResponseWriter, httpStatus, code int, message string) {
	r.Error = &util.Error{
		Code:    code,
		Message: message,
	}
	r.write(w, httpStatus)
}

// write returns a normal response to the http client.
func (r *Response) write(w http.ResponseWriter, httpStatus int) {
	r.Version = Version
	w.Header().Set("Content-Type", "application/json;charset=utf-8")
	w.WriteHeader(httpStatus)
	data, _ := json.Marshal(r)
	w.Write(data)
}

// Config is the configuration of the JSON-RPC server.
type Config struct {
	Path      string
	ServePort uint16
	NetListen func(port uint16) (net.Listener, error)
}

// Server is the JSON-RPC server instance class.
type Server struct {
	cfg    Config
	server *http.Server

	mutex     sync.Mutex
	paramsMap map[string][]string
	handlers  map[string]Handler
}

// RegisterAction register a service handler method by it's name and parameters. When a
// JSON-RPC client's request method matches the registered handler name, it will be invoked.
// This method is safe for concurrency access.
func (s *Server) RegisterAction(name string, handler Handler, params ...string) {
	s.mutex.Lock()
	s.paramsMap[name] = params
	s.handlers[name] = handler
	s.mutex.Unlock()
}

func (s *Server) Start() error {
	if s.cfg.ServePort == 0 {
		return fmt.Errorf("jsonrpc ServePort not configured")
	}

	var err error
	var listener net.Listener
	if s.cfg.NetListen != nil {
		listener, err = s.cfg.NetListen(s.cfg.ServePort)
	} else {
		listener, err = net.Listen("tcp", fmt.Sprint(":", s.cfg.ServePort))
	}
	if err != nil {
		return err
	}

	if s.cfg.Path == "" {
		s.server = &http.Server{Handler: s}
	} else {
		http.Handle(s.cfg.Path, s)
		s.server = &http.Server{}
	}
	return s.server.Serve(listener)
}

func (s *Server) Stop() error {
	if s.server != nil {
		return s.server.Shutdown(context.Background())
	}
	return fmt.Errorf("server not started")
}

func (s *Server) parseParams(method string, array []interface{}) util.Params {
	s.mutex.Lock()
	fields := s.paramsMap[method]
	s.mutex.Unlock()

	params := make(util.Params)
	count := min(len(array), len(fields))
	for i := 0; i < count; i++ {
		params[fields[i]] = array[i]
	}
	return params
}

func (s *Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	//JSON RPC commands should be POSTs
	if r.Method != "POST" {
		log.Warn("HTTP JSON RPC Handle - Method!=\"POST\"")
		http.Error(w, "JSON RPC procotol only allows POST method",
			http.StatusMethodNotAllowed)
		return
	}

	if r.Header["Content-Type"][0] != "application/json" {
		log.Warn("HTTP JSON RPC Handle - Content-Type: ",
			r.Header["Content-Type"][0], " not supported")
		http.Error(w, "need content type to be application/json",
			http.StatusUnsupportedMediaType)
		return
	}

	//read the body of the request
	body, _ := ioutil.ReadAll(r.Body)
	var req Request
	var resp Response
	err := json.Unmarshal(body, &req)
	if err != nil {
		log.Warn("HTTP JSON RPC Handle - json.Unmarshal: ", err)
		resp.error(w, http.StatusBadRequest, ParseError,
			fmt.Sprintf("json parse failed: %s",err))
		return
	}

	resp.Id = req.Id

	if len(req.Method) == 0 {
		resp.error(w, http.StatusBadRequest, InvalidRequest,
			"method parameter not found")
		return
	}
	handler, ok := s.handlers[req.Method]
	if !ok {
		resp.error(w, http.StatusNotFound, MethodNotFound,
			fmt.Sprintf("method %s not found", req.Method))
		return
	}

	// Json rpc 1.0 support positional parameters while json rpc 2.0 support
	// named parameters.
	// Positional parameters: { "params":[1, 2, 3....] }
	// named parameters: { "params":{ "a":1, "b":2, "c":3 } }
	// Here we support both of them.
	var params util.Params
	switch requestParams := req.Params.(type) {
	case nil:
	case []interface{}:
		params = s.parseParams(req.Method, requestParams)
	case map[string]interface{}:
		params = util.Params(requestParams)
	default:
		resp.error(w, http.StatusBadRequest, InvalidRequest,
			"params format err, must be an array or a map")
		return
	}

	result, err := handler(params)
	if err != nil {
		code := InternalError
		message := fmt.Sprintf("internal error: %s", err)

		switch e := err.(type) {
		case *util.Error:
			code = e.Code
			message = e.Message
		}

		resp.error(w, http.StatusInternalServerError, code, message)
		return
	}

	resp.Result = result
	resp.write(w, http.StatusOK)
}

// NewServer creates and return a JSON-RPC server instance.
func NewServer(cfg *Config) *Server {
	return &Server{
		cfg:       *cfg,
		paramsMap: make(map[string][]string),
		handlers:  make(map[string]Handler),
	}
}

func min(a int, b int) int {
	if a > b {
		return b
	}
	return a
}
