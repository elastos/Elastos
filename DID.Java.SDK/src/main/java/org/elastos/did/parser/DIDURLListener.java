// Generated from DIDURL.g4 by ANTLR 4.7.2
package org.elastos.did.parser;
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link DIDURLParser}.
 */
public interface DIDURLListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#didurl}.
	 * @param ctx the parse tree
	 */
	void enterDidurl(DIDURLParser.DidurlContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#didurl}.
	 * @param ctx the parse tree
	 */
	void exitDidurl(DIDURLParser.DidurlContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#did}.
	 * @param ctx the parse tree
	 */
	void enterDid(DIDURLParser.DidContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#did}.
	 * @param ctx the parse tree
	 */
	void exitDid(DIDURLParser.DidContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#method}.
	 * @param ctx the parse tree
	 */
	void enterMethod(DIDURLParser.MethodContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#method}.
	 * @param ctx the parse tree
	 */
	void exitMethod(DIDURLParser.MethodContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#methodSpecificString}.
	 * @param ctx the parse tree
	 */
	void enterMethodSpecificString(DIDURLParser.MethodSpecificStringContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#methodSpecificString}.
	 * @param ctx the parse tree
	 */
	void exitMethodSpecificString(DIDURLParser.MethodSpecificStringContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#params}.
	 * @param ctx the parse tree
	 */
	void enterParams(DIDURLParser.ParamsContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#params}.
	 * @param ctx the parse tree
	 */
	void exitParams(DIDURLParser.ParamsContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#param}.
	 * @param ctx the parse tree
	 */
	void enterParam(DIDURLParser.ParamContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#param}.
	 * @param ctx the parse tree
	 */
	void exitParam(DIDURLParser.ParamContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramQName}.
	 * @param ctx the parse tree
	 */
	void enterParamQName(DIDURLParser.ParamQNameContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramQName}.
	 * @param ctx the parse tree
	 */
	void exitParamQName(DIDURLParser.ParamQNameContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramMethod}.
	 * @param ctx the parse tree
	 */
	void enterParamMethod(DIDURLParser.ParamMethodContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramMethod}.
	 * @param ctx the parse tree
	 */
	void exitParamMethod(DIDURLParser.ParamMethodContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramName}.
	 * @param ctx the parse tree
	 */
	void enterParamName(DIDURLParser.ParamNameContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramName}.
	 * @param ctx the parse tree
	 */
	void exitParamName(DIDURLParser.ParamNameContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#paramValue}.
	 * @param ctx the parse tree
	 */
	void enterParamValue(DIDURLParser.ParamValueContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#paramValue}.
	 * @param ctx the parse tree
	 */
	void exitParamValue(DIDURLParser.ParamValueContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#path}.
	 * @param ctx the parse tree
	 */
	void enterPath(DIDURLParser.PathContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#path}.
	 * @param ctx the parse tree
	 */
	void exitPath(DIDURLParser.PathContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#query}.
	 * @param ctx the parse tree
	 */
	void enterQuery(DIDURLParser.QueryContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#query}.
	 * @param ctx the parse tree
	 */
	void exitQuery(DIDURLParser.QueryContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParam}.
	 * @param ctx the parse tree
	 */
	void enterQueryParam(DIDURLParser.QueryParamContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParam}.
	 * @param ctx the parse tree
	 */
	void exitQueryParam(DIDURLParser.QueryParamContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParamName}.
	 * @param ctx the parse tree
	 */
	void enterQueryParamName(DIDURLParser.QueryParamNameContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParamName}.
	 * @param ctx the parse tree
	 */
	void exitQueryParamName(DIDURLParser.QueryParamNameContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#queryParamValue}.
	 * @param ctx the parse tree
	 */
	void enterQueryParamValue(DIDURLParser.QueryParamValueContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#queryParamValue}.
	 * @param ctx the parse tree
	 */
	void exitQueryParamValue(DIDURLParser.QueryParamValueContext ctx);
	/**
	 * Enter a parse tree produced by {@link DIDURLParser#frag}.
	 * @param ctx the parse tree
	 */
	void enterFrag(DIDURLParser.FragContext ctx);
	/**
	 * Exit a parse tree produced by {@link DIDURLParser#frag}.
	 * @param ctx the parse tree
	 */
	void exitFrag(DIDURLParser.FragContext ctx);
}