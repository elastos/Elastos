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
 * The 4th level of a BIP44 path, create via a {@link Change}.
 * m / purpose' / coin_type' / account' / change / address_index
 * <p>
 * This level splits the key space into independent user identities, so the wallet never mixes the coins across
 * different accounts.
 * <p>
 * Users can use these accounts to organize the funds in the same fashion as bank accounts; for donation purposes
 * (where all addresses are considered public), for saving purposes, for common expenses etc.
 * <p>
 * Accounts are numbered from index 0 in sequentially increasing manner. This number is used as child index in BIP32
 * derivation.
 * <p>
 * Hardened derivation is used at this level.
 */
public final class Account {
    public static final Derivation<Account> DERIVATION = new AccountDerivation();

    private final CoinType coinType;
    private final int account;
    private final String string;

    Account(final CoinType coinType, final int account) {
        if (isHardened(account))
            throw new IllegalArgumentException();
        this.coinType = coinType;
        this.account = account;
        string = String.format("%s/%d'", coinType, account);
    }

    public int getValue() {
        return account;
    }

    public CoinType getParent() {
        return coinType;
    }

    @Override
    public String toString() {
        return string;
    }

    /**
     * Create a {@link Change} for this purpose, coin type and account.
     * <p>
     * Constant 0 is used for external chain.
     * External chain is used for addresses that are meant to be visible outside of the wallet (e.g. for receiving
     * payments).
     *
     * @return A {@link Change} = 0 instance for this purpose, coin type and account
     */
    public Change external() {
        return new Change(this, 0);
    }

    /**
     * Create a {@link Change} for this purpose, coin type and account.
     * <p>
     * Constant 1 is used for internal chain (also known as change addresses).
     * Internal chain is used for addresses which are not meant to be visible outside of the wallet and is used for
     * return transaction change.
     *
     * @return A {@link Change} = 1 instance for this purpose, coin type and account
     */
    public Change internal() {
        return new Change(this, 1);
    }

}