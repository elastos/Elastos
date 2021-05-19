// Generated from DIDURL.g4 by ANTLR 4.7.2
package org.elastos.did.parser;
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class DIDURLLexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.7.2", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, STRING=9, 
		HEX=10, SPACE=11;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "STRING", 
			"HEX", "SPACE"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "';'", "'/'", "'?'", "'#'", "'did'", "':'", "'='", "'&'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, null, "STRING", "HEX", 
			"SPACE"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}


	public DIDURLLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "DIDURL.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\3\u608b\ua72a\u8133\ub9ed\u417c\u3be7\u7786\u5964\2\rB\b\1\4\2\t\2\4"+
		"\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t"+
		"\13\4\f\t\f\3\2\3\2\3\3\3\3\3\4\3\4\3\5\3\5\3\6\3\6\3\6\3\6\3\7\3\7\3"+
		"\b\3\b\3\t\3\t\3\n\3\n\5\n.\n\n\3\n\3\n\7\n\62\n\n\f\n\16\n\65\13\n\3"+
		"\13\3\13\3\13\6\13:\n\13\r\13\16\13;\3\f\6\f?\n\f\r\f\16\f@\2\2\r\3\3"+
		"\5\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\27\r\3\2\6\6\2\62;C\\c|\u0080"+
		"\u0080\6\2/\60\62;C\\c|\5\2\62;CHch\5\2\13\f\17\17\"\"\2F\2\3\3\2\2\2"+
		"\2\5\3\2\2\2\2\7\3\2\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2"+
		"\2\2\2\21\3\2\2\2\2\23\3\2\2\2\2\25\3\2\2\2\2\27\3\2\2\2\3\31\3\2\2\2"+
		"\5\33\3\2\2\2\7\35\3\2\2\2\t\37\3\2\2\2\13!\3\2\2\2\r%\3\2\2\2\17\'\3"+
		"\2\2\2\21)\3\2\2\2\23-\3\2\2\2\259\3\2\2\2\27>\3\2\2\2\31\32\7=\2\2\32"+
		"\4\3\2\2\2\33\34\7\61\2\2\34\6\3\2\2\2\35\36\7A\2\2\36\b\3\2\2\2\37 \7"+
		"%\2\2 \n\3\2\2\2!\"\7f\2\2\"#\7k\2\2#$\7f\2\2$\f\3\2\2\2%&\7<\2\2&\16"+
		"\3\2\2\2\'(\7?\2\2(\20\3\2\2\2)*\7(\2\2*\22\3\2\2\2+.\t\2\2\2,.\5\25\13"+
		"\2-+\3\2\2\2-,\3\2\2\2.\63\3\2\2\2/\62\t\3\2\2\60\62\5\25\13\2\61/\3\2"+
		"\2\2\61\60\3\2\2\2\62\65\3\2\2\2\63\61\3\2\2\2\63\64\3\2\2\2\64\24\3\2"+
		"\2\2\65\63\3\2\2\2\66\67\7\'\2\2\678\t\4\2\28:\t\4\2\29\66\3\2\2\2:;\3"+
		"\2\2\2;9\3\2\2\2;<\3\2\2\2<\26\3\2\2\2=?\t\5\2\2>=\3\2\2\2?@\3\2\2\2@"+
		">\3\2\2\2@A\3\2\2\2A\30\3\2\2\2\b\2-\61\63;@\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}