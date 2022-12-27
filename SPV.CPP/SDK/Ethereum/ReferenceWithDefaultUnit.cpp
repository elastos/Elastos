/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Common/ErrorChecker.h>
#include "ReferenceWithDefaultUnit.h"

namespace Elastos {
	namespace ElaWallet {

		EthereumAmount::Unit ReferenceWithDefaultUnit::getDefaultUnit() const {
			return _defaultUnit;
		}

		void ReferenceWithDefaultUnit::setDefaultUnit(EthereumAmount::Unit unit) {
			validUnitOrException(unit);
			_defaultUnit = unit;
		}

		ReferenceWithDefaultUnit::ReferenceWithDefaultUnit(EthereumEWM *ewm,
														   void *identifier,
														   EthereumAmount::Unit unit) :
			Reference(ewm, identifier) {
			_defaultUnit = unit;
			_defaultUnitUsesToken = EthereumAmount::isTokenUnit(unit);
		}

		ReferenceWithDefaultUnit::~ReferenceWithDefaultUnit() {
		}

		bool ReferenceWithDefaultUnit::validUnit(EthereumAmount::Unit unit) const {
			return (!_defaultUnitUsesToken
					? (unit == EthereumAmount::Unit::ETHER_WEI || unit == EthereumAmount::Unit::ETHER_GWEI ||
					   unit == EthereumAmount::Unit::ETHER_ETHER)
					: (unit == EthereumAmount::Unit::TOKEN_DECIMAL || unit == EthereumAmount::Unit::TOKEN_INTEGER));
		}

		void ReferenceWithDefaultUnit::validUnitOrException(EthereumAmount::Unit unit) const {
			ErrorChecker::CheckParam(!validUnit(unit), Error::InvalidUnitType,
									 "Invalid Unit for instance type: " + EthereumAmount::toString(unit));
		}


	}
}