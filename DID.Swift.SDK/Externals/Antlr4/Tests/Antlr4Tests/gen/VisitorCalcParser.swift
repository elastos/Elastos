// Generated from /Users/liaihong/Desktop/AN/antlr4/runtime/Swift/Tests/Antlr4Tests/VisitorCalc.g4 by ANTLR 4.7.1
import Antlr4

open class VisitorCalcParser: Parser {

	internal static var _decisionToDFA: [DFA] = {
          var decisionToDFA = [DFA]()
          let length = VisitorCalcParser._ATN.getNumberOfDecisions()
          for i in 0..<length {
            decisionToDFA.append(DFA(VisitorCalcParser._ATN.getDecisionState(i)!, i))
           }
           return decisionToDFA
     }()

	internal static let _sharedContextCache = PredictionContextCache()

	public
	enum Tokens: Int {
		case EOF = -1, INT = 1, MUL = 2, DIV = 3, ADD = 4, SUB = 5, WS = 6
	}

	public
	static let RULE_s = 0, RULE_expr = 1

	public
	static let ruleNames: [String] = [
		"s", "expr"
	]

	private static let _LITERAL_NAMES: [String?] = [
		nil, nil, "'*'", "'/'", "'+'", "'-'"
	]
	private static let _SYMBOLIC_NAMES: [String?] = [
		nil, "INT", "MUL", "DIV", "ADD", "SUB", "WS"
	]
	public
	static let VOCABULARY = Vocabulary(_LITERAL_NAMES, _SYMBOLIC_NAMES)

	override open
	func getGrammarFileName() -> String { return "VisitorCalc.g4" }

	override open
	func getRuleNames() -> [String] { return VisitorCalcParser.ruleNames }

	override open
	func getSerializedATN() -> String { return VisitorCalcParser._serializedATN }

	override open
	func getATN() -> ATN { return VisitorCalcParser._ATN }

	override open
	func getVocabulary() -> Vocabulary {
	    return VisitorCalcParser.VOCABULARY
	}

	override public
	init(_ input:TokenStream) throws {
	    RuntimeMetaData.checkVersion("4.7.1", RuntimeMetaData.VERSION)
		try super.init(input)
		_interp = ParserATNSimulator(self,VisitorCalcParser._ATN,VisitorCalcParser._decisionToDFA, VisitorCalcParser._sharedContextCache)
	}

