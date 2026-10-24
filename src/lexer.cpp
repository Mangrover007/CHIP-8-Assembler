#include "../include/lexer.hpp"

#include <fstream>
#include <filesystem>

#include <iostream>


bool isexprchar(const char& c);
bool ishex(const char& c);

// yanked this function directly from Generator::parseNum
std::string Lexer::parseNum(const std::string& num)
{
    int ans = 0;
    if (num.length() >= 2)
    {
	if (num[1] == 'x')
	{
	    int h = 1;
	    for (int d = num.length() - 1; d >= 2; d--)
	    {
		// for now just this it cant parse A,B,C,D,E,F
		int mult = stoi[num[d]];
		ans |= h * mult;
		h <<= 4u;
	    }
	}
	else if (num[1] == 'b')
	{
	    int b = 1;
	    for (int d = 2; d < num.length(); d++)
	    {
		if (num[d] == '1')
		{
		    ans |= (1 << (num.length() - 2 - b));
		}
		b++;
	    }
	}
	else
	{
	    ans = std::stoi(num);
	}
    }
    else
    {
	ans = std::stoi(num);
    }

    return std::to_string(ans);
}



void Lexer::init()
{
    // CHIP-8 instructions
    keywords.insert({"CLS", TOKEN::CLS});
    keywords.insert({"RET", TOKEN::RET});
    keywords.insert({"JMP", TOKEN::JMP});
    keywords.insert({"CALL", TOKEN::CALL});
    keywords.insert({"ADD", TOKEN::ADD});
    keywords.insert({"SE", TOKEN::SE});
    keywords.insert({"SNE", TOKEN::SNE});
    keywords.insert({"LD", TOKEN::LD});
    keywords.insert({"OR", TOKEN::OR});
    keywords.insert({"AND", TOKEN::AND});
    keywords.insert({"XOR", TOKEN::XOR});
    keywords.insert({"SUB", TOKEN::SUB});

    keywords.insert({"SHR", TOKEN::SHR});
    keywords.insert({"SUBN", TOKEN::SUBN});
    keywords.insert({"SHL", TOKEN::SHL});

    keywords.insert({"RND", TOKEN::RND});
    keywords.insert({"DRW", TOKEN::DRW});
    keywords.insert({"SKP", TOKEN::SKP});
    keywords.insert({"SKNP", TOKEN::SKNP});

    // registers
    keywords.insert({"V0", TOKEN::REG});
    keywords.insert({"V1", TOKEN::REG});
    keywords.insert({"V2", TOKEN::REG});
    keywords.insert({"V3", TOKEN::REG});
    keywords.insert({"V4", TOKEN::REG});
    keywords.insert({"V5", TOKEN::REG});
    keywords.insert({"V6", TOKEN::REG});
    keywords.insert({"V7", TOKEN::REG});
    keywords.insert({"V8", TOKEN::REG});
    keywords.insert({"V9", TOKEN::REG});
    keywords.insert({"VA", TOKEN::REG});
    keywords.insert({"VB", TOKEN::REG});
    keywords.insert({"VC", TOKEN::REG});
    keywords.insert({"VD", TOKEN::REG});
    keywords.insert({"VE", TOKEN::REG});
    keywords.insert({"VF", TOKEN::REG});

    keywords.insert({"I", TOKEN::IDX});
    keywords.insert({"[I]", TOKEN::IDX_DEREF});
    keywords.insert({"DT", TOKEN::DELAY_TIMER});
    keywords.insert({"ST", TOKEN::SOUND_TIMER});
    
    // custom instructions
    keywords.insert({"STR", TOKEN::STR});
    keywords.insert({"RES", TOKEN::RES});

    stoi['0'] = 0;
    stoi['1'] = 1;
    stoi['2'] = 2;
    stoi['3'] = 3;
    stoi['4'] = 4;
    stoi['5'] = 5;
    stoi['6'] = 6;
    stoi['7'] = 7;
    stoi['8'] = 8;
    stoi['9'] = 9;
    stoi['A'] = 10;
    stoi['B'] = 11;
    stoi['C'] = 12;
    stoi['D'] = 13;
    stoi['E'] = 14;
    stoi['F'] = 15;
}


