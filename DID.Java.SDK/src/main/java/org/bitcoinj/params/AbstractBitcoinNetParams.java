/*
 * Copyright 2013 Google Inc.
 * Copyright 2015 Andreas Schildbach
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.bitcoinj.params;

import org.bitcoinj.core.NetworkParameters;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Parameters for Bitcoin-like networks.
 */
public abstract class AbstractBitcoinNetParams extends NetworkParameters {

    /**
     * Scheme part for Bitcoin URIs.
     */
    public static final String BITCOIN_SCHEME = "bitcoin";
    public static final int REWARD_HALVING_INTERVAL = 210000;

    private static final Logger log = LoggerFactory.getLogger(AbstractBitcoinNetParams.class);

    public AbstractBitcoinNetParams() {
        super();
    }

    /**
     * Checks if we are at a reward halving point.
     * @param height The height of the previous stored block
     * @return If this is a reward halving point
     */
    public final boolean isRewardHalvingPoint(final int height) {
        return ((height + 1) % REWARD_HALVING_INTERVAL) == 0;
    }

    /**
     * Checks if we are at a difficulty transition point.
     * @param height The height of the previous stored block
     * @return If this is a difficulty transition point
     */
    public final boolean isDifficultyTransitionPoint(final int height) {
        return ((height + 1) % this.getInterval()) == 0;
    }

    @Override
    public int getProtocolVersionNum(final ProtocolVersion version) {
        return version.getBitcoinProtocolVersion();
    }

    @Override
    public String getUriScheme() {
        return BITCOIN_SCHEME;
    }

    @Override
    public boolean hasMaxMoney() {
        return true;
    }
}