	public class SContext: ParserRuleContext {
			open
			func expr() -> ExprContext? {
				return getRuleContext(ExprContext.self, 0)
			}
			open
			func EOF() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.EOF.rawValue, 0)
			}
		override open
		func getRuleIndex() -> Int {
			return VisitorCalcParser.RULE_s
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.enterS(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.exitS(self)
			}
		}
		override open
		func accept<T>(_ visitor: ParseTreeVisitor<T>) -> T? {
			if let visitor = visitor as? VisitorCalcVisitor {
			    return visitor.visitS(self)
			}
			else if let visitor = visitor as? VisitorCalcBaseVisitor {
			    return visitor.visitS(self)
			}
			else {
			     return visitor.visitChildren(self)
			}
		}
	}
	@discardableResult
	 open func s() throws -> SContext {
		var _localctx: SContext = SContext(_ctx, getState())
		try enterRule(_localctx, 0, VisitorCalcParser.RULE_s)
		defer {
	    		try! exitRule()
	    }
		do {
		 	try enterOuterAlt(_localctx, 1)
		 	setState(4)
		 	try expr(0)
		 	setState(5)
		 	try match(VisitorCalcParser.Tokens.EOF.rawValue)

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx
	}

	public class ExprContext: ParserRuleContext {
		override open
		func getRuleIndex() -> Int {
			return VisitorCalcParser.RULE_expr
		}
	 
		open
		func copyFrom(_ ctx: ExprContext) {
			super.copyFrom(ctx)
		}
	}
	public class NumberContext: ExprContext {
			open
			func INT() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.INT.rawValue, 0)
			}

		public
		init(_ ctx: ExprContext) {
			super.init()
			copyFrom(ctx)
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.enterNumber(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.exitNumber(self)
			}
		}
		override open
		func accept<T>(_ visitor: ParseTreeVisitor<T>) -> T? {
			if let visitor = visitor as? VisitorCalcVisitor {
			    return visitor.visitNumber(self)
			}
			else if let visitor = visitor as? VisitorCalcBaseVisitor {
			    return visitor.visitNumber(self)
			}
			else {
			     return visitor.visitChildren(self)
			}
		}
	}
	public class MultiplyContext: ExprContext {
			open
			func expr() -> [ExprContext] {
				return getRuleContexts(ExprContext.self)
			}
			open
			func expr(_ i: Int) -> ExprContext? {
				return getRuleContext(ExprContext.self, i)
			}
			open
			func MUL() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.MUL.rawValue, 0)
			}
			open
			func DIV() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.DIV.rawValue, 0)
			}

		public
		init(_ ctx: ExprContext) {
			super.init()
			copyFrom(ctx)
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.enterMultiply(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.exitMultiply(self)
			}
		}
		override open
		func accept<T>(_ visitor: ParseTreeVisitor<T>) -> T? {
			if let visitor = visitor as? VisitorCalcVisitor {
			    return visitor.visitMultiply(self)
			}
			else if let visitor = visitor as? VisitorCalcBaseVisitor {
			    return visitor.visitMultiply(self)
			}
			else {
			     return visitor.visitChildren(self)
			}
		}
	}
	public class AddContext: ExprContext {
			open
			func expr() -> [ExprContext] {
				return getRuleContexts(ExprContext.self)
			}
			open
			func expr(_ i: Int) -> ExprContext? {
				return getRuleContext(ExprContext.self, i)
			}
			open
			func ADD() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.ADD.rawValue, 0)
			}
			open
			func SUB() -> TerminalNode? {
				return getToken(VisitorCalcParser.Tokens.SUB.rawValue, 0)
			}

		public
		init(_ ctx: ExprContext) {
			super.init()
			copyFrom(ctx)
		}
		override open
		func enterRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.enterAdd(self)
			}
		}
		override open
		func exitRule(_ listener: ParseTreeListener) {
			if let listener = listener as? VisitorCalcListener {
				listener.exitAdd(self)
			}
		}
		override open
		func accept<T>(_ visitor: ParseTreeVisitor<T>) -> T? {
			if let visitor = visitor as? VisitorCalcVisitor {
			    return visitor.visitAdd(self)
			}
			else if let visitor = visitor as? VisitorCalcBaseVisitor {
			    return visitor.visitAdd(self)
			}
			else {
			     return visitor.visitChildren(self)
			}
		}
	}

	 public final  func expr( ) throws -> ExprContext   {
		return try expr(0)
	}
	@discardableResult
	private func expr(_ _p: Int) throws -> ExprContext   {
		let _parentctx: ParserRuleContext? = _ctx
		var _parentState: Int = getState()
		var _localctx: ExprContext = ExprContext(_ctx, _parentState)
		var  _prevctx: ExprContext = _localctx
		var _startState: Int = 2
		try enterRecursionRule(_localctx, 2, VisitorCalcParser.RULE_expr, _p)
		var _la: Int = 0
		defer {
	    		try! unrollRecursionContexts(_parentctx)
	    }
		do {
			var _alt: Int
			try enterOuterAlt(_localctx, 1)
			_localctx = NumberContext(_localctx)
			_ctx = _localctx
			_prevctx = _localctx

			setState(8)
			try match(VisitorCalcParser.Tokens.INT.rawValue)

			_ctx!.stop = try _input.LT(-1)
			setState(18)
			try _errHandler.sync(self)
			_alt = try getInterpreter().adaptivePredict(_input,1,_ctx)
			while (_alt != 2 && _alt != ATN.INVALID_ALT_NUMBER) {
				if ( _alt==1 ) {
					if _parseListeners != nil {
					   try triggerExitRuleEvent()
					}
					_prevctx = _localctx
					setState(16)
					try _errHandler.sync(self)
					switch(try getInterpreter().adaptivePredict(_input,0, _ctx)) {
					case 1:
						_localctx = MultiplyContext(  ExprContext(_parentctx, _parentState))
						try pushNewRecursionContext(_localctx, _startState, VisitorCalcParser.RULE_expr)
						setState(10)
						if (!(precpred(_ctx, 2))) {
						    throw ANTLRException.recognition(e:FailedPredicateException(self, "precpred(_ctx, 2)"))
						}
						setState(11)
						_la = try _input.LA(1)
						if (!(//closure
						 { () -> Bool in
						      let testSet: Bool = _la == VisitorCalcParser.Tokens.MUL.rawValue || _la == VisitorCalcParser.Tokens.DIV.rawValue
						      return testSet
						 }())) {
						try _errHandler.recoverInline(self)
						}
						else {
							_errHandler.reportMatch(self)
							try consume()
						}
						setState(12)
						try expr(3)

						break
					case 2:
						_localctx = AddContext(  ExprContext(_parentctx, _parentState))
						try pushNewRecursionContext(_localctx, _startState, VisitorCalcParser.RULE_expr)
						setState(13)
						if (!(precpred(_ctx, 1))) {
						    throw ANTLRException.recognition(e:FailedPredicateException(self, "precpred(_ctx, 1)"))
						}
						setState(14)
						_la = try _input.LA(1)
						if (!(//closure
						 { () -> Bool in
						      let testSet: Bool = _la == VisitorCalcParser.Tokens.ADD.rawValue || _la == VisitorCalcParser.Tokens.SUB.rawValue
						      return testSet
						 }())) {
						try _errHandler.recoverInline(self)
						}
						else {
							_errHandler.reportMatch(self)
							try consume()
						}
						setState(15)
						try expr(2)

						break
					default: break
					}
			 
				}
				setState(20)
				try _errHandler.sync(self)
				_alt = try getInterpreter().adaptivePredict(_input,1,_ctx)
			}

		}
		catch ANTLRException.recognition(let re) {
			_localctx.exception = re
			_errHandler.reportError(self, re)
			try _errHandler.recover(self, re)
		}

		return _localctx;
	}

	override open
	func sempred(_ _localctx: RuleContext?, _ ruleIndex: Int,  _ predIndex: Int)throws -> Bool {
		switch (ruleIndex) {
		case  1:
			return try expr_sempred(_localctx?.castdown(ExprContext.self), predIndex)
	    default: return true
		}
	}
	private func expr_sempred(_ _localctx: ExprContext!,  _ predIndex: Int) throws -> Bool {
		switch (predIndex) {
		    case 0:return precpred(_ctx, 2)
		    case 1:return precpred(_ctx, 1)
		    default: return true
		}
	}


	public
	static let _serializedATN = VisitorCalcParserATN().jsonString

	public
	static let _ATN = ATNDeserializer().deserializeFromJson(_serializedATN)
}