std::vector<std::pair<TOKEN, std::string>> Lexer::lex()
{
    while (true)
    {
	auto [token, value] = nextToken();

	if (token == TOKEN::COMMENT)
	{
	    continue;
	}

	if (token == TOKEN::NUM)
	{
#ifdef DEBUG_LEXER
	    std::cout << "Number: " << std::endl;
#endif
	    value = parseNum(value);
#ifdef DEBUG_LEXER
	    std::cout << "Number: " << std::endl;
#endif
	}

	tokstream.push_back({token, value});

#ifdef DEBUG_LEXER
	std::cout << "Pushing token: " << value << std::endl;
#endif

	if (token == TOKEN::EOFF)
	{
	    break;
	}
    }

#ifdef DEBUG_LEXER
    std::cout << "Returning token stream" << std::endl;
#endif

    return tokstream;
}


TOKEN Lexer::tokFromWord(const std::string& word)
{
    // instruction keywords like CLS, DRW, LD, etc.
    if (keywords.find(word) != keywords.end())
    {
	return keywords[word];
    }

    // number
    else if (isdigit(word[0]))
    {
	return TOKEN::NUM;
    }

    // label dereference
    else if (word[0] == '#')
    {
	return TOKEN::LABEL_DEREF;
    }

    // either variable or variable dereference
    else if (word[0] == '%')
    {
	if (word[word.length() - 1] == '%')
	{
	    return TOKEN::VAR_DEREF;
	}

	return TOKEN::VAR;
    }

    // either list or list dereference
    else if (word[0] == '$')
    {
	if (word[word.length() - 1] == '$')
	{
	    return TOKEN::LIST_DEREF;
	}

	return TOKEN::LIST;
    }

    // math expression tokens
    else if (word[0] == '=')
    {
	return TOKEN::ASSIGN;
    }
    else if (word[0] == '+' || word[0] == '*' || word[0] == '-' || word[0] == '/' || word[0] == '^' || word[0] == '|')
    {
	return TOKEN::MATH_OP;
    }
    else if (word[0] == '[')
    {
	return TOKEN::LS;
    }
    else if (word[0] == ']')
    {
	return TOKEN::RS;
    }
    else if (word[0] == '(')
    {
	return TOKEN::LP;
    }
    else if (word[0] == ')')
    {
	return TOKEN::RP;
    }

    return TOKEN::LABEL;
}


Lexer::Lexer(const char* path)
{
    init();

    i = 0;

    std::fstream asmFile(path, std::ios::in);
    asmFile.seekg(0, std::ios::beg);

    size = std::filesystem::file_size(path);

    buf = new char[size];

    asmFile.read(buf, size);

    asmFile.close();
}


Lexer::~Lexer()
{
    delete[] buf;
}


void Lexer::removeWhitespaces()
{
    // whitespace or ',' between instruction args or ':' after
    // a label
    int j = i;
    while (j < size && (isspace(buf[j]) || buf[j] == ':' || buf[j] == ','))
    {
	j++;
    }

    i = j;
}

