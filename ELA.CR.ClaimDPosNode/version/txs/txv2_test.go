package txs

import (
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/version/verconf"
	"testing"

	"github.com/stretchr/testify/suite"
)

type txVersionV2TestSuite struct {
	suite.Suite

	Version TxVersion
	Cfg     *verconf.Config
}

func (s *txVersionV2TestSuite) SetupTest() {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}

	s.Cfg = &verconf.Config{}
	s.Version = NewTxV2(s.Cfg)
}

func TestTxVersionV2Suit(t *testing.T) {
	suite.Run(t, new(txVersionV2TestSuite))
}
