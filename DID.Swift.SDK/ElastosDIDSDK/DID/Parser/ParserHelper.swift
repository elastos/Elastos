import Foundation
import Antlr4

class ParserHelper: NSObject {
    
    public static func parse(_ didurl: String, _ didOnly: Bool, _ listener: DIDURLBaseListener) throws {
        // TODO: 自定义BaseErrorListener
        let error: BaseErrorListener = BaseErrorListener()
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
