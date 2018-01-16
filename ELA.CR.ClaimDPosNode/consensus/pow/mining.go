package pow

import (
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/ledger"
	"fmt"
	"time"

	zmq "github.com/pebbe/zmq4"
)

const (
	MSGHASKBLOCK = "hashblock"
	MSGHASKTX    = "hashtx"
)

func (pow *PowService) ZMQClientSend(MsgBlock ledger.Block) {
	requester, err := zmq.NewSocket(zmq.REQ)
	defer requester.Close()
	if nil != err {
		log.Error("New ZMQ socket err", err.Error())
		return
	}

	serverIP := fmt.Sprintf("tcp://%s:%d", config.Parameters.PowConfiguration.MiningServerIP,
		config.Parameters.PowConfiguration.MiningServerPort)

	err = requester.Connect(serverIP)
	if nil != err {
		log.Error("ZMQ Connect err", err.Error())
		return
	}

	_, err = requester.Send("Hello world", zmq.SNDMORE)
	if nil != err {
		log.Error("ZMQ Send err", err.Error())
	}

	requester.Disconnect(serverIP)
}

func (pow *PowService) ZMQServer() {
	//  Socket to talk to clients
	log.Info("ZMQ Service Start")
	publisher, _ := zmq.NewSocket(zmq.PUB)
	defer publisher.Close()

	bindIP := fmt.Sprintf("tcp://*:%d", config.Parameters.PowConfiguration.MiningSelfPort)
	publisher.Bind(bindIP)
	for {
		select {
		case <-pow.ZMQPublish:
			publisher.Send(MSGHASKTX+"==Coming from elacoin node, glad to see you, Timestamp:"+string(time.Now().Unix()), zmq.SNDMORE)
		}
	}
}
