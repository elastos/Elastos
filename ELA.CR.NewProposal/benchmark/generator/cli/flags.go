package cli

import (
	"github.com/urfave/cli"
)

var (
	WorkingDirFlag = cli.StringFlag{
		Name:  "dir",
		Usage: "specify the working dir to generate data",
		Value: "",
	}
	HeightFlag = cli.Uint64Flag{
		Name:  "height",
		Usage: "specify generation height",
	}
	GenerationModeFlag = cli.StringFlag{
		Name:  "mode",
		Usage: "set the generation mode, default is fast mode",
		Value: "fast",
	}
	PrepareStartHeightFlag = cli.Uint64Flag{
		Name:  "prepareheight",
		Usage: "specify the prepare start height that can allocate fund",
		Value: 100,
	}
	RandomStartHeightFlag = cli.Uint64Flag{
		Name:  "randomheight",
		Usage: "specify the random start height that can consume UTXO",
		Value: 200,
	}
	InputsPerBlockFlag = cli.Uint64Flag{
		Name:  "inputsperblock",
		Usage: "specify inputs count of each blocks",
		Value: 5000,
	}
	MaxRefersCountFlag = cli.Uint64Flag{
		Name:  "maxrefers",
		Usage: "specify max count each block should have after random height",
		Value: 5000,
	}
	MinRefersCountFlag = cli.Uint64Flag{
		Name:  "minrefers",
		Usage: "specify min count each block should have after random height",
		Value: 2000,
	}
	AddressCountFlag = cli.Uint64Flag{
		Name:  "addresscount",
		Usage: "specify count of addresses need to be generated",
		Value: 500,
	}
)
