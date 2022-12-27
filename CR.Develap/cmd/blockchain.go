/*
Copyright © 2019 Cyber Republic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
package cmd

import (
	"os"

	"github.com/cyber-republic/develap/cmd/blockchain"
	"github.com/spf13/cobra"
)

// blockchainCmd represents the blockchain command
var blockchainCmd = &cobra.Command{
	Use:   "blockchain",
	Short: "Interact with blockchain nodes",
	Long:  `Interact with blockchain nodes`,
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Help()
		os.Exit(1)
	},
}

func init() {
	blockchainCmd.AddCommand(blockchain.ListCmd)
	blockchainCmd.AddCommand(blockchain.RunCmd)
	blockchainCmd.AddCommand(blockchain.KillCmd)
	rootCmd.AddCommand(blockchainCmd)

	// Here you will define your flags and configuration settings.

	// Cobra supports Persistent Flags which will work for this command
	// and all subcommands, e.g.:
	// blockchainCmd.PersistentFlags().String("foo", "", "A help for foo")
	blockchainCmd.PersistentFlags().StringVarP(&blockchain.Env, "env", "e", "", "environment to use [mainnet,testnet,localnet]")
	blockchainCmd.MarkFlagRequired("env")

	// Cobra supports local flags which will only run when this command
	// is called directly, e.g.:
	//blockchainCmd.Flags().BoolP("toggle", "t", false, "Help message for toggle")
}
