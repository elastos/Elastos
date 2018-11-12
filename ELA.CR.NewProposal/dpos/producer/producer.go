package producer

import (
	"bytes"
	"errors"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	"github.com/elastos/Elastos.ELA/dpos/chain"
	"github.com/elastos/Elastos.ELA/dpos/config"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

var ProducerSingleton *Producer

type Producer struct {
	CurrentHeight  uint32
	ReceivedBlocks map[common.Uint256]uint32

	producerLock sync.Mutex
}

func (p *Producer) OnBlockReceived(peer *peer.Peer, b *chain.Block) {
	p.producerLock.Lock()
	defer p.producerLock.Unlock()

	log.Info("[OnBlockReceived] started, hash:", b.Hash)

	if _, ok := p.ReceivedBlocks[b.Hash]; ok {
		log.Info("Received exist block")
		return
	}
	p.ReceivedBlocks[b.Hash] = b.Height
	log.Info("[OnConfirmReceived] end")
}

func (p *Producer) OnConfirmReceived(peer *peer.Peer, s *chain.ProposalVoteSlot) {
	p.producerLock.Lock()
	defer p.producerLock.Unlock()

	log.Info("[OnConfirmReceived] started, hash:", s.Hash)

	if len(s.Votes) == 0 {
		log.Warn("Received unsigned block.")
		return
	}

	if _, ok := p.ReceivedBlocks[s.Hash]; !ok {
		log.Info("Received invalid confirm message")
		return
	}

	if p.ReceivedBlocks[s.Hash] <= p.CurrentHeight {
		log.Warn("Received old signed block.")
		return
	}

	p.CurrentHeight = p.ReceivedBlocks[s.Hash]
	log.Info("[OnConfirmReceived] local chain height:", p.CurrentHeight)
	time.Sleep(time.Duration(config.Parameters.SleepDuration) * time.Second)
	p.Produce(s.Hash)

	log.Info("[OnConfirmReceived] end")

}

func (p *Producer) Produce(prevHash common.Uint256) error {
	hash, err := getBlockHash(config.Parameters.Name, p.CurrentHeight+uint32(1))
	if err != nil {
		log.Warn("Produce err: ", err)
		return err
	}

	block := chain.Block{
		Hash:      hash,
		PrevHash:  prevHash,
		Height:    p.CurrentHeight + uint32(1),
		TimeStamp: uint64(time.Now().Unix()),
	}

	blockMsg := &cs.BlockMessage{
		Command: cs.ReceivedBlock,
		Block:   block,
	}

	log.Info("Produced block:", block.Hash)

	cs.P2PClientSingleton.AddMessageHash(cs.P2PClientSingleton.GetMessageHash(blockMsg), p.CurrentHeight)
	cs.P2PClientSingleton.Broadcast(blockMsg)

	if _, ok := p.ReceivedBlocks[block.Hash]; ok {
		return errors.New("Produce block failed")
	}
	p.ReceivedBlocks[block.Hash] = block.Height
	return nil
}

func getBlockHash(name string, height uint32) (common.Uint256, error) {
	buf := new(bytes.Buffer)
	if err := common.WriteVarString(buf, name); err != nil {
		return common.Uint256{}, err
	}

	if err := common.WriteUint32(buf, height); err != nil {
		return common.Uint256{}, err
	}

	msgHash := common.Sha256D(buf.Bytes())
	return msgHash, nil
}
