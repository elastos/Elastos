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

/**
 * The 5th part of a BIP44 path, create via a {@link Account}.
 * m / purpose' / coin_type' / account' / change / address_index
 * <p>
 * Constant 0 is used for external chain and constant 1 for internal chain (also known as change addresses).
 * External chain is used for addresses that are meant to be visible outside of the wallet (e.g. for receiving
 * payments).
 * Internal chain is used for addresses which are not meant to be visible outside of the wallet and is used for return
 * transaction change.
 * <p>
 * Public derivation is used at this level (not hardened).
 */
public final class Change {
    private final Account account;
    private final int change;
    private final String string;

    Change(final Account account, final int change) {
        this.account = account;
        this.change = change;
        string = String.format("%s/%d", account, change);
    }

    public int getValue() {
        return change;
    }

    public Account getParent() {
        return account;
    }

    @Override
    public String toString() {
        return string;
    }

    /**
     * Create a {@link AddressIndex} for this purpose, coin type, account and change.
     *
     * @param addressIndex The index of the child
     * @return A coin type instance for this purpose, coin type, account and change.
     */
    public AddressIndex address(final int addressIndex) {
        return new AddressIndex(this, addressIndex);
    }
}