// Generated from /Users/liaihong/Desktop/AN/antlr4/runtime/Swift/Tests/Antlr4Tests/VisitorCalc.g4 by ANTLR 4.7.1
import Antlr4

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link VisitorCalcParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
open class VisitorCalcVisitor<T>: ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link VisitorCalcParser#s}.
	- Parameters:
	  - ctx: the parse tree
	- returns: the visitor result
	 */
	open func visitS(_ ctx: VisitorCalcParser.SContext) -> T {
	 	fatalError(#function + " must be overridden")
	}

	/**
	 * Visit a parse tree produced by the {@code multiply}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	- Parameters:
	  - ctx: the parse tree
	- returns: the visitor result
	 */
	open func visitMultiply(_ ctx: VisitorCalcParser.MultiplyContext) -> T {
	 	fatalError(#function + " must be overridden")
	}

	/**
	 * Visit a parse tree produced by the {@code number}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	- Parameters:
	  - ctx: the parse tree
	- returns: the visitor result
	 */
	open func visitNumber(_ ctx: VisitorCalcParser.NumberContext) -> T {
	 	fatalError(#function + " must be overridden")
	}

	/**
	 * Visit a parse tree produced by the {@code add}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	- Parameters:
	  - ctx: the parse tree
	- returns: the visitor result
	 */
	open func visitAdd(_ ctx: VisitorCalcParser.AddContext) -> T {
	 	fatalError(#function + " must be overridden")
	}

}
