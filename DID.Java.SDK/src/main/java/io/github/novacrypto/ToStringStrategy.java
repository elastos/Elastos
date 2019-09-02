/*
 *  SecureString library, Obfuscated/clearable in memory string management
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
 *  Original source: https://github.com/NovaCrypto/SecureString
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto;

enum ToStringStrategy {

    RESTRICT {
        @Override
        public String toString(final CharSequence charSequence) {
            throw new UnsupportedOperationException();
        }
    },

    ALLOW {
        @Override
        public String toString(final CharSequence charSequence) {
            final int length = charSequence.length();
            final char[] chars = new char[length];
            for (int i = 0; i < length; i++) {
                chars[i] = charSequence.charAt(i);
            }
            return new String(chars);
        }
    };

    abstract String toString(final CharSequence charSequence);
}