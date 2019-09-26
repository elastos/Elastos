import Foundation
import Antlr4

class ParserHelper: NSObject {
    
    public static func parase(_ didurl: String, _ didOnly: Bool, _ listener: DIDURLBaseListener) {
        //        let input: CharStream = CharStream
        //        let lexer: DIDURLLexer = DIDURLLexer(input)
        
    }
    
    //        BaseErrorListener errorListener = new BaseErrorListener() {
    //            @Override
    //            public void syntaxError(Recognizer<?, ?> recognizer,
    //            Object offendingSymbol, int line, int charPositionInLine,
    //            String msg, RecognitionException e) {
    //                throw new IllegalArgumentException(
    //                    "At position " + charPositionInLine + ": " + msg, e);
    //            }
    //        };
    //
    //        CharStream input = CharStreams.fromString(didurl);
    //        DIDURLLexer lexer = new DIDURLLexer(input);
    //        lexer.removeErrorListeners();
    //        lexer.addErrorListener(errorListener);
    //
    //        CommonTokenStream tokens = new CommonTokenStream(lexer);
    //        DIDURLParser parser = new DIDURLParser(tokens);
    //        parser.removeErrorListeners();
    //        parser.addErrorListener(errorListener);
    //
    //        ParseTree tree;
    //
    //        if (didOnly)
    //        tree = parser.did();
    //        else
    //        tree = parser.didurl();
    //
    //        ParseTreeWalker walker = new ParseTreeWalker();
    //        walker.walk(listener, tree);
    
}
