// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package main

import (
	"fmt"
	"os"

	bencli "github.com/elastos/Elastos.ELA/benchmark/common/cli"
	"github.com/elastos/Elastos.ELA/benchmark/tools/generator/chain"
	"github.com/elastos/Elastos.ELA/utils/signal"

	"github.com/urfave/cli"
)

var (
	Version        string
	essentialFlags []cli.Flag
)

func main() {
	app := cli.NewApp()
	app.Name = "ela-datagen"
	app.Version = Version
	app.HelpName = "ela-datagen"
	app.Usage = "command line tools for ELA data generation"
	app.UsageText = "ela-datagen [global options] command [command options] [args]"
	app.HideHelp = false
	app.HideVersion = false
	essentialFlags = []cli.Flag{
		bencli.WorkingDirFlag,
		bencli.HeightFlag,
		bencli.GenerationModeFlag,
		bencli.PrepareStartHeightFlag,
		bencli.RandomStartHeightFlag,
		bencli.InputsPerBlockFlag,
		bencli.MaxRefersCountFlag,
		bencli.MinRefersCountFlag,
		bencli.AddressCountFlag,
	}
	app.Flags = essentialFlags
	app.Action = func(c *cli.Context) error {
		return generate(c)
	}

	if err := app.Run(os.Args); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

func generate(c *cli.Context) error {
	if !c.IsSet(bencli.HeightFlag.Name) {
		return fmt.Errorf("generation height is missing")
	}

	if !c.IsSet(bencli.WorkingDirFlag.Name) {
		return fmt.Errorf("working dir is missing")
	}

	params, err := parseParams(c)
	if err != nil {
		return err
	}

	var interrupt = signal.NewInterrupt()
	gen, err := chain.NewDataGen(c.String(bencli.WorkingDirFlag.Name),
		interrupt.C, params)
	if err != nil {
		return err
	}

	exit := make(chan error)
	go func() {
		err := gen.Generate(uint32(c.Uint64(bencli.HeightFlag.Name)))
		if err != nil {
			exit <- err
			return
		}

		err = gen.Save()
		gen.Exit()
		exit <- err
	}()

	select {
	case err := <-exit:
		return err
	case <-interrupt.C:
		return nil
	}
}

func parseParams(c *cli.Context) (params *chain.GenerationParams, err error) {
	params = &chain.GenerationParams{
		Mode:               modeFromString(bencli.GenerationModeFlag.Value),
		PrepareStartHeight: uint32(bencli.PrepareStartHeightFlag.Value),
		RandomStartHeight:  uint32(bencli.RandomStartHeightFlag.Value),
		InputsPerBlock:     uint32(bencli.InputsPerBlockFlag.Value),
		MaxRefersCount:     uint32(bencli.MaxRefersCountFlag.Value),
		MinRefersCount:     uint32(bencli.MinRefersCountFlag.Value),
		AddressCount:       uint32(bencli.AddressCountFlag.Value),
	}

	if c.IsSet(bencli.GenerationModeFlag.Name) {
		params.Mode = modeFromString(c.String(bencli.GenerationModeFlag.Name))
	}
	if c.IsSet(bencli.PrepareStartHeightFlag.Name) {
		params.PrepareStartHeight =
			uint32(c.Uint64(bencli.PrepareStartHeightFlag.Name))
	}
	if c.IsSet(bencli.RandomStartHeightFlag.Name) {
		params.RandomStartHeight =
			uint32(c.Uint64(bencli.RandomStartHeightFlag.Name))
	}
	if c.IsSet(bencli.InputsPerBlockFlag.Name) {
		params.InputsPerBlock = uint32(c.Uint64(bencli.InputsPerBlockFlag.Name))
	}
	if c.IsSet(bencli.MaxRefersCountFlag.Name) {
		params.MaxRefersCount = uint32(c.Uint64(bencli.MaxRefersCountFlag.Name))
	}
	if c.IsSet(bencli.MinRefersCountFlag.Name) {
		params.MinRefersCount = uint32(c.Uint64(bencli.MinRefersCountFlag.Name))
	}
	if c.IsSet(bencli.AddressCountFlag.Name) {
		params.AddressCount = uint32(c.Uint64(bencli.AddressCountFlag.Name))
	}

	return
}

func modeFromString(str string) chain.GenerationMod {
	switch str {
	case "fast":
		return chain.Fast
	case "normal":
		return chain.Normal
	case "minimal":
		return chain.Minimal
	default:
		return chain.Fast
	}
}
