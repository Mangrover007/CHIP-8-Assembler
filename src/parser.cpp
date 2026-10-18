#include "../include/parser.hpp"

#include <iostream>
#include <stack>


bool priority(const std::string& op1, const std::string& op2);


Parser::Parser() : offset(0x200), j(0)
{
}


std::unordered_map<std::string, uint16_t> Parser::getOffsetTable()
{
    return offset_table;
}


std::unordered_map<std::string, int> Parser::getVarLookupTable()
{
    return var_lookup_table;
}


std::unordered_map<std::string, std::vector<uint8_t>> Parser::getListLookupTable()
{
    return list_lookup_table;
}


std::vector<std::pair<TOKEN, std::string>> Parser::parse(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    std::vector<std::pair<TOKEN, std::string>> mod_tokstream;

    while (j < tokstream.size())
    {
	auto [token, value] = tokstream[j];

	switch(token)
	{
	    case TOKEN::LABEL:
		offset_table.insert({value, offset});
		break;

	    case TOKEN::VAR:
		{
		    std::cout << "Parsing variable assignment" << std::endl;
		    // syntax:
		    // VAR ASSIGN EXPR
		    // EXPR starts with [ and ends with ]
		    j++;
		    if (tokstream[j].first == TOKEN::ASSIGN)
		    {
			j++;
			int exprval  = parseAssignment(tokstream);
			var_lookup_table.insert({value, exprval});
		    }
		    else
		    {
			// bad syntax
		    }
		}
		break;

	    case TOKEN::LIST:
		{
		    std::cout << "Parsing list assignment" << std::endl;
		    // syntax:
		    // LIST ASSIGN <val>
		    // <val> starts with ( and ends with )
		    j++;
		    if (tokstream[j].first == TOKEN::ASSIGN)
		    {
			j++;
			std::vector<uint8_t> listVal = parseListAssignment(tokstream);
			list_lookup_table.insert({value, listVal});
			std::cout << "After parsing the list, token is: " << tokstream[j].second << std::endl;
		    }
		}
		break;

	    case TOKEN::LS:
		{
		    // could be one of two things:
		    // [expression]
		    // [I] in instructions like LD [I], Vx
		    j++;
		    std::cout << "Parsing pure expression" << std::endl;
		    int exprval = parseExpr(tokstream);
		    std::cout << "Current token: " << (int) tokstream[j].first << std::endl;
		    mod_tokstream.push_back({TOKEN::NUM, std::to_string(exprval)});
		}
		break;

	    case TOKEN::STR:
		mod_tokstream.push_back(tokstream[j]); // push TOKEN::STR
		{
		    std::cout << "STORE TOKEN ENCOUNTERED" << std::endl;
		    j++;

		    if (tokstream[j].first == TOKEN::LIST_DEREF)
		    {
			std::string list_var = tokstream[j].second;
			list_var.pop_back();
			offset += 1 * list_lookup_table[list_var].size(); // single byte
			
			// offset can not be odd because 2-byte alignment is needed
			if (offset % 2 == 1)
			{
			    offset += 1;
			}
		    }
		}
		mod_tokstream.push_back(tokstream[j]); // push TOKEN::[LABEL | VAR | LIST]_DEREF
		break;

	    case TOKEN::CLS:
	    case TOKEN::JMP:
	    case TOKEN::LD:
	    case TOKEN::DRW:
	    case TOKEN::ADD:
	    case TOKEN::SNE:
	    case TOKEN::SE:
	    case TOKEN::RET:
	    case TOKEN::CALL:
	    case TOKEN::SUB:
	    case TOKEN::SHL:
	    case TOKEN::SHR:
	    case TOKEN::AND:
	    case TOKEN::OR:
	    case TOKEN::XOR:
		std::cout << "CURRENT OFFSET: " << std::hex << offset << ' ' << value << std::endl;
		offset += 2;
		mod_tokstream.push_back({token, value});
		break;

	    case TOKEN::REG:
	    case TOKEN::IDX:
	    case TOKEN::IDX_DEREF:
	    case TOKEN::NUM:
	    case TOKEN::LABEL_DEREF:
	    case TOKEN::VAR_DEREF:
	    case TOKEN::LIST_DEREF:
	    case TOKEN::EOFF:
		mod_tokstream.push_back({token, value});
		break;

	    default:
		break;
	}

	var_lookup_table["%OFFSET"] = offset;

	j++;
    }

    return mod_tokstream;
}


std::vector<std::string> Parser::infix_to_postfix(const std::vector<std::string>& buffer)
{
    std::vector<std::string> postfix;
    std::stack<std::string> st;

    std::cout << "Buffer: ";
    for (auto& it : buffer)
    {
	std::cout << it << ' ';
    }
    std::cout << std::endl;

    int i = 0;
    while (i < buffer.size())
    {
	if (buffer[i] == "+" || buffer[i] == "-" || buffer[i] == "*" || buffer[i] == "/" || buffer[i] == "^")
	{
	    while (!st.empty() && priority(st.top(), buffer[i]))
	    {
		postfix.push_back(st.top());
		st.pop();
	    }

	    // push the math operator
	    st.push(buffer[i]);
	}
	else
	{
	    postfix.push_back(buffer[i]);
	}

	i++;
    }

    while (!st.empty())
    {
	postfix.push_back(st.top());
	st.pop();
    }

    std::cout << "Converting buffer into postfix expression produces the following stream: ";
    for (auto& it : postfix)
    {
	std::cout << it << ' ';
    }
    std::cout << std::endl;

    return postfix;
}


