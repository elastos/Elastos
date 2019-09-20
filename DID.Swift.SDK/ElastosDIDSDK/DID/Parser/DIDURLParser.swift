// Generated from DIDURL.g4 by ANTLR 4.7.2
import Antlr4

open class DIDURLParser: Parser {

	internal static var _decisionToDFA: [DFA] = {
          var decisionToDFA = [DFA]()
          let length = DIDURLParser._ATN.getNumberOfDecisions()
          for i in 0..<length {
            decisionToDFA.append(DFA(DIDURLParser._ATN.getDecisionState(i)!, i))
           }
           return decisionToDFA
     }()

	internal static let _sharedContextCache = PredictionContextCache()

	public
	enum Tokens: Int {
		case EOF = -1, T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, 
                 T__6 = 7, T__7 = 8, STRING = 9, HEX = 10, SPACE = 11
	}

	public
	static let RULE_didurl = 0, RULE_did = 1, RULE_method = 2, RULE_methodSpecificString = 3, 
            RULE_params = 4, RULE_param = 5, RULE_paramQName = 6, RULE_paramMethod = 7, 
            RULE_paramName = 8, RULE_paramValue = 9, RULE_path = 10, RULE_query = 11, 
            RULE_queryParam = 12, RULE_queryParamName = 13, RULE_queryParamValue = 14, 
            RULE_frag = 15

	public
	static let ruleNames: [String] = [
		"didurl", "did", "method", "methodSpecificString", "params", "param", 
		"paramQName", "paramMethod", "paramName", "paramValue", "path", "query", 
		"queryParam", "queryParamName", "queryParamValue", "frag"
	]

	private static let _LITERAL_NAMES: [String?] = [
		nil, "';'", "'/'", "'?'", "'#'", "'did'", "':'", "'='", "'&'"
	]
	private static let _SYMBOLIC_NAMES: [String?] = [
		nil, nil, nil, nil, nil, nil, nil, nil, nil, "STRING", "HEX", "SPACE"
	]
	public
	static let VOCABULARY = Vocabulary(_LITERAL_NAMES, _SYMBOLIC_NAMES)

	override open
	func getGrammarFileName() -> String { return "DIDURL.g4" }

	override open
	func getRuleNames() -> [String] { return DIDURLParser.ruleNames }

	override open
	func getSerializedATN() -> String { return DIDURLParser._serializedATN }

	override open
	func getATN() -> ATN { return DIDURLParser._ATN }


	override open
	func getVocabulary() -> Vocabulary {
	    return DIDURLParser.VOCABULARY
	}

	override public
	init(_ input:TokenStream) throws {
	    RuntimeMetaData.checkVersion("4.7.2", RuntimeMetaData.VERSION)
		try super.init(input)
		_interp = ParserATNSimulator(self,DIDURLParser._ATN,DIDURLParser._decisionToDFA, DIDURLParser._sharedContextCache)
	}


