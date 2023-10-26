#pragma once
#include <string>
#include <deque>
#include <format>
#include <fstream>
#include <vector>
#include <cctype>
#include <iostream>
#include <stack>
#include <optional>
#include "Types.hpp"
using namespace std;
using namespace Loco::Types;

namespace Loco{
    namespace Lexer {
        #define ENUM_TO_STR(ENUM) std::string(#ENUM)

        bool string_contains_char(string s, char c) {
            return s.find(c) != std::string::npos;
        }

        struct SourceLocation {
        public:
            string filename;
            int lineNum;
            int colNum;

            SourceLocation() {
                this->filename = "";
                this->lineNum = 1;
                this->colNum = 1;
            }
            SourceLocation(string filename, int lineNum = 1, int colNum = 1) {
                this->filename = filename;
                this->lineNum = lineNum;
                this->colNum = colNum;
            }
            SourceLocation(SourceLocation& other) {
                this->filename = other.filename;
                this->lineNum = other.lineNum;
                this->colNum = other.colNum;
            }

            friend bool operator==(const SourceLocation& a, const SourceLocation& b) {
                return a.filename == b.filename;
            }

            string to_string() {
                return format("SourceLoc: ({},{},{})", this->filename, this->lineNum, this->colNum);
            }
        };

        enum class KeywordType {
            int_ = 0,
            float_ = 1,
            string_ = 2,
            bool_ = 3,
            import_ = 4,
            for_,
            while_,
            def,
            return_,
            if_,
            else_,
            elif_,
            and_,
            or_
        };

        struct Token {
            public:
                enum class Type {
                    eof = 1, // end of file (stop)
                    eol = 2, // end of line
                    indentation = 3, // indentation (tab)
                    identifier = 4, // identifier (name of variable,func,..)
                    keyword = 5, // reserved keyword
                    string = 6, // string of characters
                    symbol = 7, // non-alphanumeric character
                    number = 6 // number (int, float)
                };

                Token(){}
                Token(Type type, SourceLocation loc, Loco::Types::LType* value) {
                    this->type = type;
                    this->location = loc;
                    this->value = value;
                }

                string type_to_string() {
                    return ENUM_TO_STR(this->type);
                }
                static Type string_to_type(string s) {
                    throw;
                }

                Type type;
                Loco::Types::LType* value;
                SourceLocation location;
        };

        struct InputStream {
        public:
            ifstream stream;
            SourceLocation location;
            optional<char> saved_char;
            SourceLocation savedLocation;
            stack<Token> savedTokens;
            int tabulations;


            InputStream(string filename, SourceLocation& loc, int tabs = 4) {
                this->stream.open(filename);
                this->location = loc;
                this->tabulations = tabs;
            }
            InputStream(string filename, int tabs = 4) {
                this->stream.open(filename);
                this->location = SourceLocation(filename);
                this->tabulations = tabs;
            }

            /// <summary>
            /// Updates the position of the stream reader.
            /// The method only updates the row and column values, it doesn't return anything.
            /// </summary>
            void UpdatePosition(optional<char> c) {
                if (!c.has_value()) {
                    return;
                }
                if (c.value() == '\n') {
                    this->location.lineNum += 1;
                    this->location.colNum = 1;
                }
                else if (c.value() == '\t') {
                    this->location.colNum += this->tabulations;
                }
                else {
                    this->location.colNum += 1;
                }
            }

            /// <summary>
            /// Read the next character of the stream.
            /// </summary>
            /// <returns>An option between none and the read character.</returns>
            optional<char> ReadChar() {
                optional<char> c;
                if (this->saved_char.has_value()) {
                    c = saved_char;
                    this->saved_char = optional<char>();
                }
                else {
                    c = this->stream.get();
                }
                this->savedLocation = this->location;
                this->UpdatePosition(c);
                return c;
            }

            /// <summary>
            /// Pushes a character back to the stream.
            /// </summary>
            void UnreadChar(char c) {
                this->saved_char = optional<char>(c);
                this->location = this->savedLocation;
            }


