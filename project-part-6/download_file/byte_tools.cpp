#include "byte_tools.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <vector>

int BytesToInt(std::string_view bytes) {
    int res = 0;
    for(int i = 0; i < bytes.size(); ++i)
    {
        res = (res << 8) + (int)(unsigned char)(bytes[i]);
    }
    return res;
}

std::string CalculateSHA1(const std::string& msg) {
    unsigned char hash[20];
    SHA1((const unsigned char *) msg.c_str(), msg.length(), hash);
    std::string result;
    for (int i = 0; i < 20; i++) {
        result += hash[i];
    }
    return result;
}

std::string IntToBytes(int val)
{
    std::string ans;
    ans.resize(4);
    ans[0] = (val >> 24) & 0xFF;
    ans[1] = (val >> 16) & 0xFF;
    ans[2] = (val >> 8) & 0xFF;
    ans[3] = (val >> 0) & 0xFF;
    return ans;

}
/*
 * Представить массив байтов в виде строки, содержащей только символы, соответствующие цифрам в шестнадцатеричном исчислении.
 * Конкретный формат выходной строки не важен. Важно то, чтобы выходная строка не содержала символов, которые нельзя
 * было бы представить в кодировке utf-8. Данная функция будет использована для вывода SHA1 хеш-суммы в лог.
 */
std::string HexEncode(const std::string& input) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : input) {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}
