// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef LINGO_TOKEN_HPP
#define LINGO_TOKEN_HPP

#include "lingo/location.hpp"
#include "lingo/symbol.hpp"
#include "lingo/string.hpp"
#include "lingo/integer.hpp"
#include "lingo/print.hpp"
#include "lingo/debug.hpp"

#include <cstdint>
#include <vector>

namespace lingo
{

struct Symbol;

// -------------------------------------------------------------------------- //
//                              Token kinds
//
// Note that the token spellings below are suggestions. The 
// actual spelling of tokens is determined by the lexer.

// The Token_kind enumerates the different kinds of lexemes 
// that can appear in the source language. 
using Token_kind = int;

constexpr Token_kind error_tok      = 0; // not a valid token

// Common punctuation tokens
constexpr Token_kind lparen_tok     = 1;  // (
constexpr Token_kind rparen_tok     = 2;  // )
constexpr Token_kind lbrace_tok     = 3;  // {
constexpr Token_kind rbrace_tok     = 4;  // }
constexpr Token_kind lbrack_tok     = 5;  // ]
constexpr Token_kind rbrack_tok     = 6;  // [

constexpr Token_kind dot_tok        = 7;  // .
constexpr Token_kind comma_tok      = 8;  // ,
constexpr Token_kind semicolon_tok  = 9;  // ;
constexpr Token_kind colon_tok      = 10; // :
constexpr Token_kind equal_tok      = 11; // =
constexpr Token_kind plus_tok       = 12; // +
constexpr Token_kind minus_tok      = 13; // -
constexpr Token_kind star_tok       = 14; // *
constexpr Token_kind slash_tok      = 15; // /
constexpr Token_kind percent_tok    = 16; // %
constexpr Token_kind amp_tok        = 17; // &
constexpr Token_kind bar_tok        = 18; // |
constexpr Token_kind caret_tok      = 19; // ^
constexpr Token_kind tilde_tok      = 20; // ~
constexpr Token_kind bang_tok       = 21; // !
constexpr Token_kind lt_tok         = 22; // <
constexpr Token_kind gt_tok         = 23; // >

constexpr Token_kind minus_gt_tok   = 24; // ->
constexpr Token_kind eq_gt_tok      = 25; // =>
constexpr Token_kind lt_lt_tok      = 26; // <<
constexpr Token_kind gt_gt_tok      = 27; // >>
constexpr Token_kind eq_eq_tok      = 28; // ==
constexpr Token_kind bang_eq_tok    = 29; // !=
constexpr Token_kind lt_eq_tok      = 30; // <=
constexpr Token_kind gt_eq_tok      = 31; // >=
constexpr Token_kind amp_amp_tok    = 32; // &&
constexpr Token_kind bar_bar_tok    = 33; // ||
constexpr Token_kind dot_dot_tok    = 34; // ..

// Common value classes
constexpr Token_kind identifier_tok          = 50; // identifiers (like C)
constexpr Token_kind boolean_tok             = 51; // true | false
constexpr Token_kind binary_integer_tok      = 52; // 0b[:binary-digit:]*
constexpr Token_kind decimal_integer_tok     = 53; // [:decimal-digit:]*
constexpr Token_kind octal_integer_tok       = 54; // 0o[:octal-digit:]*
constexpr Token_kind hexadecimal_integer_tok = 55; // 0x[:hexadecimal-digit]*


char const* get_token_name(Token_kind);
char const* get_token_spelling(Token_kind);


// Returns true if `k` is one of the integer tokens.
inline bool
is_integer(Token_kind k)
{
  return binary_integer_tok <= k && k <= hexadecimal_integer_tok;
}


std::ostream& operator<<(std::ostream&, Token_kind);


// -------------------------------------------------------------------------- //
//                            Token class

// The Token class represents the occurrence of a lexeme  within a 
// source file. It associates the class of the the lexeme with its 
// associated value (if any) and its location in the source file.
//
// Each token indexes an entry in the symbol table, which stores 
// additional attributes associated with the token  (e.g. scope 
// bindings, numeric interpretation of values, etc.).
//
// A token whose kind is error_tok is not a valid token. This can be 
// used as a method for determining when a matching  algorithm fails 
// to match a sequence of characters.
class Token
{
public:
  // Construct an error token.
  Token()
    : loc_(), kind_(error_tok)
  { }

  Token(Location, char const*, int);
  Token(Location, char const*, char const*);

  Token(Location, Token_kind, char const*, int);
  Token(Location, Token_kind, char const*, char const*);

  Token(Location, Symbol&);

  // Observers
  char const*   token_name() const     { return get_token_name(kind_); }
  char const*   token_spelling() const { return get_token_spelling(kind_); }
  Location      location() const       { return loc_; }
  Token_kind    kind() const           { return kind_; }
  
  // Symbol/text representation
  Symbol const& symbol() const { return *sym_; }
  String const& str() const    { return symbol().str; }
  String const* ptr() const    { return &str(); }

  // Contextually convertible to bool.
  // True when this is not an error token.
  explicit operator bool() const { return kind_ != error_tok; }

  // Token builders.
  static Token make_identifier(Location loc, Symbol& sym);
  static Token make_integer(Location loc, Symbol& sym, Token_kind);

private:
  Location   loc_;
  Token_kind kind_;
  Symbol*    sym_;
};


// A sequence of tokens.
using Token_list = std::vector<Token>;


// -------------------------------------------------------------------------- //
//                              Elaboration

bool    as_boolean(Token);
Integer as_integer(Token);
String  as_string(Token);


// Returns true if the token is an integer.
inline bool
is_integer(Token const& k)
{
  return is_integer(k.kind());
}


// -------------------------------------------------------------------------- //
//                              Printing

void print(Printer&, Token);
void debug(Printer&, Token);

std::ostream& operator<<(std::ostream&, Token);


// -------------------------------------------------------------------------- //
//                              Token stream


// A token stream provides a sequence of tokens and has a very 
// simple streaming interface consisting of only 5 functions:
// peek(), get(), and eof(), begin(), and end(). Character streams
// are the input to lexical analyzers.
class Token_stream
{
public:
  using value_type = Token;

  Token_stream(Token const* f, Token const* l)
    : first_(f), last_(l)
  { }

  Token_stream(Token_list const& toks)
    : Token_stream(toks.data(), toks.data() + toks.size())
  { }

  // Stream control
  bool eof() const { return first_ == last_; }
  Token const& peek() const;
  Token        peek(int) const;
  Token const& get();

  // Iterators
  Token const* begin()       { return first_; }
  Token const* begin() const { return first_; }
  Token const* end()       { return last_; }
  Token const* end() const { return last_; }

  // Returns the source location of the the current token.
  Location location() { return eof() ? Location{} : peek().location(); }

  Token const* first_; // Current character pointer
  Token const* last_;  // Past the end of the character buffer
};


void debug(Printer&, Token_stream const&);


// -------------------------------------------------------------------------- //
//                              Token sets

// A token set defines hooks between the general token
// processing faclities in lingo and those in client
// languages.
//
// TODO: Should we allow multiple token sets? That would
// effectively imply that we partition token values values
// among applications and that there could be NO overlapping
// values.
struct Token_set
{
  virtual ~Token_set() { }

  virtual char const* token_name(Token_kind) const = 0;
  virtual char const* token_spelling(Token_kind) const = 0;
};


void install_token_set(Token_set&);
void uninstall_token_set(Token_set&);

void install_token(char const*, Token_kind);
void install_tokens(std::initializer_list<std::pair<char const*, Token_kind>>);

} // namespace lingo


#endif
