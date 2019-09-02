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

package io.github.novacrypto.bip32.derivation;

public interface Derivation<Path> {

    /**
     * Traverse the nodes from the root key node to find the node referenced by the path.
     *
     * @param rootKey     The root of the path
     * @param path        The path to follow
     * @param ckdFunction Allows you to follow one link
     * @param <Key>       The type of node we are visiting
     * @return The final node found at the end of the path
     */
    <Key> Key derive(final Key rootKey, final Path path, final CkdFunction<Key> ckdFunction);
}