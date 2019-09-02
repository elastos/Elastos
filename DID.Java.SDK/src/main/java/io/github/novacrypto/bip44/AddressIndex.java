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

import io.github.novacrypto.bip32.derivation.Derivation;

import static io.github.novacrypto.bip32.Index.isHardened;

/**
 * The 6th and final part of a BIP44 path, create via a {@link Change}.
 * m / purpose' / coin_type' / account' / change / address_index
 * <p>
 * Addresses are numbered from index 0 in sequentially increasing manner. This number is used as child index in BIP32
 * derivation.
 * <p>
 * Public derivation is used at this level (not hardened).
 */
public final class AddressIndex {
    public static final Derivation<AddressIndex> DERIVATION = new AddressIndexDerivation();
    public static final Derivation<AddressIndex> DERIVATION_FROM_ACCOUNT = new AddressIndexFromAccountDerivation();

    private final Change change;
    private final int addressIndex;

    private final String string;

    AddressIndex(final Change change, final int addressIndex) {
        if (isHardened(addressIndex))
            throw new IllegalArgumentException();
        this.change = change;
        this.addressIndex = addressIndex;
        string = String.format("%s/%d", change, addressIndex);
    }

    public int getValue() {
        return addressIndex;
    }

    public Change getParent() {
        return change;
    }

    @Override
    public String toString() {
        return string;
    }
}