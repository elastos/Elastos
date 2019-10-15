// Generated from /Users/liaihong/Desktop/AN/antlr4/runtime/Swift/Tests/Antlr4Tests/VisitorCalc.g4 by ANTLR 4.7.1
import Antlr4

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link VisitorCalcParser}.
 */
public protocol VisitorCalcListener: ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link VisitorCalcParser#s}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterS(_ ctx: VisitorCalcParser.SContext)
	/**
	 * Exit a parse tree produced by {@link VisitorCalcParser#s}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitS(_ ctx: VisitorCalcParser.SContext)
	/**
	 * Enter a parse tree produced by the {@code multiply}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterMultiply(_ ctx: VisitorCalcParser.MultiplyContext)
	/**
	 * Exit a parse tree produced by the {@code multiply}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitMultiply(_ ctx: VisitorCalcParser.MultiplyContext)
	/**
	 * Enter a parse tree produced by the {@code number}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterNumber(_ ctx: VisitorCalcParser.NumberContext)
	/**
	 * Exit a parse tree produced by the {@code number}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitNumber(_ ctx: VisitorCalcParser.NumberContext)
	/**
	 * Enter a parse tree produced by the {@code add}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterAdd(_ ctx: VisitorCalcParser.AddContext)
	/**
	 * Exit a parse tree produced by the {@code add}
	 * labeled alternative in {@link VisitorCalcParser#expr}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitAdd(_ ctx: VisitorCalcParser.AddContext)
}
