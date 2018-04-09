//
// Created by jzh on 18-4-9.
//

#include <iostream>

#include "Util.h"

using namespace std;

namespace Elastos {
    namespace APP {

        bool Util::selectAddress(boost::shared_ptr<TWallet> twallet, std::string &address) {
            Addresses addr = twallet->getAddresses();
            if (addr.size() == 0) {
                cout << "fail to load wallet addresses !" << endl;
                return false;
            }

            if (addr.size() == 1) {
                address = addr[0].Address;
                return true;
            }

            string spaceLine = string(" ", addr[0].Address.length() - 7);
            string brokenLine = string("-", addr[0].Address.length() - 7);
            cout << "INDEX ADDRESS" << spaceLine << "BALANCE" <<endl;
            cout << "----- -------" << brokenLine << "--------------------" << endl;
            for (int i=0; i<addr.size(); i++) {
                //TODO twallet.getAddressUTXOs and show
            }

            return true;
        }
    }
}
