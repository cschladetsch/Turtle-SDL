// Copyright 2020 christian@schladetsch.com

#pragma once

#include "Processor/EToken.hpp"
#include "Processor/StringSplice.hpp"

namespace Turtle1 { namespace Processor {
struct Token {
    EToken Type;
    StringSplice Splice;

    explicit Token(EToken type = EToken::None) : Type(type) { }
    Token(EToken type, StringSplice splice) : Token(type) {
        Splice = splice;
    }

    friend bool operator==(const Token& left, const Token& right) {
        // note: we only compare the token types
        return left.Type == right.Type;
    }
};
}  // namespace Processor
}  // namespace Turtle1

