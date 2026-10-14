#pragma once

#include <vector>
#include "lexer.hpp"

#include <unordered_map>
#include <cstdint>

class Parser
{
public:
    Parser();
    std::vector<std::pair<TOKEN, std::string>> parse(const std::vector<std::pair<TOKEN, std::string>>& tokstream);

    std::unordered_map<std::string, int> getVarLookupTable();
    std::unordered_map<std::string, uint16_t> getOffsetTable();
    std::unordered_map<std::string, std::vector<uint8_t>> getListLookupTable();

private:
    int j; // internal pointer to tokstream
    
    uint16_t offset; // initial offset default to 0x200
    std::unordered_map<std::string, uint16_t> offset_table;
    std::unordered_map<std::string, int> var_lookup_table;
    std::unordered_map<std::string, std::vector<uint8_t>> list_lookup_table;

private:
    std::vector<std::string> infix_to_postfix(const std::vector<std::string>& buffer);
    int eval_postfixexpr(const std::vector<std::string>& postfix);
    int evalExpr(const std::vector<std::string>& buffer);
    int parseExpr(const std::vector<std::pair<TOKEN, std::string>>& tokstream);
    int parseAssignment(const std::vector<std::pair<TOKEN, std::string>>& tokstream);


    std::vector<uint8_t> parseList(const std::vector<std::pair<TOKEN, std::string>>& tokstream);
    std::vector<uint8_t> parseListAssignment(const std::vector<std::pair<TOKEN, std::string>>& tokstream);
};
