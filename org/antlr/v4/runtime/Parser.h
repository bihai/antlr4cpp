﻿#pragma once

#include "Recognizer.h"
#include "ParseTreeListener.h"
#include "TokenStream.h"


#include <stack>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include "Declarations.h"


/*
 * [The "BSD license"]
 *  Copyright (c) 2013 Terence Parr
 *  Copyright (c) 2013 Dan McLaughlin
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

namespace org {
    namespace antlr {
        namespace v4 {
            namespace runtime {

                
                /// <summary>
                /// This is all the parsing support code essentially; most of it is error recovery stuff. </summary>
                class Parser : public Recognizer<Token*, atn::ParserATNSimulator*> {
                public:
                    class TraceListener : public tree::ParseTreeListener {
                        private:
                        Parser *const outerInstance;

                        public:
                        TraceListener(Parser *outerInstance);

                        virtual void enterEveryRule(ParserRuleContext *ctx) override;

                        virtual void visitTerminal(tree::TerminalNode *node) override;

                        virtual void visitErrorNode(tree::ErrorNode *node) override;

                        virtual void exitEveryRule(ParserRuleContext *ctx) override;
                    };

                public:
                    class TrimToSizeListener : public tree::ParseTreeListener {
                    public:
                        static TrimToSizeListener *const INSTANCE;

                        virtual void enterEveryRule(ParserRuleContext *ctx) override;
                        virtual void visitTerminal(tree::TerminalNode *node) override;
                        virtual void visitErrorNode(tree::ErrorNode *node) override;
                        virtual void exitEveryRule(ParserRuleContext *ctx) override;
                    };

                    /// <summary>
                    /// This field maps from the serialized ATN string to the deserialized <seealso cref="ATN"/> with
                    /// bypass alternatives.
                    /// </summary>
                    /// <seealso cref= ATNDeserializationOptions#isGenerateRuleBypassTransitions() </seealso>
                private:
                    static std::map<std::wstring, atn::ATN*> * bypassAltsAtnCache;

                    /// <summary>
                    /// The error handling strategy for the parser. The default value is a new
                    /// instance of <seealso cref="DefaultErrorStrategy"/>.
                    /// </summary>
                    /// <seealso cref= #getErrorHandler </seealso>
                    /// <seealso cref= #setErrorHandler </seealso>
                protected:
                    ANTLRErrorStrategy *_errHandler;

                    /// <summary>
                    /// The input stream.
                    /// </summary>
                    /// <seealso cref= #getInputStream </seealso>
                    /// <seealso cref= #setInputStream </seealso>
                    TokenStream *_input;

                    std::vector<int> _precedenceStack;

                    /// <summary>
                    /// The <seealso cref="ParserRuleContext"/> object for the currently executing rule.
                    /// This is always non-null during the parsing process.
                    /// </summary>
                public:
                    ParserRuleContext *_ctx;

                    /// <summary>
                    /// Specifies whether or not the parser should construct a parse tree during
                    /// the parsing process. The default value is {@code true}.
                    /// </summary>
                    /// <seealso cref= #getBuildParseTree </seealso>
                    /// <seealso cref= #setBuildParseTree </seealso>
                protected:
                    bool _buildParseTrees;


                    /// <summary>
                    /// When <seealso cref="#setTrace"/>{@code (true)} is called, a reference to the
                    /// <seealso cref="TraceListener"/> is stored here so it can be easily removed in a
                    /// later call to <seealso cref="#setTrace"/>{@code (false)}. The listener itself is
                    /// implemented as a parser listener so this field is not directly used by
                    /// other parser methods.
                    /// </summary>
                private:
                    TraceListener *_tracer;

                    /// <summary>
                    /// The list of <seealso cref="ParseTreeListener"/> listeners registered to receive
                    /// events during the parse.
                    /// </summary>
                    /// <seealso cref= #addParseListener </seealso>
                protected:
                    std::vector<tree::ParseTreeListener*> _parseListeners;

                    /// <summary>
                    /// The number of syntax errors reported during parsing. This value is
                    /// incremented each time <seealso cref="#notifyErrorListeners"/> is called.
                    /// </summary>
                    int _syntaxErrors;

                public:
                    Parser(TokenStream *input);

                    /// <summary>
                    /// reset the parser's state </summary>
                    virtual void reset();

                    /// <summary>
                    /// Match current input symbol against {@code ttype}. If the symbol type
                    /// matches, <seealso cref="ANTLRErrorStrategy#reportMatch"/> and <seealso cref="#consume"/> are
                    /// called to complete the match process.
                    /// <p/>
                    /// If the symbol type does not match,
                    /// <seealso cref="ANTLRErrorStrategy#recoverInline"/> is called on the current error
                    /// strategy to attempt recovery. If <seealso cref="#getBuildParseTree"/> is
                    /// {@code true} and the token index of the symbol returned by
                    /// <seealso cref="ANTLRErrorStrategy#recoverInline"/> is -1, the symbol is added to
                    /// the parse tree by calling <seealso cref="ParserRuleContext#addErrorNode"/>.
                    /// </summary>
                    /// <param name="ttype"> the token type to match </param>
                    /// <returns> the matched symbol </returns>
                    /// <exception cref="RecognitionException"> if the current input symbol did not match
                    /// {@code ttype} and the error strategy could not recover from the
                    /// mismatched symbol </exception>
                    virtual Token *match(int ttype);

                    /// <summary>
                    /// Match current input symbol as a wildcard. If the symbol type matches
                    /// (i.e. has a value greater than 0), <seealso cref="ANTLRErrorStrategy#reportMatch"/>
                    /// and <seealso cref="#consume"/> are called to complete the match process.
                    /// <p/>
                    /// If the symbol type does not match,
                    /// <seealso cref="ANTLRErrorStrategy#recoverInline"/> is called on the current error
                    /// strategy to attempt recovery. If <seealso cref="#getBuildParseTree"/> is
                    /// {@code true} and the token index of the symbol returned by
                    /// <seealso cref="ANTLRErrorStrategy#recoverInline"/> is -1, the symbol is added to
                    /// the parse tree by calling <seealso cref="ParserRuleContext#addErrorNode"/>.
                    /// </summary>
                    /// <returns> the matched symbol </returns>
                    /// <exception cref="RecognitionException"> if the current input symbol did not match
                    /// a wildcard and the error strategy could not recover from the mismatched
                    /// symbol </exception>
                    virtual Token *matchWildcard();

                    /// <summary>
                    /// Track the <seealso cref="ParserRuleContext"/> objects during the parse and hook
                    /// them up using the <seealso cref="ParserRuleContext#children"/> list so that it
                    /// forms a parse tree. The <seealso cref="ParserRuleContext"/> returned from the start
                    /// rule represents the root of the parse tree.
                    /// <p/>
                    /// Note that if we are not building parse trees, rule contexts only point
                    /// upwards. When a rule exits, it returns the context but that gets garbage
                    /// collected if nobody holds a reference. It points upwards but nobody
                    /// points at it.
                    /// <p/>
                    /// When we build parse trees, we are adding all of these contexts to
                    /// <seealso cref="ParserRuleContext#children"/> list. Contexts are then not candidates
                    /// for garbage collection.
                    /// </summary>
                    virtual void setBuildParseTree(bool buildParseTrees);

                    /// <summary>
                    /// Gets whether or not a complete parse tree will be constructed while
                    /// parsing. This property is {@code true} for a newly constructed parser.
                    /// </summary>
                    /// <returns> {@code true} if a complete parse tree will be constructed while
                    /// parsing, otherwise {@code false} </returns>
                    virtual bool getBuildParseTree();

                    /// <summary>
                    /// Trim the internal lists of the parse tree during parsing to conserve memory.
                    /// This property is set to {@code false} by default for a newly constructed parser.
                    /// </summary>
                    /// <param name="trimParseTrees"> {@code true} to trim the capacity of the <seealso cref="ParserRuleContext#children"/>
                    /// list to its size after a rule is parsed. </param>
                    virtual void setTrimParseTree(bool trimParseTrees);

                    /// <returns> {@code true} if the <seealso cref="ParserRuleContext#children"/> list is trimmed
                    /// using the default <seealso cref="Parser.TrimToSizeListener"/> during the parse process. </returns>
                    virtual bool getTrimParseTree();

                    virtual std::vector<tree::ParseTreeListener*> getParseListeners();

                    /// <summary>
                    /// Registers {@code listener} to receive events during the parsing process.
                    /// <p/>
                    /// To support output-preserving grammar transformations (including but not
                    /// limited to left-recursion removal, automated left-factoring, and
                    /// optimized code generation), calls to listener methods during the parse
                    /// may differ substantially from calls made by
                    /// <seealso cref="ParseTreeWalker#DEFAULT"/> used after the parse is complete. In
                    /// particular, rule entry and exit events may occur in a different order
                    /// during the parse than after the parser. In addition, calls to certain
                    /// rule entry methods may be omitted.
                    /// <p/>
                    /// With the following specific exceptions, calls to listener events are
                    /// <em>deterministic</em>, i.e. for identical input the calls to listener
                    /// methods will be the same.
                    /// 
                    /// <ul>
                    /// <li>Alterations to the grammar used to generate code may change the
                    /// behavior of the listener calls.</li>
                    /// <li>Alterations to the command line options passed to ANTLR 4 when
                    /// generating the parser may change the behavior of the listener calls.</li>
                    /// <li>Changing the version of the ANTLR Tool used to generate the parser
                    /// may change the behavior of the listener calls.</li>
                    /// </ul>
                    /// </summary>
                    /// <param name="listener"> the listener to add
                    /// </param>
                    /// <exception cref="NullPointerException"> if {@code} listener is {@code null} </exception>
                    virtual void addParseListener(tree::ParseTreeListener *listener);

                    /// <summary>
                    /// Remove {@code listener} from the list of parse listeners.
                    /// <p/>
                    /// If {@code listener} is {@code null} or has not been added as a parse
                    /// listener, this method does nothing.
                    /// </summary>
                    /// <seealso cref= #addParseListener
                    /// </seealso>
                    /// <param name="listener"> the listener to remove </param>
                    virtual void removeParseListener(tree::ParseTreeListener *listener);

                    /// <summary>
                    /// Remove all parse listeners.
                    /// </summary>
                    /// <seealso cref= #addParseListener </seealso>
                    virtual void removeParseListeners();

                    /// <summary>
                    /// Notify any parse listeners of an enter rule event.
                    /// </summary>
                    /// <seealso cref= #addParseListener </seealso>
                protected:
                    virtual void triggerEnterRuleEvent();

                    /// <summary>
                    /// Notify any parse listeners of an exit rule event.
                    /// </summary>
                    /// <seealso cref= #addParseListener </seealso>
                    virtual void triggerExitRuleEvent();

                    /// <summary>
                    /// Gets the number of syntax errors reported during parsing. This value is
                    /// incremented each time <seealso cref="#notifyErrorListeners"/> is called.
                    /// </summary>
                    /// <seealso cref= #notifyErrorListeners </seealso>
                public:
                    virtual int getNumberOfSyntaxErrors();

                    virtual TokenFactory<CommonToken*> *getTokenFactory() override;

                    /// <summary>
                    /// Tell our token source and error strategy about a new way to create tokens. </summary>
                    template<typename T1>
                    void setTokenFactory(TokenFactory<T1> *factory);

                    /// <summary>
                    /// The ATN with bypass alternatives is expensive to create so we create it
                    /// lazily.
                    /// </summary>
                    /// <exception cref="UnsupportedOperationException"> if the current parser does not
                    /// implement the <seealso cref="#getSerializedATN()"/> method. </exception>
                    virtual atn::ATN *getATNWithBypassAlts();

                    /// <summary>
                    /// The preferred method of getting a tree pattern. For example, here's a
                    /// sample use:
                    /// 
                    /// <pre>
                    /// ParseTree t = parser.expr();
                    /// ParseTreePattern p = parser.compileParseTreePattern("<ID>+0", MyParser.RULE_expr);
                    /// ParseTreeMatch m = p.match(t);
                    /// String id = m.get("ID");
                    /// </pre>
                    /// </summary>
                    virtual tree::pattern::ParseTreePattern *compileParseTreePattern(const std::wstring &pattern, int patternRuleIndex);

                    /// <summary>
                    /// The same as <seealso cref="#compileParseTreePattern(String, int)"/> but specify a
                    /// <seealso cref="Lexer"/> rather than trying to deduce it from this parser.
                    /// </summary>
                    virtual tree::pattern::ParseTreePattern *compileParseTreePattern(const std::wstring &pattern, int patternRuleIndex, Lexer *lexer);

                    virtual ANTLRErrorStrategy *getErrorHandler();

                    virtual void setErrorHandler(ANTLRErrorStrategy *handler);

                    virtual TokenStream *getInputStream() override;
                    void setInputStream(IntStream *input) override;

                    virtual TokenStream *getTokenStream();

                    /// <summary>
                    /// Set the token stream and reset the parser. </summary>
                    virtual void setTokenStream(TokenStream *input);

                    /// <summary>
                    /// Match needs to return the current input symbol, which gets put
                    ///  into the label for the associated token ref; e.g., x=ID.
                    /// </summary>
                    virtual Token *getCurrentToken();

                    void notifyErrorListeners(const std::wstring &msg);

                    virtual void notifyErrorListeners(Token *offendingToken, const std::wstring &msg, RecognitionException *e);

                    /// <summary>
                    /// Consume and return the <seealso cref="#getCurrentToken current symbol"/>.
                    /// <p/>
                    /// E.g., given the following input with {@code A} being the current
                    /// lookahead symbol, this function moves the cursor to {@code B} and returns
                    /// {@code A}.
                    /// 
                    /// <pre>
                    ///  A B
                    ///  ^
                    /// </pre>
                    /// 
                    /// If the parser is not in error recovery mode, the consumed symbol is added
                    /// to the parse tree using <seealso cref="ParserRuleContext#addChild(Token)"/>, and
                    /// <seealso cref="ParseTreeListener#visitTerminal"/> is called on any parse listeners.
                    /// If the parser <em>is</em> in error recovery mode, the consumed symbol is
                    /// added to the parse tree using
                    /// <seealso cref="ParserRuleContext#addErrorNode(Token)"/>, and
                    /// <seealso cref="ParseTreeListener#visitErrorNode"/> is called on any parse
                    /// listeners.
                    /// </summary>
                    virtual Token *consume();

                protected:
                    virtual void addContextToParseTree();

                    /// <summary>
                    /// Always called by generated parsers upon entry to a rule. Access field
                    /// <seealso cref="#_ctx"/> get the current context.
                    /// </summary>
                public:
                    virtual void enterRule(ParserRuleContext *localctx, int state, int ruleIndex);

                    virtual void exitRule();

                    virtual void enterOuterAlt(ParserRuleContext *localctx, int altNum);

                    /// @deprecated Use
                    /// <seealso cref="#enterRecursionRule(ParserRuleContext, int, int, int)"/> instead. 
                    virtual void enterRecursionRule(ParserRuleContext *localctx, int ruleIndex);

                    virtual void enterRecursionRule(ParserRuleContext *localctx, int state, int ruleIndex, int precedence);

                    /// <summary>
                    /// Like <seealso cref="#enterRule"/> but for recursive rules.
                    /// </summary>
                    virtual void pushNewRecursionContext(ParserRuleContext *localctx, int state, int ruleIndex);

                    virtual void unrollRecursionContexts(ParserRuleContext *_parentctx);

                    virtual ParserRuleContext *getInvokingContext(int ruleIndex);

                    virtual ParserRuleContext *getContext();

                    virtual void setContext(ParserRuleContext *ctx);

                    virtual bool precpred(RuleContext *localctx, int precedence) override;

                    virtual bool inContext(const std::wstring &context);

                    /// <summary>
                    /// Checks whether or not {@code symbol} can follow the current state in the
                    /// ATN. The behavior of this method is equivalent to the following, but is
                    /// implemented such that the complete context-sensitive follow set does not
                    /// need to be explicitly constructed.
                    /// 
                    /// <pre>
                    /// return getExpectedTokens().contains(symbol);
                    /// </pre>
                    /// </summary>
                    /// <param name="symbol"> the symbol type to check </param>
                    /// <returns> {@code true} if {@code symbol} can follow the current state in
                    /// the ATN, otherwise {@code false}. </returns>
                    virtual bool isExpectedToken(int symbol);

                    /// <summary>
                    /// Computes the set of input symbols which could follow the current parser
                    /// state and context, as given by <seealso cref="#getState"/> and <seealso cref="#getContext"/>,
                    /// respectively.
                    /// </summary>
                    /// <seealso cref= ATN#getExpectedTokens(int, RuleContext) </seealso>
                    virtual misc::IntervalSet *getExpectedTokens();

                    virtual misc::IntervalSet *getExpectedTokensWithinCurrentRule();

                    /// <summary>
                    /// Get a rule's index (i.e., {@code RULE_ruleName} field) or -1 if not found. </summary>
                    virtual int getRuleIndex(const std::wstring &ruleName);

                    virtual ParserRuleContext *getRuleContext();

                    /// <summary>
                    /// Return List&lt;String&gt; of the rule names in your parser instance
                    ///  leading up to a call to the current rule.  You could override if
                    ///  you want more details such as the file/line info of where
                    ///  in the ATN a rule is invoked.
                    /// 
                    ///  This is very useful for error messages.
                    /// </summary>
                    virtual std::vector<std::wstring> getRuleInvocationStack();

                    virtual std::vector<std::wstring> getRuleInvocationStack(RuleContext *p);

                    /// <summary>
                    /// For debugging and other purposes. </summary>
                    virtual std::vector<std::wstring> getDFAStrings();

                    /// <summary>
                    /// For debugging and other purposes. </summary>
                    virtual void dumpDFA();

                    virtual std::string getSourceName();

                    /// <summary>
                    /// During a parse is sometimes useful to listen in on the rule entry and exit
                    ///  events as well as token matches. This is for quick and dirty debugging.
                    /// </summary>
                    virtual void setTrace(bool trace);

                private:
                    void InitializeInstanceFields();
                };

            }
        }
    }
}
