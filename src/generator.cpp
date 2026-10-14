#include "../include/generator.hpp"
#include <cstdint>

#include <iostream>


const char* path = "binary.ch8";


// helper functions
int reg_x(const std::string& reg);
uint16_t reorder_byte(const uint16_t byte);


// helper function definitions
uint16_t reorder_byte(const uint16_t byte)
{
    uint16_t rbyte = (0xFF00 & (byte << 8u)) | (0x00FF & (byte >> 8u));
    return rbyte;
}


int reg_x(const std::string& reg)
{
    switch(reg.c_str()[1])
    {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    return std::atoi(&reg[1]);
	case 'A':
	    return 10;
	case 'B':
	    return 11;
	case 'C':
	    return 12;
	case 'D':
	    return 13;
	case 'E':
	    return 14;
	case 'F':
	    return 15;
	default:
	    break;
    }

    return -1;
}

// ---------------------------------------------------------------

Generator::Generator
    (const std::vector<std::pair<TOKEN, std::string>>& tokstream,
     const std::unordered_map<std::string, uint16_t>& offset_table,
     const std::unordered_map<std::string, int>& var_lookup_table,
     const std::unordered_map<std::string, std::vector<uint8_t>>& list_lookup_table)
    : tokstream(tokstream), offset_table(offset_table),
      var_lookup_table(var_lookup_table), list_lookup_table(list_lookup_table)
{
    j = -1;
    out.open(path, std::ios::out | std::ios::binary);

    init();
}


Generator::~Generator()
{
    out.close();
}


void Generator::init()
{
    parseTable.insert({TOKEN::CLS, &Generator::parseCLS});
    parseTable.insert({TOKEN::JMP, &Generator::parseJMP});
    parseTable.insert({TOKEN::LD, &Generator::parseLD});
    parseTable.insert({TOKEN::DRW, &Generator::parseDRW});
    parseTable.insert({TOKEN::ADD, &Generator::parseADD});
    parseTable.insert({TOKEN::SNE, &Generator::parseSNE});
    parseTable.insert({TOKEN::SE, &Generator::parseSE});
    parseTable.insert({TOKEN::RET, &Generator::parseRET});
    parseTable.insert({TOKEN::CALL, &Generator::parseCALL});
    parseTable.insert({TOKEN::SUB, &Generator::parseSUB});
}


std::pair<TOKEN, std::string> Generator::nextToken()
{
    j++;
    return tokstream[j];
}


void Generator::generate()
{
    std::cout << std::dec << j << ' ' << tokstream.size() << std::endl;

    std::cout << (j < (int) tokstream.size()) << std::endl;

    while (j < (int) tokstream.size())
    {
	auto [token, value] = nextToken();
	std::cout << "val: " << value << std::endl;
	if (token == TOKEN::EOFF)
	{
	    break;
	}
	else if (token == TOKEN::STR)
	{
	    // store the fucker
	    auto [tok2, val2] = nextToken();

	    if (tok2 == TOKEN::LIST_DEREF)
	    {
		std::string list_var = val2;
		list_var.pop_back();
		std::vector<uint8_t> vec = list_lookup_table[list_var];
		std::cout << "ENTERING RAW BYTES INTO MEMORY" << std::endl;
		int i = 0;
		while (i < vec.size() - 1)
		{
		    uint16_t byte = 0x0000;
		    byte |= (vec[i] << 8u) | (vec[i + 1]);
		    byte = reorder_byte(byte);
		    out.write((char*) &byte, sizeof(byte));
		    i += 2;
		}

		if (vec.size() % 2 == 1)
		{
		    std::cout << "ONE BYTE REMAINING" << std::endl;
		    // one byte still remaining
		    uint16_t byte = 0x0000;
		    byte |= (vec.back() << 8u);
		    byte = reorder_byte(byte);
		    out.write((char*) &byte, sizeof(byte));
		}
	    }
	    else if (tok2 == TOKEN::VAR_DEREF)
	    {
		std::string var = val2;
		var.pop_back();
		uint16_t byte = var_lookup_table[var];
		byte = reorder_byte(byte);
		out.write((char*) &byte, sizeof(byte));
	    }
	    else if (tok2 == TOKEN::NUM)
	    {
		uint16_t byte = std::stoi(val2);
		byte = reorder_byte(byte);
		out.write((char*) &byte, sizeof(byte));
	    }
	}
	else if (parseTable.find(token) != parseTable.end())
	{
	    uint16_t byte = (*this.*parseTable[token])();
	    byte = reorder_byte(byte);
	    out.write((char*) &byte, sizeof(byte));
	}
	else
	{
	    // unknown token
	}

	std::cout << j << std::endl;
    }
}


