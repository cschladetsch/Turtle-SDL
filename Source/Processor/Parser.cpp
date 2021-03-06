// Copyright � 2020 christian@schladetsch.com

#include "Processor/Pch.hpp"
#include "Processor/Parser.hpp"

namespace TurtleGraphics::Processor {

Parser::Parser(const Lexer& lexer) {
    SetLexer(lexer);
}

void Parser::SetLexer(const Lexer& lexer) {
    EnterNode(AstNode::New(EToken::Start));

    for (const auto& token : lexer.GetTokens()) {
        if (token.Type != EToken::WhiteSpace)
            _tokens.push_back(token);
    }
}

bool Parser::Run(const Lexer& lexer) {
    SetLexer(lexer);
    return Run();
}

bool Parser::Run() {
    try {
        return ParseStatements();
    } catch (std::exception const &e) {
        Fail() << e.what();
        return false;
    }
}

bool Parser::ParseStatements() {
    while (ParseStatement()) {
        continue;
    }

    return HasSucceeded();
}

bool Parser::ParseStatement() {
    if (AtEnd()) {
        return false;
    }

    switch (CurrentTokenType()) {
    case EToken::PenDown: return AddChild(EToken::PenDown);
    case EToken::PenUp: return AddChild(EToken::PenUp);
    case EToken::Repeat: return ParseRepeat();
    case EToken::Rotate: return ParseRotate();
    case EToken::Move: return ParseMove();
    case EToken::Quit: return AddChild(EToken::Quit);
    case EToken::Function: return ParseFunction();
    case EToken::Number: return AddChild(CurrentToken());
    case EToken::Delta: return ParseDelta();
    default: {}
    }

    return false;
}

bool Parser::ParseRepeat() {
    if (!Peek(EToken::Number))
        return Fail("Number expected");

    EnterNode(AstNode::New(EToken::Repeat));
    AddChild(NextToken());

    if (!ParseStatementBlock())
        return false;

    LeaveNode();

    return true;
}

AstNodePtr Parser::GetRoot() const {
    if (_context.size() != 1) {
        Fail("Unbalanced parse tree");
        return nullptr;
    }

    return _context.front();
}

bool Parser::ParseFunction() {
    NextToken();

    const auto funName = CurrentToken();

    if (!Expect(EToken::Identifier)) {
        return Fail("KeyResponseFunction name expected");
    }

    auto fun = AstNode::New(EToken::Function);
    fun->AddChild(AstNode::New(funName));
    EnterNode(fun);

    if (!AddArguments()) {
        return Fail("Failed to parse arguments");
    }

    if (!AddStatementBlock()) {
        return Fail("Statement block expected");
    }

    LeaveNode();

    return true;
}

bool Parser::AddStatementBlock() {
    EnterNode(AstNode::New(EToken::StatementBlock));
    if (!ParseStatementBlock()) {
        return false;
    }

    LeaveNode();
    return true;
}

bool Parser::ParseColorName() {
    if (!Peek(EToken::Identifier)) {
        return Fail("Color Identifier expected");
    }
    // auto& changeColor = EnterNode(EToken::ColorName);

    // Use span instead (bounds.1).auto colorName = NextToken();
    // switch (colorName.Type)
    // {
    // case EToken::Red:
    // case EToken::Blue:
    // case EToken::Green:
    // }

    return Fail("Not implemented");
}

bool Parser::AddDelta(EToken type) {
    const auto what = CurrentToken();
    const auto num = NextToken();
    auto delta = AstNode::New(EToken::Delta);
    delta->AddChild(what);
    delta->AddChild(num);
    AddChild(delta);
    ++_currentToken;
    return true;
}

bool Parser::ParseDelta() {
    switch (NextToken().Type) {
    case EToken::Red:
        return AddDelta(EToken::Red);
    default: { }
    }

    return false;
}

bool Parser::AddArguments() {
    if (!Expect(EToken::OpenParan)) {
        Fail() << "Open parenthesis expected";
        return false;
    }

    const auto args = AstNode::New(EToken::ArgList);
    EnterNode(args);

    while (AddChild(EToken::Identifier)) {
        if (!Peek(EToken::Comma)) {
            break;
        }

        NextToken();
    }

    if (!Expect(EToken::CloseParan)) {
        _lexer->Fail() << "Close parenthesis expected";
        return false;
    }

    LeaveNode();

    return true;
}

void Parser::EnterNode(AstNodePtr const &node) {
    if (!_context.empty()) {
        _context.back()->AddChild(node);
    }

    _context.push_back(node);
}

void Parser::LeaveNode() {
    _context.pop_back();
}

bool Parser::ParseStatementBlock() {
    if (!Expect(EToken::OpenBrace))
        return false;

    if (!ParseStatements())
        return false;

    if (!Expect(EToken::CloseBrace))
        return false;

    return true;
}

Token Parser::NextToken() {
    if (AtEnd()) {
        Fail() << "Token expected";
        return Token();
    }
    return GetTokens().at(++_currentToken);
}

bool Parser::Expect(EToken type) {
    if (!CurrentTokenType(type)) {
        Fail() << "Expected " << type << ", got " << CurrentToken().Type;
        return false;
    }

    ++_currentToken;

    return true;
}

bool Parser::ParseRotate() {
    return AddParameterisedCommand(EToken::Rotate);
}

bool Parser::ParseMove() {
    return AddParameterisedCommand(EToken::Move);
}

bool Parser::AddParameterisedCommand(EToken type) {
    if (!Peek(EToken::Number)) {
        Fail() << "Number expected";
        return false;
    }

    auto node = AstNode::New(type);
    node->AddChild(AstNode::New(NextToken()));
    ++_currentToken;
    AddChild(node);
    return true;
}

bool Parser::ParseExpression() {
    return false;
}

bool Parser::AddChild(Token const &token) {
    ++_currentToken;
    return AddChild(AstNode::New(token));
}

bool Parser::AddChild(EToken type) {
    return AddChild(Token(type));
}

bool Parser::AddChild(AstNodePtr const &child) {
    _context.back()->AddChild(child);
    return true;
}

}  // namespace TurtleGraphics::Processor

