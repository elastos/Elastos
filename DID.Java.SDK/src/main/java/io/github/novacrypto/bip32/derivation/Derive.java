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

public interface Derive<Key> {

    /**
     * Derive from a string path such as m/44'/0'/0'/0/1
     *
     * @param derivationPath Path
     * @return Key at the path
     */
    Key derive(final CharSequence derivationPath);

    /**
     * Derive from a generic path using the {@link Derivation} supplied to extract the child indexes
     *
     * @param derivationPath Path
     * @param derivation     The class that extracts the path elements
     * @param <Path>         The generic type of the path
     * @return Key at the path
     */
    <Path> Key derive(final Path derivationPath, final Derivation<Path> derivation);
}