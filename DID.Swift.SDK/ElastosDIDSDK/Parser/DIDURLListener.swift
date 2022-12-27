// Generated from DIDURL.g4 by ANTLR 4.7.2
import Antlr4

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link DIDURLParser}.
 */
public protocol DIDURLListener: ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#didurl}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterDidurl(_ ctx: DIDURLParser.DidurlContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#didurl}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitDidurl(_ ctx: DIDURLParser.DidurlContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#did}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterDid(_ ctx: DIDURLParser.DidContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#did}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitDid(_ ctx: DIDURLParser.DidContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#method}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterMethod(_ ctx: DIDURLParser.MethodContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#method}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitMethod(_ ctx: DIDURLParser.MethodContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#methodSpecificString}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#methodSpecificString}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#params}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParams(_ ctx: DIDURLParser.ParamsContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#params}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParams(_ ctx: DIDURLParser.ParamsContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#param}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParam(_ ctx: DIDURLParser.ParamContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#param}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParam(_ ctx: DIDURLParser.ParamContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramQName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParamQName(_ ctx: DIDURLParser.ParamQNameContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramQName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParamQName(_ ctx: DIDURLParser.ParamQNameContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramMethod}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParamMethod(_ ctx: DIDURLParser.ParamMethodContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramMethod}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParamMethod(_ ctx: DIDURLParser.ParamMethodContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParamName(_ ctx: DIDURLParser.ParamNameContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParamName(_ ctx: DIDURLParser.ParamNameContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramValue}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterParamValue(_ ctx: DIDURLParser.ParamValueContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramValue}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitParamValue(_ ctx: DIDURLParser.ParamValueContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#path}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterPath(_ ctx: DIDURLParser.PathContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#path}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitPath(_ ctx: DIDURLParser.PathContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#query}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterQuery(_ ctx: DIDURLParser.QueryContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#query}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitQuery(_ ctx: DIDURLParser.QueryContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParam}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterQueryParam(_ ctx: DIDURLParser.QueryParamContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParam}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitQueryParam(_ ctx: DIDURLParser.QueryParamContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParamName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParamName}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitQueryParamName(_ ctx: DIDURLParser.QueryParamNameContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParamValue}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParamValue}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitQueryParamValue(_ ctx: DIDURLParser.QueryParamValueContext)
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#frag}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func enterFrag(_ ctx: DIDURLParser.FragContext)
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#frag}.
	 - Parameters:
	   - ctx: the parse tree
	 */
	func exitFrag(_ ctx: DIDURLParser.FragContext)
}