	public class DidurlContext: ParserRuleContext {
			open
			func did() -> DidContext? {
				return getRuleContext(DidContext.self, 0)
			}
			open
			func params() -> ParamsContext? {
				return getRuleContext(ParamsContext.self, 0)
			}
			open
			func path() -> PathContext? {
				return getRuleContext(PathContext.self, 0)
			}
			open
			func query() -> QueryContext? {
				return getRuleContext(QueryContext.self, 0)
			}
			open
			func frag() -> FragContext? {
				return getRuleContext(FragContext.self, 0)
			}
			open
			func SPACE() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.SPACE.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_didurl
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterDidurl(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitDidurl(self)
			}
		}
	}
	@discardableResult
	 open func didurl() throws -> DidurlContext {
		var _localctx: DidurlContext = DidurlContext(_ctx, getState())
		try enterRule(_localctx, 0, DIDURLParser.RULE_didurl)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(32)
		 	try did()
		 	setState(35)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__0.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(33)
		 		try match(DIDURLParser.Tokens.T__0.rawValue)
		 		setState(34)
		 		try params()

		 	}

		 	setState(39)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__1.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(37)
		 		try match(DIDURLParser.Tokens.T__1.rawValue)
		 		setState(38)
		 		try path()

		 	}

		 	setState(43)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__2.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(41)
		 		try match(DIDURLParser.Tokens.T__2.rawValue)
		 		setState(42)
		 		try query()

		 	}

		 	setState(47)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__3.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(45)
		 		try match(DIDURLParser.Tokens.T__3.rawValue)
		 		setState(46)
		 		try frag()

		 	}

		 	setState(50)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.SPACE.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(49)
		 		try match(DIDURLParser.Tokens.SPACE.rawValue)

		 	}


		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class DidContext: ParserRuleContext {
			open
			func method() -> MethodContext? {
				return getRuleContext(MethodContext.self, 0)
			}
			open
			func methodSpecificString() -> MethodSpecificStringContext? {
				return getRuleContext(MethodSpecificStringContext.self, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_did
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterDid(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitDid(self)
			}
		}
	}
	@discardableResult
	 open func did() throws -> DidContext {
		var _localctx: DidContext = DidContext(_ctx, getState())
		try enterRule(_localctx, 2, DIDURLParser.RULE_did)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(52)
		 	try match(DIDURLParser.Tokens.T__4.rawValue)
		 	setState(53)
		 	try match(DIDURLParser.Tokens.T__5.rawValue)
		 	setState(54)
		 	try method()
		 	setState(55)
		 	try match(DIDURLParser.Tokens.T__5.rawValue)
		 	setState(56)
		 	try methodSpecificString()

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class MethodContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_method
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterMethod(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitMethod(self)
			}
		}
	}
	@discardableResult
	 open func method() throws -> MethodContext {
		var _localctx: MethodContext = MethodContext(_ctx, getState())
		try enterRule(_localctx, 4, DIDURLParser.RULE_method)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(58)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class MethodSpecificStringContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_methodSpecificString
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterMethodSpecificString(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitMethodSpecificString(self)
			}
		}
	}
	@discardableResult
	 open func methodSpecificString() throws -> MethodSpecificStringContext {
		var _localctx: MethodSpecificStringContext = MethodSpecificStringContext(_ctx, getState())
		try enterRule(_localctx, 6, DIDURLParser.RULE_methodSpecificString)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(60)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamsContext: ParserRuleContext {
			open
			func param() -> [ParamContext] {
				return getRuleContexts(ParamContext.self)
			}
			open
			func param(_ i: Int) -> ParamContext? {
				return getRuleContext(ParamContext.self, i)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_params
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParams(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParams(self)
			}
		}
	}
	@discardableResult
	 open func params() throws -> ParamsContext {
		var _localctx: ParamsContext = ParamsContext(_ctx, getState())
		try enterRule(_localctx, 8, DIDURLParser.RULE_params)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(62)
		 	try param()
		 	setState(67)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	while (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__0.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(63)
		 		try match(DIDURLParser.Tokens.T__0.rawValue)
		 		setState(64)
		 		try param()


		 		setState(69)
		 		try _errHandler.sync(self)
		 		_la = try _input.LA(1)
		 	}

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamContext: ParserRuleContext {
			open
			func paramQName() -> ParamQNameContext? {
				return getRuleContext(ParamQNameContext.self, 0)
			}
			open
			func paramValue() -> ParamValueContext? {
				return getRuleContext(ParamValueContext.self, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_param
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParam(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParam(self)
			}
		}
	}
	@discardableResult
	 open func param() throws -> ParamContext {
		var _localctx: ParamContext = ParamContext(_ctx, getState())
		try enterRule(_localctx, 10, DIDURLParser.RULE_param)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(70)
		 	try paramQName()
		 	setState(73)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__6.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(71)
		 		try match(DIDURLParser.Tokens.T__6.rawValue)
		 		setState(72)
		 		try paramValue()

		 	}


		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamQNameContext: ParserRuleContext {
			open
			func paramName() -> ParamNameContext? {
				return getRuleContext(ParamNameContext.self, 0)
			}
			open
			func paramMethod() -> ParamMethodContext? {
				return getRuleContext(ParamMethodContext.self, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_paramQName
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParamQName(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParamQName(self)
			}
		}
	}
	@discardableResult
	 open func paramQName() throws -> ParamQNameContext {
		var _localctx: ParamQNameContext = ParamQNameContext(_ctx, getState())
		try enterRule(_localctx, 12, DIDURLParser.RULE_paramQName)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(78)
		 	try _errHandler.sync(self)
		 	switch (try getInterpreter().adaptivePredict(_input,7,_ctx)) {
		 	case 1:
		 		setState(75)
		 		try paramMethod()
		 		setState(76)
		 		try match(DIDURLParser.Tokens.T__5.rawValue)

		 		break
		 	default: break
		 	}
		 	setState(80)
		 	try paramName()

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamMethodContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_paramMethod
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParamMethod(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParamMethod(self)
			}
		}
	}
	@discardableResult
	 open func paramMethod() throws -> ParamMethodContext {
		var _localctx: ParamMethodContext = ParamMethodContext(_ctx, getState())
		try enterRule(_localctx, 14, DIDURLParser.RULE_paramMethod)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(82)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamNameContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_paramName
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParamName(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParamName(self)
			}
		}
	}
	@discardableResult
	 open func paramName() throws -> ParamNameContext {
		var _localctx: ParamNameContext = ParamNameContext(_ctx, getState())
		try enterRule(_localctx, 16, DIDURLParser.RULE_paramName)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(84)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ParamValueContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_paramValue
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterParamValue(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitParamValue(self)
			}
		}
	}
	@discardableResult
	 open func paramValue() throws -> ParamValueContext {
		var _localctx: ParamValueContext = ParamValueContext(_ctx, getState())
		try enterRule(_localctx, 18, DIDURLParser.RULE_paramValue)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(86)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class PathContext: ParserRuleContext {
			open
			func STRING() -> [TerminalNode] {
				return getTokens(DIDURLParser.Tokens.STRING.rawValue)
			}
			open
			func STRING(_ i:Int) -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, i)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_path
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterPath(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitPath(self)
			}
		}
	}
	@discardableResult
	 open func path() throws -> PathContext {
		var _localctx: PathContext = PathContext(_ctx, getState())
		try enterRule(_localctx, 20, DIDURLParser.RULE_path)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(88)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)
		 	setState(93)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	while (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__1.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(89)
		 		try match(DIDURLParser.Tokens.T__1.rawValue)
		 		setState(90)
		 		try match(DIDURLParser.Tokens.STRING.rawValue)


		 		setState(95)
		 		try _errHandler.sync(self)
		 		_la = try _input.LA(1)
		 	}

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class QueryContext: ParserRuleContext {
			open
			func queryParam() -> [QueryParamContext] {
				return getRuleContexts(QueryParamContext.self)
			}
			open
			func queryParam(_ i: Int) -> QueryParamContext? {
				return getRuleContext(QueryParamContext.self, i)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_query
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterQuery(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitQuery(self)
			}
		}
	}
	@discardableResult
	 open func query() throws -> QueryContext {
		var _localctx: QueryContext = QueryContext(_ctx, getState())
		try enterRule(_localctx, 22, DIDURLParser.RULE_query)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(96)
		 	try queryParam()
		 	setState(101)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	while (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__7.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(97)
		 		try match(DIDURLParser.Tokens.T__7.rawValue)
		 		setState(98)
		 		try queryParam()


		 		setState(103)
		 		try _errHandler.sync(self)
		 		_la = try _input.LA(1)
		 	}

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class QueryParamContext: ParserRuleContext {
			open
			func queryParamName() -> QueryParamNameContext? {
				return getRuleContext(QueryParamNameContext.self, 0)
			}
			open
			func queryParamValue() -> QueryParamValueContext? {
				return getRuleContext(QueryParamValueContext.self, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_queryParam
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterQueryParam(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitQueryParam(self)
			}
		}
	}
	@discardableResult
	 open func queryParam() throws -> QueryParamContext {
		var _localctx: QueryParamContext = QueryParamContext(_ctx, getState())
		try enterRule(_localctx, 24, DIDURLParser.RULE_queryParam)
		var _la: Int = 0
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(104)
		 	try queryParamName()
		 	setState(107)
		 	try _errHandler.sync(self)
		 	_la = try _input.LA(1)
		 	if (//closure
		 	 { () -> Bool in
		 	      let testSet: Bool = _la == DIDURLParser.Tokens.T__6.rawValue
		 	      return testSet
		 	 }()) {
		 		setState(105)
		 		try match(DIDURLParser.Tokens.T__6.rawValue)
		 		setState(106)
		 		try queryParamValue()

		 	}


		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class QueryParamNameContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_queryParamName
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterQueryParamName(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitQueryParamName(self)
			}
		}
	}
	@discardableResult
	 open func queryParamName() throws -> QueryParamNameContext {
		var _localctx: QueryParamNameContext = QueryParamNameContext(_ctx, getState())
		try enterRule(_localctx, 26, DIDURLParser.RULE_queryParamName)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(109)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class QueryParamValueContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_queryParamValue
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterQueryParamValue(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitQueryParamValue(self)
			}
		}
	}
	@discardableResult
	 open func queryParamValue() throws -> QueryParamValueContext {
		var _localctx: QueryParamValueContext = QueryParamValueContext(_ctx, getState())
		try enterRule(_localctx, 28, DIDURLParser.RULE_queryParamValue)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(111)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class FragContext: ParserRuleContext {
			open
			func STRING() -> TerminalNode? {
				return getToken(DIDURLParser.Tokens.STRING.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return DIDURLParser.RULE_frag
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.enterFrag(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? DIDURLListener {
				listener.exitFrag(self)
			}
		}
	}
	@discardableResult
	 open func frag() throws -> FragContext {
		var _localctx: FragContext = FragContext(_ctx, getState())
		try enterRule(_localctx, 30, DIDURLParser.RULE_frag)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(113)
		 	try match(DIDURLParser.Tokens.STRING.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}


	public
	static let _serializedATN = DIDURLParserATN().jsonString

	public
	static let _ATN = ATNDeserializer().deserializeFromJson(_serializedATN)
}