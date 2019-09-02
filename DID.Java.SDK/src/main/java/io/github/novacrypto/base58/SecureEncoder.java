/*
 *  Base58 library, a Java implementation of Base58 encode/decode
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
 *  Original source: https://github.com/NovaCrypto/Base58
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.base58;

public interface SecureEncoder {

    /**
     * Encodes given bytes as a number in base58.
     *
     * @param bytes  bytes to encode
     * @param target where to write resulting string to
     */
    void encode(final byte[] bytes, final EncodeTarget target);


    /**
     * Encodes given bytes as a number in base58.
     *
     * @param bytes       bytes to encode
     * @param setCapacity a callback to the target to set its capacity
     * @param target      where to write resulting string to
     */
    void encode(final byte[] bytes, final EncodeTargetCapacity setCapacity, final EncodeTarget target);

    /**
     * Encodes given bytes as a number in base58.
     *
     * @param bytes  bytes to encode
     * @param target where to write resulting string to
     */
    void encode(final byte[] bytes, final EncodeTargetFromCapacity target);
}