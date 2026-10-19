#include <vector>
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/generator.hpp"

#include <string>

#include <iostream>
#include <cstdint>
#include <unordered_map>

// #include "../include/generator.hpp"


void printLabels(const std::unordered_map<std::string, uint16_t>& offset_table)
{
    for (auto [label, offset] : offset_table)
    {
	std::cout << label << ' ' << std::hex << unsigned(offset) << std::endl;
    }
}

void printTokStream(const std::vector<std::pair<TOKEN, std::string>>& tokstream)
{
    for (auto [token, value] : tokstream)
    {
	std::cout << (int) token << '\t' << value << std::endl;
    }
}

void printVarLookupTable(const std::unordered_map<std::string, int>& var_lookup_table)
{
    for (auto [var, val] : var_lookup_table)
    {
	std::cout << var << ' ' << val << std::endl;
    }
}

void printListLookupTable(const std::unordered_map<std::string, std::vector<uint8_t>>& list_lookup_table)
{
    for (auto& [list, vec] : list_lookup_table)
    {
	std::cout << list << ": ";
	for (auto& it : vec)
	{
	    std::cout << std::hex << (unsigned) it << ' ';
	}
	std::cout << std::endl;
    }
}


int main(int argc, char** argv)
{
    std::vector<std::pair<TOKEN, std::string>> tokstream;

    Lexer lexer = Lexer{"/home/mango/personal/CHIP-8-Assembler/test-roms/rom5.asm"};
    tokstream = lexer.lex();
    
    printTokStream(tokstream);

    Parser parser = Parser();
    tokstream = parser.parse(tokstream);

    printTokStream(tokstream);

    std::cout << "First pass done" << std::endl;

    std::cout << "Labels: " << std::endl;
    std::unordered_map<std::string, uint16_t> offset_table = parser.getOffsetTable();
    printLabels(offset_table);

    std::cout << "Variables: " << std::endl;
    std::unordered_map<std::string, int> var_lookup_table = parser.getVarLookupTable();
    printVarLookupTable(var_lookup_table);

    std::cout << "Lists: " << std::endl;
    std::unordered_map<std::string, std::vector<uint8_t>> list_lookup_table = parser.getListLookupTable();
    printListLookupTable(list_lookup_table);

    // right now, I don't know what will happen to this phase
    // second pass, write instruction to outfile
    Generator generator = Generator(tokstream, offset_table, var_lookup_table, list_lookup_table);
    generator.generate();

    std::cout << "Second pass done" << std::endl;

    std::cout << tokstream.size() << std::endl;

    return 0;
}

