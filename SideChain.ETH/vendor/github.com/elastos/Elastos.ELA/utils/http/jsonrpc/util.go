package jsonrpc

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"net/http"

	htp "github.com/elastos/Elastos.ELA/utils/http"
)

// Call is a util method to send a JSON-RPC request to server.
func Call(url string, reqData Request, rpcUser string, rpcPassword string) (interface{}, error) {
	data, err := json.Marshal(reqData)
	if err != nil {
		return nil, err
	}
	req, err := http.NewRequest("POST", url, bytes.NewReader(data))
	if err != nil {
		return nil, err
	}
	req.Header.Set("Content-Type", "application/json")
	req.SetBasicAuth(rpcUser, rpcPassword)
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	ret := Response{}
	if err := json.Unmarshal(body, &ret); err != nil {
		return nil, err
	}

	if ret.Error != nil {
		return nil, ret.Error
	}

	return ret.Result, nil
}

func CallParams(url, method string, params htp.Params) (interface{}, error) {
	req := Request{
		Method: method,
		Params: params,
	}
	return Call(url, req, "", "")
}

// CallArray is a util method to send a JSON-RPC request to server.
func CallArray(url, method string, params ...interface{}) (interface{}, error) {
	req := Request{
		Method: method,
		Params: params,
	}
	return Call(url, req, "", "")
}
