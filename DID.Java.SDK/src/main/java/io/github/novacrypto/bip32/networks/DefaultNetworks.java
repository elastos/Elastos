/*
 *  BIP32 library, a Java implementation of BIP32
 *  Copyright (C) 2017-2019 Alan Evans, NovaCrypto
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Original source: https://github.com/NovaCrypto/BIP32
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip32.networks;

import io.github.novacrypto.bip32.Network;
import io.github.novacrypto.bip32.Networks;

public enum DefaultNetworks implements Networks {
    INSTANCE(new NetworkCollection(Bitcoin.MAIN_NET, Litecoin.MAIN_NET, Bitcoin.TEST_NET, Elastos.MAIN_NET));

    private final Networks networks;

    DefaultNetworks(final Networks networks) {

        this.networks = networks;
    }

    @Override
    public Network findByPrivateVersion(final int privateVersion) {
        return networks.findByPrivateVersion(privateVersion);
    }

    @Override
    public Network findByPublicVersion(final int publicVersion) {
        return networks.findByPublicVersion(publicVersion);
    }
}