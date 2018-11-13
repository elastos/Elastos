package arbitrator

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

var ArbitratorGroupSingleton ArbitratorGroup

type ArbitratorGroupListener interface {
	GetPublicKey() string
	OnDutyArbitratorChanged(onDuty bool)
}

type ArbitratorGroup struct {
	Arbitrators []string
	OnDutyIndex uint32

	listener ArbitratorGroupListener
}

func (ag *ArbitratorGroup) Initialize(listener ArbitratorGroupListener) {
	ag.listener = listener
}

func (ag *ArbitratorGroup) ChangeHeight() {
	blockHeight := blockchain.DefaultLedger.Blockchain.BlockHeight
	ag.OnDutyIndex = blockHeight % uint32(len(ag.Arbitrators))
	log.Info("[ChangeHeight] block height:", blockHeight, "old index:", ag.OnDutyIndex, "current index:", ag.OnDutyIndex)

	ag.listener.OnDutyArbitratorChanged(ag.Arbitrators[ag.OnDutyIndex] == ag.listener.GetPublicKey())
}

func GetCurrentArbitrator(viewOffset uint32) string {
	a := ArbitratorGroupSingleton.Arbitrators[(ArbitratorGroupSingleton.OnDutyIndex+viewOffset)%uint32(len(ArbitratorGroupSingleton.Arbitrators))]
	log.Info("[GetCurrentArbitrator] arbiter:", a, " offset:", viewOffset)
	return a
}

func init() {
	ArbitratorGroupSingleton = ArbitratorGroup{
		Arbitrators: []string{"A", "B", "C", "D", "E"},
		OnDutyIndex: 0,
	}
}
