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

package io.github.novacrypto.bip32;

import static io.github.novacrypto.bip32.Checksum.checksum;

final class Serializer {

    private final Network network;
    private final boolean neutered;
    private final int depth;
    private final int childNumber;
    private final int fingerprint;

    private Serializer(final Builder builder) {
        network = builder.network;
        neutered = builder.neutered;
        depth = builder.depth;
        childNumber = builder.childNumber;
        fingerprint = builder.fingerprint;
    }

    byte[] serialize(final byte[] key, final byte[] chainCode) {
        if (key == null)
            throw new IllegalArgumentException("Key is null");
        if (chainCode == null)
            throw new IllegalArgumentException("Chain code is null");
        if (chainCode.length != 32)
            throw new IllegalArgumentException("Chain code must be 32 bytes");
        if (neutered) {
            if (key.length != 33)
                throw new IllegalArgumentException("Key must be 33 bytes for neutered serialization");
        } else {
            if (key.length != 32)
                throw new IllegalArgumentException("Key must be 32 bytes for non neutered serialization");
        }

        final byte[] privateKey = new byte[82];
        final ByteArrayWriter writer = new ByteArrayWriter(privateKey);
        writer.concatSer32(getVersion());
        writer.concat((byte) depth);
        writer.concatSer32(fingerprint);
        writer.concatSer32(childNumber);
        writer.concat(chainCode);
        if (!neutered) {
            writer.concat((byte) 0);
            writer.concat(key);
        } else {
            writer.concat(key);
        }
        writer.concat(checksum(privateKey), 4);
        return privateKey;
    }

    private int getVersion() {
        // return neutered ? network.getPublicVersion() : network.getPrivateVersion();
    	return 0;
    }

    static class Builder {

        private Network network;
        private boolean neutered;
        private int depth;
        private int childNumber;
        private int fingerprint;

        Builder network(final Network network) {
            this.network = network;
            return this;
        }

        Builder neutered(final boolean neutered) {
            this.neutered = neutered;
            return this;
        }

        Builder depth(final int depth) {
            if (depth < 0 || depth > 255)
                throw new IllegalArgumentException("Depth must be [0..255]");
            this.depth = depth;
            return this;
        }

        Builder childNumber(final int childNumber) {
            this.childNumber = childNumber;
            return this;
        }

        Builder fingerprint(final int fingerprint) {
            this.fingerprint = fingerprint;
            return this;
        }

        Serializer build() {
            return new Serializer(this);
        }
    }
}