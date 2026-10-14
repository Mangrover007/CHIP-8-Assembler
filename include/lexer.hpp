#pragma once

#include <string>
#include <unordered_map>

#include <vector>

enum class TOKEN
{
    // misc, i don't need this in the modified token stream
    LABEL,
    COMMENT,

    // misc but still need this in the modified token stream
    STR,	// to store sprites because i cant be bothered to do it otherwise
		// note that only 8 bit lists are allowed as a consequence
		// because only 8 bit sprites exist in CHIP-8

    // i need all of these in the modified token stream
    REG, 	// registers
    IDX,	// index register
    IDX_DEREF,	// for instructions like Fx55: LD [I], Vx
    LABEL_DEREF,	// what the fuck is this doing?

    // variables and lists
    VAR,	// example: %var
    VAR_DEREF,  // example: %var%

    LIST,	// example: $list
    LIST_DEREF, // example: $list$
    LP,		// left parenthesis
    RP,		// right parenthesis

    // expression tokens
    ASSIGN, 	// assignment operator '='
    LS, 	// left square bracket
    RS, 	// right square bracket
    MATH_OP,
    NUM,

    // instructions
    CLS,
    DRW,

    // partially implemented into the generator / parser
    ADD,
    LD,
    JMP,

    // not implemented into the generator / parser yet
    RET,
    CALL,
    SE,
    SNE,
    OR,
    AND,
    XOR,
    SUB,
    SHR,
    SUBN,
    SHL,
    RND,
    SKP,
    SKNP,

    EOFF,
};


class Lexer
{
public:
    Lexer(const char* path);
    ~Lexer();
    std::vector<std::pair<TOKEN, std::string>> lex();

private:
    void init();
    std::pair<TOKEN, std::string> nextToken();

    std::unordered_map<char, int> stoi;
    std::string parseNum(const std::string& num);

private:
    char* buf;
    int size;
    std::unordered_map<std::string, TOKEN> keywords;

    std::vector<std::pair<TOKEN, std::string>> tokstream;

    int i; // read pointer

    TOKEN tokFromWord(const std::string& word);

    // whitespace or ',' between instruction args or ':' after
    // a label
    void removeWhitespaces();
};