// ---------------------------------------------------------------
// CHIP-8 instructions

uint16_t Generator::parseCLS()
{
    uint16_t byte = 0x00E0;
    return byte;
}


uint16_t Generator::parseJMP()
{
    // two JMP instructions are possible:
    // JMP addr 	(0x1nnn)
    // JMP V0, addr	(0xbnnn)
    auto [arg1, v1] = nextToken();
    uint16_t byte = 0x1000;

    if (arg1 == TOKEN::REG)
    {
	auto [arg2, v2] = nextToken();
	arg1 = arg2;
	v1 = v2;
	byte = 0xB000;
    }

    if (arg1 == TOKEN::LABEL_DEREF)
    {
	uint16_t nnn = offset_table[v1.substr(1)];
	byte |= nnn;
    }
    else if (arg1 == TOKEN::NUM)
    {
	uint16_t nnn = std::stoi(v1.c_str());
	byte |= nnn;
    }
    else if (arg1 == TOKEN::VAR_DEREF)
    {
	std::string var = v1;
	var.pop_back();
	uint16_t nnn = var_lookup_table[var];
	byte |= nnn;
    }

    return byte;
}


uint16_t Generator::parseLD()
{
    // bunch of LD instruction are possible:
    // LD Vx, nn (0x6xnn)
    // LD I, nnn (0xAnnn)
    // LD [I], Vx (0xFx55)
    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();

    uint16_t byte;

    if (arg1 == TOKEN::REG)
    {
	int reg = reg_x(v1);

	if (arg2 == TOKEN::REG)
	{
	    int reg2 = reg_x(v2);
	    byte = 0x8000;
	    byte |= (reg << 8u) | (reg2 << 4u);
	}
	else if (arg2 == TOKEN::NUM)
	{
	    byte = 0x6000;
	    int nn = std::stoi(v2);
	    uint16_t nnn = (reg << 8u) | nn;

	    byte |= nnn;
	}
	else if (arg2 == TOKEN::VAR_DEREF)
	{
	    byte = 0x6000;
	    std::string var = v2; var.pop_back();
	    int nn = var_lookup_table[var];
	    uint16_t nnn = (reg << 8u) | nn;
	    
	    byte |= nnn;
	}
	else if (arg2 == TOKEN::IDX_DEREF)
	{
	    byte = 0xF065;
	    byte |= (reg << 8u);
	}
    }
    else if (arg1 == TOKEN::IDX)
    {
	if (arg2 == TOKEN::NUM)
	{
	    byte = 0xA000;
	    int nnn = std::stoi(v2);
	    byte |= nnn;
	}
	else if (arg2 == TOKEN::VAR_DEREF)
	{
	    byte = 0xA000;
	    std::string var = v2; var.pop_back();
	    int nnn = var_lookup_table[var];
	    byte |= nnn;
	}
    }
    else if (arg1 == TOKEN::IDX_DEREF)
    {
	if (arg2 == TOKEN::REG)
	{
	    int reg = reg_x(v2);
	    byte = 0xF055 | (reg << 8u);
	}
    }

    return byte;
}


uint16_t Generator::parseDRW()
{
    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();
    auto [arg3, v3] = nextToken();

    int reg1 = reg_x(v1);
    int reg2 = reg_x(v2);
    int height = std::stoi(v3);

    uint16_t byte = 0xD000 | (reg1 << 8u) | (reg2 << 4u) | height;
    return byte;
}


