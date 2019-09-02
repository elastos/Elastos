/*
 *  BIP32derivation
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
 *  Original source: https://github.com/NovaCrypto/BIP32derivation
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip32.derivation;

public final class CkdFunctionDerive<Node> implements Derive<Node> {

    private final CkdFunction<Node> standardCkdFunction;
    private final Node rootNode;

    public CkdFunctionDerive(final CkdFunction<Node> standardCkdFunction, final Node rootNode) {
        this.standardCkdFunction = standardCkdFunction;
        this.rootNode = rootNode;
    }

    @Override
    public Node derive(final CharSequence derivationPath) {
        return derive(derivationPath, CharSequenceDerivation.INSTANCE);
    }

    @Override
    public <Path> Node derive(final Path derivationPath, final Derivation<Path> derivation) {
        return derivation.derive(rootNode, derivationPath, standardCkdFunction);
    }
}