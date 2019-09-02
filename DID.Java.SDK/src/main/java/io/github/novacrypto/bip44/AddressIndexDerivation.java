/*
 *  BIP44
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
 *  Original source: https://github.com/NovaCrypto/BIP44
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip44;

import io.github.novacrypto.bip32.derivation.CkdFunction;
import io.github.novacrypto.bip32.derivation.Derivation;

import static io.github.novacrypto.bip32.Index.hard;

final class AddressIndexDerivation implements Derivation<AddressIndex> {
    @Override
    public <Node> Node derive(final Node root, final AddressIndex addressIndex, final CkdFunction<Node> ckdFunction) {
        final Change change = addressIndex.getParent();
        final Account account = change.getParent();
        final CoinType coinType = account.getParent();
        final Purpose purpose = coinType.getParent();
        return ckdFunction.deriveChildKey(
                ckdFunction.deriveChildKey(
                        ckdFunction.deriveChildKey(
                                ckdFunction.deriveChildKey(
                                        ckdFunction.deriveChildKey(root,
                                                hard(purpose.getValue())),
                                        hard(coinType.getValue())),
                                hard(account.getValue())),
                        change.getValue()),
                addressIndex.getValue());
    }
}