            void SkipWhitespacesAndComments() {
                string whitespaces = " \t\n\r";
                std::vector<char> s {'\r','\n' };
                char c = this->ReadChar().value();
                while (string_contains_char(whitespaces, c) || c == '#') {
                    // Found
                    if (c == '#') {
                        while (!this->stream.get() && !this->stream.eof()) {
                            ;
                        }
                    }
                    c = this->ReadChar().value();
                    if (this->stream.eof() || c == '\x00') {
                        return;
                    }
                    this->UnreadChar(c);
                }
            }

            Token ParseStringToken(SourceLocation tokenLocation) {
                string token = "";
                while (true) {
                    auto c = this->ReadChar();
                    if (!c.has_value()) {
                        throw; // Add exception
                    }
                    if (c.value() == '"') {
                        break;
                    }
                    token = token + c.value();
                }
                return Token(Token::Type::string, tokenLocation, new LString(token));
            }

            Token ParseKeywordOrIdentifierToken(char firstChar, SourceLocation tokenLocation) {
                string token = std::to_string(firstChar);
                while (true) {
                    auto c = this->ReadChar().value();
                    if (!(std::isalnum(c) || c == '_')) {
                        this->UnreadChar(c);
                        break;
                    }
                    token = token + c;
                }
                try {
                    return Token(Token::Type::keyword, tokenLocation, new LString(token));
                }
                catch(exception e){
                    return Token(Token::Type::identifier, tokenLocation, new LString(token));
                }
            }

            Token ParseFloatToken(char firstChar, SourceLocation tokenLocation) {
                string token = std::to_string(firstChar);
                string digits = "1234567890";
                string e = "eE";
                while (true) {
                    auto c = this->ReadChar().value();
                    if (!(string_contains_char(digits, c) || c == '.' || e.find(c))) {
                        this->UnreadChar(c);
                        break;
                    }
                    token = token + c;
                }
                float value;
                try {
                    value = strtof(token.c_str(), NULL);
                }
                catch (exception e) {
                    throw; // Add exception
                }
                return Token(Token::Type::number, tokenLocation, new LFloat(value));
            }

            Token PeekToken() {
                string symbols = "()[],*<>=";
                string digits = "1234567890";
                string symbols2 = "+-.";
                this->SkipWhitespacesAndComments();
                auto oc = this->ReadChar();
                Token token;
                if (!oc.has_value() || oc.value() == '\x00') {
                    // No more characters left in the file
                    token = Token(Token::Type::eof, this->location, new LString(""));
                }
                SourceLocation loc(this->location);

                char c = oc.value();
                if (string_contains_char(symbols, c)) {
                    token = Token(Token::Type::symbol, loc, new LChar(c));
                }
                else if (c == '"') {
                    token = Token(Token::Type::string, loc, new LChar(c));
                }
                else if (string_contains_char(digits, c) || string_contains_char(symbols2, c)) {
                    token = this->ParseFloatToken(c, loc);
                }
                else if (std::isalnum(c) || c == '_') {
                    token = this->ParseKeywordOrIdentifierToken(c, loc);
                }
                else {
                    throw; // Add exception -> invalid char
                }
                this->UnreadChar(c);
                return token;
            }

            Token ReadToken() {
                string symbols = "()[],*<>=";
                string digits = "1234567890";
                string symbols2 = "+-.";
                this->SkipWhitespacesAndComments();
                auto oc = this->ReadChar();
                Token token;
                if (!oc.has_value() || oc.value() == '\x00') {
                    // No more characters left in the file
                    token = Token(Token::Type::eof, this->location, new LString(""));
                }
                SourceLocation loc(this->location);
           
                char c = oc.value();
                if (string_contains_char(symbols, c)) {
                    token = Token(Token::Type::symbol, loc, new LChar(c));
                }
                else if (c == '"') {
                    token = Token(Token::Type::string, loc, new LChar(c));
                }
                else if (string_contains_char(digits, c) || string_contains_char(symbols2, c)) {
                    token = this->ParseFloatToken(c, loc);
                }
                else if (std::isalnum(c) || c == '_') {
                    token = this->ParseKeywordOrIdentifierToken(c, loc);
                }
                else {
                    throw; // Add exception -> invalid char
                }
                return token;
            }

            void UnreadToken(Token token) {
                this->savedTokens.push(token);
            }
        };
    }
}

