package config

import (
	"math/big"
	"time"
)

var (
	mainNet = &ChainParams{
		Name:               "MainNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1f0008ff,
		TargetTimePerBlock: time.Minute * 2,
		TargetTimespan:     time.Minute * 2 * 720,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
	testNet = &ChainParams{
		Name:               "TestNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x1e1da5ff,
		TargetTimePerBlock: time.Second * 10,
		TargetTimespan:     time.Second * 10 * 10,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
	regNet = &ChainParams{
		Name:               "RegNet",
		PowLimit:           new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1)),
		PowLimitBits:       0x207fffff,
		TargetTimePerBlock: time.Second * 1,
		TargetTimespan:     time.Second * 1 * 10,
		AdjustmentFactor:   int64(4),
		MaxOrphanBlocks:    10000,
		MinMemoryNodes:     20160,
		CoinbaseLockTime:   100,
	}
)
