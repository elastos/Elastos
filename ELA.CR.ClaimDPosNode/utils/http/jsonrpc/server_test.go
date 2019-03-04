package jsonrpc

import (
	"testing"

	"github.com/elastos/Elastos.ELA.Utility/http/util"
	"fmt"

	//sv "github.com/elastos/Elastos.ELA.SideChain.Token/service"

	//"github.com/elastos/Elastos.ELA.SideChain/service"

	"github.com/elastos/Elastos.ELA/utils/test"
	"time"
	htp "github.com/elastos/Elastos.ELA/utils/http"

	"bytes"
	"net/http"
	"github.com/stretchr/testify/assert"
)


func SetLogLevel(param util.Params) (interface{}, error) {
	level, ok := param["level"].(float64)
	if !ok || level < 0 {
		return nil, util.NewError(int(InvalidParams), "level must be an integer in 0-6")
	}

	//s.cfg.SetLogLevel(elalog.Level(level))
	return fmt.Sprint("log level has been set to ", level), nil
}


func TestServer_ServeHTTP(t *testing.T) {

	t.Logf("TestServer_ServeHTTP")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("Receive POST request from path %s, data %V", "/api/test", data)
		//if !bytes.Equal(data, []byte("data")) {
		//	t.Errorf("expected data = data get %s", data)
		//}
		return nil, nil
	}, "level")
	//assert.NoError(t, err)

	go s.Start()

	req := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	req_new := bytes.NewBuffer([]byte(req))
	_, err := http.Post("http://localhost:20336/api/test", "application/json",
		req_new)//bytes.NewReader([]byte("data"))
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	select {
	case <-time.After(time.Second*1):
		s.Stop()
	}
}
