package blocks

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/version/verconf"
	"github.com/stretchr/testify/suite"
)

type blockVersionV0TestSuite struct {
	suite.Suite

	Version BlockVersion
	Cfg     *verconf.Config
}

func (s *blockVersionV0TestSuite) SetupTest() {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}

	s.Cfg = &verconf.Config{
		ChainParams: &config.DefaultParams,
	}
	s.Version = NewBlockV0(s.Cfg)
}

func (s *blockVersionV0TestSuite) TestBlockVersionMain_GetNextOnDutyArbitrator() {

	// fixme chain store removed, fix me later
	//arbitrators := make([][]byte, 0)
	//for _, v := range config.DefaultParams.OriginArbiters {
	//	a, _ := common.HexStringToBytes(v)
	//	arbitrators = append(arbitrators, a)
	//}
	//
	//originLedger := blockchain.DefaultLedger
	//chainMock := &blockchain.ChainStoreMock{
	//	BlockHeight: 0,
	//}
	//blockchain.DefaultLedger = &blockchain.Ledger{
	//	Store: chainMock,
	//}
	//
	//var currentArbitrator []byte
	//
	//chainMock.BlockHeight = 0
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(1, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(2, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(3, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(4, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(5, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//
	//chainMock.BlockHeight = 0
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//
	//chainMock.BlockHeight = 1
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[1], currentArbitrator)
	//
	//chainMock.BlockHeight = 2
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[2], currentArbitrator)
	//
	//chainMock.BlockHeight = 3
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[3], currentArbitrator)
	//
	//chainMock.BlockHeight = 4
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[4], currentArbitrator)
	//
	//chainMock.BlockHeight = 5
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 0)
	//s.Equal(arbitrators[0], currentArbitrator)
	//
	//chainMock.BlockHeight = 0
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 1)
	//s.Equal(arbitrators[1], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 2)
	//s.Equal(arbitrators[2], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 3)
	//s.Equal(arbitrators[3], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 4)
	//s.Equal(arbitrators[4], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 5)
	//s.Equal(arbitrators[0], currentArbitrator)
	//currentArbitrator = s.Version.GetNextOnDutyArbitratorV(0, 6)
	//s.Equal(arbitrators[1], currentArbitrator)
	//
	//blockchain.DefaultLedger = originLedger
}

func TestBlockVersionV0Suit(t *testing.T) {
	suite.Run(t, new(blockVersionV0TestSuite))
}