uint16_t Generator::parseADD()
{
    // three kinds of ADD instructions:
    // ADD Vx, byte (0x7xkk)
    // ADD Vx, Vy   (0x8xy2)
    // ADD I, Vx    (0xFx1E)
    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();

    uint16_t byte;

    if (arg1 == TOKEN::REG)
    {
	if (arg2 == TOKEN::REG)
	{
	    int Vx = reg_x(v1);
	    int Vy = reg_x(v2);
	    byte = 0x8002 | (Vx << 8u) | (Vy << 4u);
	}
	else if (arg2 == TOKEN::NUM)
	{
	    int Vx = reg_x(v1);
	    int kk = std::stoi(v2);
	    byte = 0x7000 | (Vx << 8u) | kk;
	}
	else if (arg2 == TOKEN::VAR_DEREF)
	{
	    int Vx = reg_x(v1);
	    std::string var = v2; var.pop_back();
	    int kk = var_lookup_table[var];
	    byte = 0x7000 | (Vx << 8u) | kk;
	}
    }
    else if (arg1 == TOKEN::IDX)
    {
	if (arg2 == TOKEN::REG)
	{
	    int Vx = reg_x(v2);
	    byte = 0xF01E | (Vx << 8u);
	}
    }

    return byte;
}


uint16_t Generator::parseSNE()
{
    // SNE - skip next instruction if not equal
    // SNE Vx Vy	(0x9xy0)
    // SNE Vx byte	(0x4xkk)

    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();

    uint16_t byte;

    if (arg1 == TOKEN::REG)
    {
	int reg = reg_x(v1);

	if (arg2 == TOKEN::REG)
	{
	    int reg2 = reg_x(v2);
	    byte = 0x9000 | (reg << 8u) | (reg2 << 4u);
	}
	else if (arg2 == TOKEN::NUM)
	{
	    uint8_t kk = std::stoi(v2);
	    byte = 0x4000 | (reg << 8u) | kk;
	}
    }

    return byte;
}


uint16_t Generator::parseRET()
{
    // RET (0x00EE)

    uint16_t byte = 0x00EE;
    return byte;
}

uint16_t Generator::parseCALL()
{
    // CALL addr (0x2nnn)

    auto [arg1, v1] = nextToken();

    uint16_t byte = 0x2000;

    if (arg1 == TOKEN::NUM)
    {
	int nnn = std::stoi(v1);
	byte |= nnn;
    }
    else if (arg1 == TOKEN::LABEL_DEREF)
    {
	std::string label = v1;
	int nnn = offset_table[label.substr(1)];
	byte |= nnn;
    }

    return byte;
}

uint16_t Generator::parseSE()
{
    // SE Vx, byte (0x3xkk)

    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();

    uint16_t byte = 0x3000;

    if (arg1 == TOKEN::REG)
    {
	int reg = reg_x(v1);

	if (arg2 == TOKEN::NUM)
	{
	    int kk = std::stoi(v2);
	    byte |= (reg << 8u) | kk;
	}
    }

    return byte;
}


uint16_t Generator::parseSUB()
{
    // SUB Vx, Vy (0x8xy5)
    // sets VF = 1 if Vx > Vy

    auto [arg1, v1] = nextToken();
    auto [arg2, v2] = nextToken();

    uint16_t byte = 0x8005;

    if (arg1 == TOKEN::REG && arg2 == TOKEN::REG)
    {
	int vx = reg_x(v1);
	int vy = reg_x(v2);

	byte |= (vx << 8u) | (vy << 4u);
    }

    return byte;
}

/*
uint16_t Generator::parseOR()
{
}

uint16_t Generator::parseAND()
{
}

uint16_t Generator::parseXOR()
{
}

uint16_t Generator::parseSHR()
{
}

uint16_t Generator::parseSUBN()
{
}

uint16_t Generator::parseSHL()
{
}


uint16_t Generator::parseRND()
{
}

uint16_t Generator::parseSKP()
{
}

uint16_t Generator::parseSKNP()
{
}
*/

