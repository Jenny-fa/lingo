// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/error.hpp"

#include <iostream>
#include <stdexcept>
#include <stack>

namespace lingo
{

std::ostream&
operator<<(std::ostream& os, Diagnostic_kind k)
{
  switch (k) {
  case error_diag: return os << "error";
  case warning_diag: return os << "warning";
  case note_diag: return os << "note";
  
  default:
    lingo_unreachable("unknown diagnostic kind '{}'", (int)k);
  }
}


std::ostream&
operator<<(std::ostream& os, Diagnostic const& diag)
{
  return os << diag.kind << ':' << diag.loc << ": " << diag.msg;
}


namespace
{

// The diagnostic stack.
std::stack<Diagnostic_context*> diags_;

} // namespace


Diagnostic_context::Diagnostic_context(bool suppress)
  : suppress_(suppress), errs_(0)
{
  diags_.push(this);
}


Diagnostic_context::~Diagnostic_context()
{
  diags_.pop();
}


// Emit a single diagnostic. If this context is suppressing
// diagnostics, then the save them for later.
void 
Diagnostic_context::emit(Diagnostic const& diag)
{
  if (diag.kind == error_diag)
    ++ errs_;
  if (suppress_)
    push_back(diag);
  else
    std::cerr << diag << '\n';
}


// Reset the diagnotic context to a prinstine state.
void
Diagnostic_context::reset()
{
  clear();
  errs_ = 0;
}


// Emit all saved diagnostics. This does nothing if
// the context is not supporessing diagnostics.
void
Diagnostic_context::emit()
{
  if (suppress_)
    for (Diagnostic const& diag : *this)
      std::cerr << diag << '\n';
}


// Initialize the root diagnostic context.
void
init_diagnostics()
{
  static Diagnostic_context root_;
}


// Print all sved diagnostics. This is useful for
// replaying diagnostcs when suppressed.
void
emit_diagnostics()
{
  diags_.top()->emit();
}


// Rest the diagnotic context to a pristine state.
void
reset_diagnostics()
{
  diags_.top()->reset();
}


// Return the number of errors in the current diagnostic
// context.
int
error_count()
{
  return diags_.top()->errors();
}


// Construct a new error diagnostic for the given source
// location.
void
error(Bound_location loc, String const& msg)
{
  diags_.top()->emit({error_diag, loc, msg});
}


// Construct a new warning diagnostic for the given source
// location.
//
// TODO: Generate errors if warnings are being treated as
// errors, with some additional information about how to
// turn those off.
void
warning(Bound_location loc, String const& msg)
{
  diags_.top()->emit({warning_diag, loc, msg});
}


} // namespace lingo
