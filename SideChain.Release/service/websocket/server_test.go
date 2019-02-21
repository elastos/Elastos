package websocket

import (
	"net/url"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/gorilla/websocket"
	"github.com/stretchr/testify/assert"
)

func mockClient(t *testing.T, msgIn chan []byte, msgOut chan []byte) error {
	u := url.URL{Scheme: "ws", Host: "localhost:22605"}
	conn, _, err := websocket.DefaultDialer.Dial(u.String(), nil)
	if err != nil {
		return err
	}

	go func() {
		for {
			_, msg, err := conn.ReadMessage()
			if err != nil {
				t.Fatal(err)
			}
			msgOut <- msg
		}
	}()

	go func() {
		for msg := range msgIn {
			err := conn.WriteMessage(websocket.TextMessage, msg)
			if err != nil {
				t.Fatal(err)
			}
		}
	}()

	return nil
}

func TestNewServer(t *testing.T) {
	svrCfg := service.Config{
		GetPayload:     service.GetPayload,
		GetPayloadInfo: service.GetPayloadInfo,
	}
	service := service.NewHttpService(&svrCfg)
	server := NewServer(&Config{
		ServiceCfg: &svrCfg,
		ServePort:  22605,
		Flags:      PFRawBlock | PFNewTx,
		Service:    service,
	})
	defer server.Stop()
	go server.Start()

	// Mock a web socket client.
	msgIn := make(chan []byte)
	msgOut := make(chan []byte)
	err := mockClient(t, msgIn, msgOut)
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// Mock block broadcast.
	events.Notify(events.ETBlockConnected, &types.Block{})
	select {
	case msg := <-msgOut:
		t.Log(string(msg))
	case <-time.After(time.Second):
		t.Fatal("Broadcast block timeout")
	}

	// Mock transaction broadcast.
	events.Notify(events.ETTransactionAccepted, &types.Transaction{
		Payload: &types.PayloadCoinBase{}})
	select {
	case msg := <-msgOut:
		t.Log(string(msg))
	case <-time.After(time.Second):
		t.Fatal("Broadcast transaction timeout")
	}
}
