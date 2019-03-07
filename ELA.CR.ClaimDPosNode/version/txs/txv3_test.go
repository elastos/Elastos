package txs

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/version/verconf"

	"github.com/stretchr/testify/suite"
)

type txVersionV3TestSuite struct {
	suite.Suite

	Version TxVersion
	Cfg     *verconf.Config
}

func (s *txVersionV3TestSuite) SetupTest() {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	s.Cfg = &verconf.Config{}
	s.Version = NewTxV3(s.Cfg)
}

func TestTxVersionV3Suit(t *testing.T) {
	suite.Run(t, new(txVersionV3TestSuite))
}
