/*
 * Copyright (c) 2019 Elastos Foundation
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

/*
 * did:elastos:method-specific-string[;params][/path][?query][#fragment]
 */
grammar DIDURL;

didurl
	: did (';' params)? ('/' path)? ('?' query)? ('#' frag)? SPACE?
	;

did
	: 'did' ':' method ':' methodSpecificString
	;

method
	: STRING
	;

methodSpecificString
	: STRING
	;

params
	: param (';' param)*
	;

param
	: paramQName ('=' paramValue)?
	;

paramQName
	: (paramMethod ':')? paramName
	;

paramMethod
	: STRING
	;

paramName
	: STRING
	;

paramValue
	: STRING
	;

path
	: STRING ('/' STRING)*
	;

query
	: queryParam ('&' queryParam)*
	;

queryParam
	: queryParamName ('=' queryParamValue)?
	;

queryParamName
	: STRING
	;

queryParamValue
	: STRING
	;

frag
	: STRING
	;

STRING
	: ([a-zA-Z~0-9] | HEX) ([a-zA-Z0-9.-] | HEX)*
	;

HEX
	: ('%' [a-fA-F0-9] [a-fA-F0-9])+
	;

SPACE
	: [ \t\n\r]+
	;
