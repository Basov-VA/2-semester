#include "byte_tools.h"
#include <string>
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

