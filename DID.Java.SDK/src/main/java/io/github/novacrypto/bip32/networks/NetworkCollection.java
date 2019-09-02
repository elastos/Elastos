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

import java.util.Arrays;
import java.util.List;

public final class NetworkCollection implements Networks {
    private final List<? extends Network> networks;

    public NetworkCollection(final Network... networks) {
        this.networks = Arrays.asList(networks);
    }

    @Override
    public Network findByPrivateVersion(final int privateVersion) {
        for (final Network network : networks) {
            if (network.getPrivateVersion() == privateVersion)
                return network;
        }
        throw new UnknownNetworkException(String.format("Can't find network that matches private version 0x%x", privateVersion));
    }

    @Override
    public Network findByPublicVersion(final int publicVersion) {
        for (final Network network : networks) {
            if (network.getPublicVersion() == publicVersion)
                return network;
        }
        throw new UnknownNetworkException(String.format("Can't find network that matches public version 0x%x", publicVersion));
    }
}