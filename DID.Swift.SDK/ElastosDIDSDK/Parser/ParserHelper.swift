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

import Foundation
import Antlr4

class DIDAntlr4ErrorListener: BaseErrorListener {
    override func syntaxError<T>(_ recognizer: Recognizer<T>,
                                 _ offendingSymbol: AnyObject?,
                                 _ line: Int,
                                 _ charPositionInLine: Int,
                                 _ msg: String,
                                 _ e: AnyObject?) throws where T : ATNSimulator {
        let msg = "At position \(charPositionInLine) : \(msg), \(String(describing: e))"
        throw DIDError.illegalArgument(msg)
    }
}

class ParserHelper: NSObject {
    public static func parse(_ didurl: String,
                             _ didOnly: Bool,
                             _ listener: DIDURLBaseListener) throws {

        let errorListener = DIDAntlr4ErrorListener()
        let lexer = DIDURLLexer(ANTLRInputStream(didurl))
        lexer.removeErrorListeners()
        lexer.addErrorListener(errorListener)
        
        let parser = try DIDURLParser(CommonTokenStream(lexer))
        parser.removeErrorListeners()
        parser.addErrorListener(errorListener)
        
        var tree: ParseTree
        if didOnly {
            tree = try parser.did()
        } else {
            tree = try parser.didurl()
        }
        
        let walker: ParseTreeWalker = ParseTreeWalker()
        try walker.walk(listener, tree)
    }
}
