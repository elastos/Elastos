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
 * Represents the first part of a BIP44 path. To create, use the static factory method {@link BIP44#m()}.
 * m / purpose' / coin_type' / account' / change / address_index
 */
public final class M {

    private final Purpose PURPOSE_44 = new Purpose(this, 44);
    private final Purpose PURPOSE_49 = new Purpose(this, 49);

    M() {
    }

    /**
     * Create a {@link Purpose}.
     * For 44 and 49 this function is guaranteed to return the same instance.
     *
     * @param purpose The purpose number.
     * @return A purpose object.
     */
    public Purpose purpose(final int purpose) {
        switch (purpose) {
            case 44:
                return PURPOSE_44;
            case 49:
                return PURPOSE_49;
            default:
                return new Purpose(this, purpose);
        }
    }

    public Purpose purpose44() {
        return PURPOSE_44;
    }

    public Purpose purpose49() {
        return PURPOSE_49;
    }

    @Override
    public String toString() {
        return "m";
    }
}