﻿#pragma once

#include "Declarations.h"
#include "MultiMap.h"
#include "Token.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>


/*
 * [The "BSD license"]
 * Copyright (c) 2013 Terence Parr
 * Copyright (c) 2013 Dan McLaughlin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

namespace org {
    namespace antlr {
        namespace v4 {
            namespace runtime {
                namespace tree {
                    namespace pattern {

                        /// <summary>
                        /// A tree pattern matching mechanism for ANTLR <seealso cref="ParseTree"/>s.
                        /// <p/>
                        /// Patterns are strings of source input text with special tags representing
                        /// token or rule references such as:
                        /// <p/>
                        /// {@code <ID> = <expr>;}
                        /// <p/>
                        /// Given a pattern start rule such as {@code statement}, this object constructs
                        /// a <seealso cref="ParseTree"/> with placeholders for the {@code ID} and {@code expr}
                        /// subtree. Then the <seealso cref="#match"/> routines can compare an actual
                        /// <seealso cref="ParseTree"/> from a parse with this pattern. Tag {@code <ID>} matches
                        /// any {@code ID} token and tag {@code <expr>} references the result of the
                        /// {@code expr} rule (generally an instance of {@code ExprContext}.
                        /// <p/>
                        /// Pattern {@code x = 0;} is a similar pattern that matches the same pattern
                        /// except that it requires the identifier to be {@code x} and the expression to
                        /// be {@code 0}.
                        /// <p/>
                        /// The <seealso cref="#matches"/> routines return {@code true} or {@code false} based
                        /// upon a match for the tree rooted at the parameter sent in. The
                        /// <seealso cref="#match"/> routines return a <seealso cref="ParseTreeMatch"/> object that
                        /// contains the parse tree, the parse tree pattern, and a map from tag name to
                        /// matched nodes (more below). A subtree that fails to match, returns with
                        /// <seealso cref="ParseTreeMatch#mismatchedNode"/> set to the first tree node that did not
                        /// match.
                        /// <p/>
                        /// For efficiency, you can compile a tree pattern in string form to a
                        /// <seealso cref="ParseTreePattern"/> object.
                        /// <p/>
                        /// See {@code TestParseTreeMatcher} for lots of examples.
                        /// <seealso cref="ParseTreePattern"/> has two static helper methods:
                        /// <seealso cref="ParseTreePattern#findAll"/> and <seealso cref="ParseTreePattern#match"/> that
                        /// are easy to use but not super efficient because they create new
                        /// <seealso cref="ParseTreePatternMatcher"/> objects each time and have to compile the
                        /// pattern in string form before using it.
                        /// <p/>
                        /// The lexer and parser that you pass into the <seealso cref="ParseTreePatternMatcher"/>
                        /// constructor are used to parse the pattern in string form. The lexer converts
                        /// the {@code <ID> = <expr>;} into a sequence of four tokens (assuming lexer
                        /// throws out whitespace or puts it on a hidden channel). Be aware that the
                        /// input stream is reset for the lexer (but not the parser; a
                        /// <seealso cref="ParserInterpreter"/> is created to parse the input.). Any user-defined
                        /// fields you have put into the lexer might get changed when this mechanism asks
                        /// it to scan the pattern string.
                        /// <p/>
                        /// Normally a parser does not accept token {@code <expr>} as a valid
                        /// {@code expr} but, from the parser passed in, we create a special version of
                        /// the underlying grammar representation (an <seealso cref="ATN"/>) that allows imaginary
                        /// tokens representing rules ({@code <expr>}) to match entire rules. We call
                        /// these <em>bypass alternatives</em>.
                        /// <p/>
                        /// Delimiters are {@code <} and {@code >}, with {@code \} as the escape string
                        /// by default, but you can set them to whatever you want using
                        /// <seealso cref="#setDelimiters"/>. You must escape both start and stop strings
                        /// {@code \<} and {@code \>}.
                        /// </summary>
                        class ParseTreePatternMatcher {
                        public:
                            class CannotInvokeStartRule : public std::exception {
                            public:
                                CannotInvokeStartRule(std::exception e);
                            };

                            /// <summary>
                            /// This is the backing field for <seealso cref="#getLexer()"/>.
                            /// </summary>
                        private:
                            Lexer *const lexer;

                            /// <summary>
                            /// This is the backing field for <seealso cref="#getParser()"/>.
                            /// </summary>
                            Parser *const parser;

                        protected:
                            std::wstring start;
                            std::wstring stop;
                            std::wstring escape; // e.g., \< and \> must escape BOTH!

                            /// <summary>
                            /// Constructs a <seealso cref="ParseTreePatternMatcher"/> or from a <seealso cref="Lexer"/> and
                            /// <seealso cref="Parser"/> object. The lexer input stream is altered for tokenizing
                            /// the tree patterns. The parser is used as a convenient mechanism to get
                            /// the grammar name, plus token, rule names.
                            /// </summary>
                        public:
                            ParseTreePatternMatcher(Lexer *lexer, Parser *parser);

                            /// <summary>
                            /// Set the delimiters used for marking rule and token tags within concrete
                            /// syntax used by the tree pattern parser.
                            /// </summary>
                            /// <param name="start"> The start delimiter. </param>
                            /// <param name="stop"> The stop delimiter. </param>
                            /// <param name="escapeLeft"> The escape sequence to use for escaping a start or stop delimiter.
                            /// </param>
                            /// <exception cref="IllegalArgumentException"> if {@code start} is {@code null} or empty. </exception>
                            /// <exception cref="IllegalArgumentException"> if {@code stop} is {@code null} or empty. </exception>
                            virtual void setDelimiters(const std::wstring &start, const std::wstring &stop, const std::wstring &escapeLeft);

                            /// <summary>
                            /// Does {@code pattern} matched as rule {@code patternRuleIndex} match {@code tree}? </summary>
                            virtual bool matches(ParseTree *tree, const std::wstring &pattern, int patternRuleIndex);

                            /// <summary>
                            /// Does {@code pattern} matched as rule patternRuleIndex match tree? Pass in a
                            ///  compiled pattern instead of a string representation of a tree pattern.
                            /// </summary>
                            virtual bool matches(ParseTree *tree, ParseTreePattern *pattern);

                            /// <summary>
                            /// Compare {@code pattern} matched as rule {@code patternRuleIndex} against
                            /// {@code tree} and return a <seealso cref="ParseTreeMatch"/> object that contains the
                            /// matched elements, or the node at which the match failed.
                            /// </summary>
                            virtual ParseTreeMatch *match(ParseTree *tree, const std::wstring &pattern, int patternRuleIndex);

                            /// <summary>
                            /// Compare {@code pattern} matched against {@code tree} and return a
                            /// <seealso cref="ParseTreeMatch"/> object that contains the matched elements, or the
                            /// node at which the match failed. Pass in a compiled pattern instead of a
                            /// string representation of a tree pattern.
                            /// </summary>
                            virtual ParseTreeMatch *match(ParseTree *tree, ParseTreePattern *pattern);

                            /// <summary>
                            /// For repeated use of a tree pattern, compile it to a
                            /// <seealso cref="ParseTreePattern"/> using this method.
                            /// </summary>
                            virtual ParseTreePattern *compile(const std::wstring &pattern, int patternRuleIndex);

                            /// <summary>
                            /// Used to convert the tree pattern string into a series of tokens. The
                            /// input stream is reset.
                            /// </summary>
                            virtual Lexer *getLexer();

                            /// <summary>
                            /// Used to collect to the grammar file name, token names, rule names for
                            /// used to parse the pattern into a parse tree.
                            /// </summary>
                            virtual Parser *getParser();

                            // ---- SUPPORT CODE ----

                            /// <summary>
                            /// Recursively walk {@code tree} against {@code patternTree}, filling
                            /// {@code match.}<seealso cref="ParseTreeMatch#labels labels"/>.
                            /// </summary>
                            /// <returns> the first node encountered in {@code tree} which does not match
                            /// a corresponding node in {@code patternTree}, or {@code null} if the match
                            /// was successful. The specific node returned depends on the matching
                            /// algorithm used by the implementation, and may be overridden. </returns>
                        protected:
                            // I don't know why this is failing to compile
                          virtual ParseTree *matchImpl(ParseTree *tree, ParseTree *patternTree, misc::MultiMap<std::wstring, ParseTree*> *labels);
                            /// <summary>
                            /// Is {@code t} {@code (expr <expr>)} subtree? </summary>
                            virtual RuleTagToken *getRuleTagToken(ParseTree *t);

                        public:
                            virtual std::vector<Token> tokenize(const std::wstring &pattern);

                            /// <summary>
                            /// Split {@code <ID> = <e:expr> ;} into 4 chunks for tokenizing by <seealso cref="#tokenize"/>. </summary>
                            virtual std::vector<Chunk*> split(const std::wstring &pattern);

                        private:
                            void InitializeInstanceFields();
                        };

                    }
                }
            }
        }
    }
}
