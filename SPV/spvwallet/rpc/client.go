package rpc

import (
	"bytes"
	"encoding/json"
	"net/http"
	"io/ioutil"
	"errors"

	. "github.com/elastos/Elastos.ELA/core"
	"encoding/hex"
)

type Client struct {
	url string
}

func GetClient() *Client {
	return &Client{url: RPCAddr}
}

func (client *Client) NotifyNewAddress(hash []byte) error {
	resp := client.send(
		&Req{
			Method: "notifynewaddress",
			Params: []interface{}{hex.EncodeToString(hash)},
		},
	)
	if resp.Code != 0 {
		return errors.New(resp.Result.(string))
	}
	return nil
}

func (client *Client) SendTransaction(tx *Transaction) error {
	buf := new(bytes.Buffer)
	tx.Serialize(buf)
	resp := client.send(
		&Req{
			Method: "sendtransaction",
			Params: []interface{}{hex.EncodeToString(buf.Bytes())},
		},
	)
	if resp.Code != 0 {
		return errors.New(resp.Result.(string))
	}
	return nil
}

func (client *Client) send(req *Req) (ret Resp) {
	data, err := json.Marshal(req)
	if err != nil {
		return MarshalRequestError
	}

	resp, err := http.Post(client.url, "application/json", bytes.NewReader(data))
	if err != nil {
		return PostRequestError
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return ReadResponseError
	}

	err = json.Unmarshal(body, &ret)
	if err != nil {
		return UnmarshalResponseError
	}

	return ret
}
