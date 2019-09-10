// Generated from DIDURL.g4 by ANTLR 4.7.2
import Antlr4

open class DIDURLLexer: Lexer {

	internal static var _decisionToDFA: [DFA] = {
          var decisionToDFA = [DFA]()
          let length = DIDURLLexer._ATN.getNumberOfDecisions()
          for i in 0..<length {
          	    decisionToDFA.append(DFA(DIDURLLexer._ATN.getDecisionState(i)!, i))
          }
           return decisionToDFA
     }()

	internal static let _sharedContextCache = PredictionContextCache()

	public
	static let T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, 
            STRING=9, HEX=10, SPACE=11

	public
	static let channelNames: [String] = [
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	]

	public
	static let modeNames: [String] = [
		"DEFAULT_MODE"
	]

	public
	static let ruleNames: [String] = [
		"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "STRING", 
		"HEX", "SPACE"
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
	func getVocabulary() -> Vocabulary {
		return DIDURLLexer.VOCABULARY
	}

	public
	required init(_ input: CharStream) {
	    RuntimeMetaData.checkVersion("4.7.2", RuntimeMetaData.VERSION)
		super.init(input)
		_interp = LexerATNSimulator(self, DIDURLLexer._ATN, DIDURLLexer._decisionToDFA, DIDURLLexer._sharedContextCache)
	}

	override open
	func getGrammarFileName() -> String { return "DIDURL.g4" }

	override open
	func getRuleNames() -> [String] { return DIDURLLexer.ruleNames }

	override open
	func getSerializedATN() -> String { return DIDURLLexer._serializedATN }

	override open
	func getChannelNames() -> [String] { return DIDURLLexer.channelNames }

	override open
	func getModeNames() -> [String] { return DIDURLLexer.modeNames }

	override open
	func getATN() -> ATN { return DIDURLLexer._ATN }


	public
	static let _serializedATN: String = DIDURLLexerATN().jsonString

	public
	static let _ATN: ATN = ATNDeserializer().deserializeFromJson(_serializedATN)
}