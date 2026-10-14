#pragma once

#include "lexer.hpp"
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <fstream>

class Generator
{
public:
    Generator(
	    const std::vector<std::pair<TOKEN, std::string>>& tokstream,
	    const std::unordered_map<std::string, uint16_t>& offset_table,
	    const std::unordered_map<std::string, int>& var_lookup_table,
	    const std::unordered_map<std::string, std::vector<uint8_t>>& list_lookup_table);
    ~Generator();

    std::pair<TOKEN, std::string> nextToken();
    void generate();

    void init();

private:
    int j; // internal pointer to the tokstream
    std::vector<std::pair<TOKEN, std::string>> tokstream;
    std::unordered_map<std::string, uint16_t> offset_table;
    std::unordered_map<std::string, int> var_lookup_table;
    std::unordered_map<std::string, std::vector<uint8_t>> list_lookup_table;

    std::unordered_map<TOKEN, uint16_t (Generator::*)()> parseTable;

    std::ofstream out;

    uint16_t parseCLS();
    uint16_t parseJMP();
    uint16_t parseLD();
    uint16_t parseDRW();
    uint16_t parseADD();
    uint16_t parseSNE();

    void parseSTR();
};

