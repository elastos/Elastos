//
// Created by jzh on 18-4-9.
//

#ifndef __SPVCLIENT_TWALLET_H__
#define __SPVCLIENT_TWALLET_H__

#include <string>
#include <vector>

namespace Elastos {
    namespace APP {

        struct TxTransaction {

        };

        struct Address {
            std::string Address;
            std::string ProgramHash ;
            char* RedeemScript;
        };

        typedef std::vector<Address> Addresses;

        class TWallet {
        public:
            TWallet();
            ~TWallet();

        public:
            TxTransaction createTransaction(std::string from, std::string to, std::string amount, std::string fee);
            TxTransaction createLockedTransaction(std::string from, std::string to, std::string amount, std::string fee);
            TxTransaction createCrossCahainTransaction(std::string fromAddress, std::string toAddress,
                                                     std::string crossChainAddress, std::string amount, std::string fee);

            Addresses getAddresses() {
                return _addresses;
            }
            bool reset();

        private:
            Addresses _addresses;
        };

    }
}


#endif //SPVCLIENT_TWALLET_H
