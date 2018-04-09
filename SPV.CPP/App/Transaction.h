//
// Created by jzh on 18-4-9.
//

#ifndef __SPVCLIENT_TRANSACTION_H__
#define __SPVCLIENT_TRANSACTION_H__

#include <iostream>
#include <boost/program_options/variables_map.hpp>

#include "TWallet.h"

namespace Elastos {
    namespace APP {

        class Transaction {
        public:
            static bool createTransaction(boost::shared_ptr<TWallet> twallet, const boost::program_options::variables_map& vm);
            static bool createMultiOutputTransaction(boost::shared_ptr<TWallet> twallet, const boost::program_options::variables_map& vm,
                                                     std::string path, std::string from, std::string fee);
            static bool signTransaction(boost::shared_ptr<TWallet> twallet, const boost::program_options::variables_map& vm,
                                        std::string name, std::string password);
            static bool sendTransaction(boost::shared_ptr<TWallet> twallet, const boost::program_options::variables_map& vm);

        private:
            static bool getTransactionContent(const boost::program_options::variables_map& vm, std::string& content);
        };

    }
}


#endif //SPVCLIENT_TRANSACTION_H
