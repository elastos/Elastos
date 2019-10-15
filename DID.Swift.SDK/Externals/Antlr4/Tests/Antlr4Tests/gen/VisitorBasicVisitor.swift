// Generated from /Users/liaihong/Desktop/AN/antlr4/runtime/Swift/Tests/Antlr4Tests/VisitorBasic.g4 by ANTLR 4.7.1
import Antlr4

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link VisitorBasicParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
open class VisitorBasicVisitor<T>: ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link VisitorBasicParser#s}.
	- Parameters:
	  - ctx: the parse tree
	- returns: the visitor result
	 */
	open func visitS(_ ctx: VisitorBasicParser.SContext) -> T {
	 	fatalError(#function + " must be overridden")
	}

}