std::pair<TOKEN, std::string> Lexer::nextToken()
{
#ifdef DEBUG_LEXER
    std::cout << "Parsing next token" << std::endl;
#endif
    if (i >= size)
    {
	return {TOKEN::EOFF, ""};
    }

    removeWhitespaces();
#ifdef DEBUG_LEXER
    std::cout << "Not a whitespace, not a newline, not a comment wtf: '" << buf[i] << "'" << std::endl;
#endif
    // ----------------------------------------------------------------- //
    // comments
    if (buf[i] == ';')
    {
	while (i < size && buf[i] != '\n')
	{
	    i++;
	}

#ifdef DEBUG_LEXER
	std::cout << "SPACE: " << isspace(buf[i]) << std::endl;
#endif
	return {TOKEN::COMMENT, ""};
    }

    // ----------------------------------------------------------------- //
    // dereference to labels
    else if (buf[i] == '#')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);

	j++;

	while (j < size && (isalnum(buf[j]) || buf[j] == '_'))
	{
	    word.push_back(buf[j]);
	    j++;
	}

	i = j;

	return {tokFromWord(word), word};
    }

    // ----------------------------------------------------------------- //
    // variable
    else if (buf[i] == '%')
    {
	// variable is a variable
	// doesn't matter if its getting declared or dereferenced right?
	int j = i;
	std::string word;
	word.push_back(buf[j]);
	j++;

	// example variable: %__varX2=<expr>

	if (isalpha(buf[j]) || buf[j] == '_')
	{
	    word.push_back(buf[j]);
	    j++;
	    while (j < size && (isalnum(buf[j]) || buf[j] == '_'))
	    {
		word.push_back(buf[j]);
		j++;
	    }

	    if (buf[j] == '%')
	    {
		word.push_back(buf[j]);
		j++;
	    }

	    i = j;

	    return {tokFromWord(word), word};
	}
	else
	{
	    // variable name must start with [a-zA-Z] or _
	}
    }

    // list
    else if (buf[i] == '$')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);
	j++;

	// example variable: %__varX2=<expr>

	if (isalpha(buf[j]) || buf[j] == '_')
	{
	    word.push_back(buf[j]);
	    j++;
	    while (j < size && (isalnum(buf[j]) || buf[j] == '_'))
	    {
		word.push_back(buf[j]);
		j++;
	    }

	    if (buf[j] == '$')
	    {
		word.push_back(buf[j]);
		j++;
	    }

	    i = j;

	    return {tokFromWord(word), word};
	}
	else
	{
	    // list name must start with [a-zA-Z] or _
	}
    }

    // ----------------------------------------------------------------- //
    // math expressions
    else if (buf[i] == '=')
    {
	int j = i;
	std::string word;

	word.push_back(buf[j]);
	j++;

	i = j;

	return {tokFromWord(word), word};
    }

    else if (buf[i] == '+' || buf[i] == '*' || buf[i] == '-' || buf[i] == '/' || buf[i] == '^' || buf[i] == '|')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);
	j++;
	i = j;
	return {tokFromWord(word), word};
    }

    else if (buf[i] == '[')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);

	j++;

	if (buf[j] == 'I' && buf[j + 1] == ']')
	{
	    word.push_back(buf[j]);
	    word.push_back(buf[j + 1]);

	    j += 2;
	}

	i = j;

	return {tokFromWord(word), word};
    }

    else if (buf[i] == ']')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);

	j++;

	i = j;

	return {tokFromWord(word), word};
    }


    else if (buf[i] == '(' || buf[i] == ')')
    {
	int j = i;
	std::string word;
	word.push_back(buf[j]);

	j++;

	i = j;

	return {tokFromWord(word), word};
    }

    // ----------------------------------------------------------------- //

    // label or instruction
    else if (isalpha(buf[i]))
    {
	// parses the word for what it is, then compares it to any
	// existing keywords
	int j = i;
	std::string word;
	while (j < size && (isalnum(buf[j]) || buf[j] == '_'))
	{
	    word.push_back(buf[j]);
	    j++;
	}

	i = j;

#ifdef DEBUG_LEXER
	std::cout << "what kind of label is this? '" << word << "'" << std::endl;
#endif

	return {tokFromWord(word), word};
    }

    // ----------------------------------------------------------------- //
    // number
    else if (isdigit(buf[i]) || buf[i] == '-')
    {
	int j = i;
	std::string number;
	while (j < size && isdigit(buf[j]))
	{
	    number.push_back(buf[j]);
	    j++;
	}

	if (buf[j] == 'x')
	{
	    number.push_back(buf[j]);
	    j++;
	    while (j < size && ishex(buf[j]))
	    {
#ifdef DEBUG_LEXER
		std::cout << "Pushing: " << buf[j] << std::endl;
#endif
		number.push_back(buf[j]);
		j++;
	    }
	}
	else if (buf[j] == 'b')
	{
	    number.push_back(buf[j]);
	    j++;
	    while (j < size && isdigit(buf[j]))
	    {
		number.push_back(buf[j]);
		j++;
	    }
	}

	i = j;

	return {tokFromWord(number), number};
    }

    // ----------------------------------------------------------------- //
    
    return {TOKEN::EOFF, ""};
}

bool ishex(const char& c)
{
    if (isdigit(c))
    {
	return true;
    }

    switch(c)
    {
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	    return true;
	default:
	    return false;
    }

    return false;
}

