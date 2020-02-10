import Foundation
import Antlr4

class DIDAntlr4ErrorListener: BaseErrorListener {
    override func syntaxError<T>(_ recognizer: Recognizer<T>,
                                 _ offendingSymbol: AnyObject?,
                                 _ line: Int,
                                 _ charPositionInLine: Int,
                                 _ msg: String,
                                 _ e: AnyObject?) where T : ATNSimulator {
        print("At position \(charPositionInLine) : \(msg), \(String(describing: e))")
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
        
        let parser = try! DIDURLParser(CommonTokenStream(lexer))
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
