// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <iostream>
#include <sqlite3.h>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "SPVSDK.h"
#include "Wallet.h"
#include "Account.h"
#include "TestConnectPeer.h"
#include "Transaction.h"

//using namespace std;
//using namespace boost;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {

    SPVSDK SDK;
    std::string version = SDK.version();
    std::cout << version << std::endl;

    TestConnectPeer::runPeerConnectTest();
    return 0;

    std::string strName = "NAME:\n         cli-spv - command line tool for ELA blockchain \n";
    std::string strUsage = "USAGE:\n         cli-spv [global options] command [command options] [args] \n";
    std::string strDescription = "DESCRIPTION:\n         with cli-spv wallet, you can create an account, check "
                                 "account balance or build, sign and send transactions. \n\n";
    std::string descName = strName.append(strUsage).append(strDescription).append("OPTIONS");

    po::options_description desc(descName);
    desc.add_options()
            ("password,p", po::value<std::string>(), "arguments to pass the password value")
            ("name,n", po::value<std::string>(),
             "to specify the created keystore file name or the keystore file path to open")
            ("create,c", "create wallet, this will generate a keystore file within you account information")
            ("account,a", "show account address, public key and program hash")
            ("changepassword", "change the password to access this wallet, must do not forget it")
            ("reset", "clear the UTXOs stored in the local database")
            ("addaccount", po::value<std::string>(), "add a standard account with a public key")
            ("addmultisignaccount", "add a multi-sign account with multiple public keys\n "
                                    "\tuse -m to specify how many signatures are needed to create a valid transaction\n "
                                    "\tby default M is public keys / 2 + 1, witch means greater than half")
            ("m", po::value<int>(),
             "the M value to specify how many signatures are needed to create a valid transaction")
            ("deleteaccount", po::value<std::string>(), "delete an account from database using it's address")
            ("balance,b", "list account balances stored in this wallet")
            ("transaction,t", po::value<std::string>(),
             "use [create, sign, send], to create, sign or send a transaction\n"
             "create:\n"
             "\tuse --to --amount --fee [--lock], or --file --fee [--lock]\n"
             "\tto create a standard transaction, or multi output transaction\n"
             "sign, send:\n"
             "\tuse --file or --hex to specify the transaction file path or content\n")
            ("from", po::value<std::string>(), "the spend address of the transaction")
            ("to", po::value<std::string>(), "the receive address of the transaction")
            ("amount", po::value<std::string>(), "the transfer amount of the transaction")
            ("fee", po::value<std::string>(), "the transfer fee of the transaction")
            ("lock", po::value<std::string>(), "the lock time to specify when the received asset can be spent")
            ("hex", po::value<std::string>(), "the transaction content in hex string format to be sign or send")
            ("file,f", po::value<std::string>(),
             "the file path to specify a CSV file path with [address,amount] format as multi output content,\n"
             "\tor the transaction file path with the hex string content to be sign or send")
            ("key", po::value<std::string>(), "the public key of target account")
            ("deposit", po::value<std::string>(), "create deposit transaction")
            ("withdraw", po::value<std::string>(), "create withdraw transaction");


    if (argc > 1) {
        std::string commandType = argv[1];
        po::variables_map vm;
        try {
            po::store(po::parse_command_line(argc, argv, desc), vm);

            if (commandType == "wallet" && Elastos::APP::Wallet::walletAction(vm)) {
                return EXIT_SUCCESS;
            }
        }
        catch (po::error &e) {
            std::cout << "invalid parameters ! /n" << std::endl;
        }
    }

    std::cout << desc << std::endl;
    return EXIT_FAILURE;
}
