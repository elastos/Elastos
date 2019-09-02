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

import static io.github.novacrypto.bip32.Index.isHardened;

/**
 * Represents the 1st level of a BIP44 path. To create, start with the static factory method {@link BIP44#m()}.
 * m / purpose' / coin_type' / account' / change / address_index
 * <p>
 * Purpose is a constant set to 44' (or 0x8000002C) following the BIP43 recommendation. It indicates that the subtree of
 * this node is used according to this specification.
 * <p>
 * Purpose 49 is used for segwit as per BIP0049.
 * <p>
 * Purpose 0 is reserved by BIP0032.
 * <p>
 * Hardened derivation is used at this level.
 */
public final class Purpose {
    private final M m;
    private final int purpose;
    private final String toString;

    Purpose(final M m, final int purpose) {
        this.m = m;
        if (purpose == 0 || isHardened(purpose))
            throw new IllegalArgumentException();
        this.purpose = purpose;
        toString = String.format("%s/%d'", m, purpose);
    }

    public int getValue() {
        return purpose;
    }

    @Override
    public String toString() {
        return toString;
    }

    /**
     * Create a {@link CoinType} for this purpose.
     *
     * @param coinType The coin type
     * @return A coin type instance for this purpose
     */
    public CoinType coinType(final int coinType) {
        return new CoinType(this, coinType);
    }
}