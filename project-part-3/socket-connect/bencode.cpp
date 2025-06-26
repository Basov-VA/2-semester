#include "bencode.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <openssl/sha.h>

void sha1(const char *data, size_t length, unsigned char *hash) {
    SHA1((const unsigned char *) data, length, hash);
}
namespace Bencode {
    std::shared_ptr<BenCode> parse_fucking_BenCode(const char* ben, uint64_t len, uint64_t &pos)
    {
        char x = ben[pos++];
        std::shared_ptr<BenCode> res;
        if(x == 'i')
        {
            uint64_t pos1 = pos;
            while(pos1 < len && ben[pos1] != 'e')
            {
                ++pos1;
            }
            if(pos1 >= len)
            {
                throw std::runtime_error("Invalid bencode: index out of rangeaaa");;
            }
            std::string s(ben + pos, pos1 - pos);
            res = std::make_shared<BenCode_Integer>(BenCode_Integer(std::stoll(s)));
            pos = pos1 + 1;
            return res;
        }

        else if(x == 'l')
        {
            res = std::make_shared<BenCode_List>();
            while (ben[pos] != 'e')
            {
                std::dynamic_pointer_cast<BenCode_List>(res)->list_data_.push_back(parse_fucking_BenCode(ben,len, pos));
            }
            ++pos;
            return res;
        }

        else if(x == 'd')
        {
            res = std::make_shared<BenCode_Dictionary>();
            while(ben[pos] != 'e')
            {
                std::string key = std::dynamic_pointer_cast<BenCode_String>(parse_fucking_BenCode(ben,len,pos))->data_;
                std::dynamic_pointer_cast<BenCode_Dictionary>(res)->dict_data_[key] = parse_fucking_BenCode(ben,len,pos);
            }
            pos++;
            return res;
        }
        else
        {
            uint64_t pos1 = pos - 1;
            while(pos1 < len && ben[pos1] != ':')
            {
                if(ben[pos1] == 'e')
                {
                    throw std::runtime_error("Invalid bencode: index out of rangezzz");;
                }
                pos1++;
            }
            if(pos1 >= len)
            {
                throw std::runtime_error("Invalid bencode: unexpected end of input");
            }
            std::string ans(ben + pos - 1, pos1 - pos + 1);
            size_t len = std::stoll(ans);
            pos = pos1 + 1;
            res = std::make_shared<BenCode_String>(std::string(ben + pos, len));
            pos += len;
            return res;
        }
    }

    std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& nig){
        std::string s = dynamic_pointer_cast<BenCode_String>(dynamic_pointer_cast<BenCode_Dictionary>(nig->get("info"))->get("pieces"))->get_s();

        std::vector<std::string> result;
        for (int i = 0; i < s.length(); i += 20) {
            result.push_back(s.substr(i, 20));
        }
        return result;
    }

    std::vector<Peer> parsePeers(const std::string& data){
        std::vector<Peer> peers_;
        for(int i = 0; i<data.size();i+=6) {
        std::string ip = std::to_string(static_cast<uint8_t>(data[i])) + "." + std::to_string(static_cast<uint8_t>(data[i + 1])) + "."
                + std::to_string(static_cast<uint8_t>(data[i + 2])) + "." +
                std::to_string(static_cast<uint8_t>(data[i + 3]));
        int port = static_cast<uint8_t>(data[i + 4]) * 256 + static_cast<uint8_t>(data[i + 5]);
        peers_.push_back({ip,port});
        }

        return peers_;
    }

    std::string GetInfoHash(const BenCode_Dictionary& FullInfo){
        std::string info = FullInfo.get("info")->getstr();

        unsigned char hash[20];
        SHA1((const unsigned char *)info.c_str(), info.length(), hash);
        std::string result;
        for (int i = 0; i < 20; i++) {
            result += hash[i];
        }

        return result;
    }
}
