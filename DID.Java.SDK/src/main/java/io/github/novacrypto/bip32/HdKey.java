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

import static io.github.novacrypto.bip32.BigIntegerUtils.parse256;
import static io.github.novacrypto.bip32.Secp256r1SC.pointSerP_gMultiply;
import static io.github.novacrypto.hashing.Hash160.hash160;

final class HdKey {

    private final boolean neutered;
    private final Network network;
    private final byte[] chainCode;
    private final byte[] key;
    private final Serializer serializer;
    private final int parentFingerprint;
    private final int childNumber;
    private final int depth;

    private HdKey(final Builder builder) {
        neutered = builder.neutered;
        network = builder.network;
        key = builder.key;
        parentFingerprint = builder.parentFingerprint;
        childNumber = builder.childNumber;
        chainCode = builder.chainCode;
        depth = builder.depth;
        serializer = new Serializer.Builder()
                .network(builder.network)
                .neutered(builder.neutered)
                .depth(builder.depth)
                .childNumber(builder.childNumber)
                .fingerprint(builder.parentFingerprint)
                .build();
    }

    byte[] serialize() {
        return serializer.serialize(key, chainCode);
    }

    byte[] getPoint() {
        return pointSerP_gMultiply(parse256(key));
    }

    byte[] getKey() {
        return key;
    }

    int getParentFingerprint() {
        return parentFingerprint;
    }

    int calculateFingerPrint() {
        final byte[] point = getPublicBuffer();
        final byte[] o = hash160(point);
        return ((o[0] & 0xFF) << 24) |
                ((o[1] & 0xFF) << 16) |
                ((o[2] & 0xFF) << 8) |
                (o[3] & 0xFF);
    }

    private byte[] getPublicBuffer() {
        return neutered ? key : getPoint();
    }

    int depth() {
        return depth;
    }

    Network getNetwork() {
        return network;
    }

    byte[] getChainCode() {
        return chainCode;
    }

    int getChildNumber() {
        return childNumber;
    }

    Builder toBuilder() {
        return new Builder()
                .neutered(neutered)
                .chainCode(chainCode)
                .key(key)
                .depth(depth)
                .childNumber(childNumber)
                .parentFingerprint(parentFingerprint);
    }

    static class Builder {

        private Network network;
        private boolean neutered;
        private byte[] chainCode;
        private byte[] key;
        private int depth;
        private int childNumber;
        private int parentFingerprint;

        Builder network(final Network network) {
            this.network = network;
            return this;
        }

        Builder neutered(final boolean neutered) {
            this.neutered = neutered;
            return this;
        }

        Builder key(final byte[] key) {
            this.key = key;
            return this;
        }

        Builder chainCode(final byte[] chainCode) {
            this.chainCode = chainCode;
            return this;
        }

        Builder depth(final int depth) {
            this.depth = depth;
            return this;
        }

        Builder childNumber(final int childNumber) {
            this.childNumber = childNumber;
            return this;
        }

        Builder parentFingerprint(final int parentFingerprint) {
            this.parentFingerprint = parentFingerprint;
            return this;
        }

        HdKey build() {
            return new HdKey(this);
        }
    }
}