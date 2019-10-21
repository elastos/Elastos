import Foundation
import Antlr4


class DIDAntlr4ErrorListener: BaseErrorListener {
    
    override func syntaxError<T>(_ recognizer: Recognizer<T>, _ offendingSymbol: AnyObject?, _ line: Int, _ charPositionInLine: Int, _ msg: String, _ e: AnyObject?) where T : ATNSimulator {
        print("At position \(charPositionInLine) : \(msg), \(String(describing: e))")
    }
}

class ParserHelper: NSObject {
    
    public static func parase(_ didurl: String, _ didOnly: Bool, _ listener: DIDURLBaseListener) throws {
        let error: DIDAntlr4ErrorListener = DIDAntlr4ErrorListener()
        let input: ANTLRInputStream =  ANTLRInputStream(didurl)
        let lexer: DIDURLLexer = DIDURLLexer(input)
        lexer.removeErrorListeners()
        lexer.addErrorListener(error)
        
        let tokens: CommonTokenStream = CommonTokenStream(lexer)
        let parser: DIDURLParser = try! DIDURLParser(tokens)
        parser.removeErrorListeners()
        parser.addErrorListener(error)
        
        var tree: ParseTree
        if didOnly {
            tree = try parser.did()
        }else {
            tree = try parser.didurl()
        }
        
        let walker: ParseTreeWalker = ParseTreeWalker()
        try walker.walk(listener, tree)
    }
}
