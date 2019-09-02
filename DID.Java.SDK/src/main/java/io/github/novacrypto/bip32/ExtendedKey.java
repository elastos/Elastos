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

package io.github.novacrypto.bip32;

public interface ExtendedKey {

    /**
     * The network of this extended key
     *
     * @return The network of this extended key
     */
    // Network network();

    /**
     * 1 byte: 0 for master nodes, 1 for level-1 derived keys, etc.
     *
     * @return the depth of this key node
     */
    int depth();

    /**
     * 4 bytes: child number. e.g. 3 for m/3, hard(7) for m/7'
     * 0 if master key
     *
     * @return the child number
     */
    int childNumber();

    /**
     * Serialized Base58 String of this extended key
     *
     * @return the Base58 String representing this key
     */
    String extendedBase58();

    /**
     * Serialized data of this extended key
     *
     * @return the byte array representing this key
     */
    byte[] extendedKeyByteArray();

    /**
     * Coerce this key on to another network.
     *
     * @param otherNetwork Network to put key on.
     * @return A new extended key, or this instance if key already on the other Network.
     */
    ExtendedKey toNetwork(final Network otherNetwork);
}