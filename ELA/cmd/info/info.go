package info

import "github.com/urfave/cli"

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:      "info",
		Usage:     "Display information about the blockchain",
		ArgsUsage: "[args]",
	}
}
