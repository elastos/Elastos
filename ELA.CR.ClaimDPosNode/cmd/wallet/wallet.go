package wallet

import (
	"github.com/urfave/cli"
)

func NewCommand() *cli.Command {
	var subCommands []cli.Command
	subCommands = append(subCommands, txCommand...)
	subCommands = append(subCommands, accountCommand...)

	return &cli.Command{
		Name:        "wallet",
		Usage:       "Wallet operations",
		Description: "With ela-cli wallet, you could control your asset.",
		ArgsUsage:   "[args]",
		Subcommands: subCommands,
	}
}