int Parser::eval_postfixexpr(const std::vector<std::string>& postfix)
{
    std::stack<int> st;
    int i = 0;
    while (i < postfix.size())
    {
	if (postfix[i] == "*" || postfix[i] == "/" || postfix[i] == "+" || postfix[i] == "-" || postfix[i] == "^")
	{
	    int a = st.top(); st.pop();
	    int b = st.top(); st.pop();

	    if (postfix[i] == "*")
	    {
		st.push(b * a);
	    }
	    else if (postfix[i] == "+")
	    {
		st.push(b + a);
	    }
	    else if (postfix[i] == "/")
	    {
		st.push(b / a);
	    }
	    else if (postfix[i] == "-")
	    {
		st.push(b - a);
	    }
	    else if (postfix[i] == "^")
	    {
		st.push(b ^ a);
	    }
	}
	else
	{
	    st.push(std::stoi(postfix[i]));
	}

	i++;
    }

    std::cout << "Expression value after postfix evaluation: " << st.top() << std::endl;

    return st.top();
}


int Parser::evalExpr(const std::vector<std::string>& buffer)
{
    // buffer is an infix expression
    // convert to postfix expression
    
    std::vector<std::string> postfix = infix_to_postfix(buffer);
    
    // postfix evaluation
    return eval_postfixexpr(postfix);
}


// This design assumes a few shitty syntax that you just have to deal with
// because I am not smart enough to figure out how to avoid / fix
//
// 1. all variables must be dereferenced ONLY through the help of
// TOKEN::LS and TOKEN::RS
//
// AKA, like this:
// [%var%]
//
// 2. all list variables must also be dereferenced ONLY through
// the help of TOKEN::LS and TOKEN::RS
//
// AKA like this:
// [$list$]
//
// you can index the lists like this:
// [$list$[<expr>]]


// !!NOTE: ONLY call this method IF you encounter a TOKEN::LS
// !!NOTE: before calling this method, you MUST consume TOKEN::LS
// !!NOTE: after calling this mehtod, you MUST consume TOKEN::RS
int Parser::parseExpr(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    std::vector<std::string> buffer;
    TOKEN tok  = tokstream[j].first;
    while (tok != TOKEN::RS)
    {
	std::string val;
	if (tok == TOKEN::LS)
	{
	    j++;
	    int temp = parseExpr(tokstream);
	    val = std::to_string(temp);
	}
	else if (tok == TOKEN::LABEL_DEREF)
	{
	    std::string label = val;
	    val = std::to_string(offset_table[label.substr(1)]);
	}
	else if (tok == TOKEN::VAR_DEREF)
	{
	    std::string var = tokstream[j].second;
	    var.pop_back();
	    val = std::to_string(var_lookup_table[var]);
	}
	else if (tok == TOKEN::LIST_DEREF)
	{
	    std::string list_var = tokstream[j].second;
	    list_var.pop_back();

	    j++;
	    int list_index = 0;

	    if (tokstream[j].first == TOKEN::LS)
	    {
		list_index = parseExpr(tokstream);
	    }
	    
	    val = std::to_string(list_lookup_table[list_var][list_index]);
	}
	else if (tok == TOKEN::MATH_OP || tok == TOKEN::NUM)
	{
	    val = tokstream[j].second;
	}

	buffer.push_back(val);

	j++;

	tok = tokstream[j].first;
    }

    int exprval = evalExpr(buffer);

    return exprval;
}


int Parser::parseAssignment(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    auto [token, value] = tokstream[j];

    if (token == TOKEN::LS)
    {
	j++;
	int temp = parseExpr(tokstream);
	return temp;
    }
    else if (token == TOKEN::NUM)
    {
	return std::stoi(value);
    }
    else if (token == TOKEN::VAR_DEREF)
    {
	std::string var = value;
	var.pop_back();
	return var_lookup_table[var];
    }

    return -1;
}


// !!NOTE: ONLY call this method IF you encounter a TOKEN::LP
// !!NOTE: you MUST consume TOKEN::LP before calling this method
// !!NOTE: you MUST consume the ending TOKEN::RP after the call to this method is done
std::vector<uint8_t> Parser::parseList(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    std::vector<uint8_t> buffer;
    auto [token, value] = tokstream[j];
    while (token != TOKEN::RP)
    {
	switch(token)
	{
	    case TOKEN::NUM:
		buffer.push_back(std::stoi(value));
		break;

	    case TOKEN::VAR_DEREF:
		{
		    std::string var = value;
		    var.pop_back();
		    buffer.push_back(var_lookup_table[var]);
		    break;
		}

	    case TOKEN::LS:
		j++;
		buffer.push_back(parseExpr(tokstream));

	    default:
		break;
	}

	j++;

	token = tokstream[j].first; value = tokstream[j].second;
    }

    return buffer;
}


std::vector<uint8_t> Parser::parseListAssignment(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    auto [token, value] = tokstream[j];

    if (token == TOKEN::LP)
    {
	j++;
	std::vector<uint8_t> temp = parseList(tokstream);
	return temp;
    }

    return {};
}


bool priority(const std::string& op1, const std::string& op2)
{
    if (op1 == "^")
    {
	return true;
    }
    else if (op1 == "*" || op1 == "/")
    {
	return true;
    }
    else
    {
	if (op2 == "+" || op2 == "-")
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
}

