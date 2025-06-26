#pragma once

#include "peer.h"
#include "torrent_file.h"
#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <cpr/cpr.h>
#include <memory>
#include <iostream>

class BenCode
{
public:
    virtual ~BenCode() = default;
    virtual std::string getstr() const = 0;
};

class BenCode_List : public BenCode
{
public:
    std::vector<std::shared_ptr<BenCode>> list_data_;

    uint64_t size() const noexcept
    {
        return list_data_.size();
    }

    std::shared_ptr<BenCode> get_l(size_t  index) const
    {
        if(index >= list_data_.size()){
            throw std::runtime_error("Invalid bencode: index out of rangeqqq");
        }
        return list_data_[index];
    }

    std::string getstr() const noexcept
    {
        std::string sub_ans = "";
        for(auto& el : list_data_)
        {
            sub_ans += el->getstr();
        }
        return "l" + sub_ans + "e";
    }
};


class BenCode_Integer : public BenCode
{
public:
    int64_t data_;
    BenCode_Integer(long long data): data_(data){}
    long long get_i() const noexcept
    {
        return data_;
    }

    std::string getstr() const noexcept
    {
        return "i" + std::to_string(data_) + "e";
    }
};

class BenCode_String : public BenCode
{
public:
    std::string data_;
    BenCode_String(std::string data) : data_(data) {}

    std::string get_s() const noexcept
    {
        return data_;
    }

    std::string getstr() const noexcept override
            {
                    return std::to_string(data_.length()) + ":" + data_;
            }

};


struct BenCode_Dictionary: public BenCode
{
public:
    std::map<std::string, std::shared_ptr<BenCode>> dict_data_;

    uint64_t size() const noexcept
    {
        return dict_data_.size();
    }


    std::shared_ptr<BenCode> get(const std::string &key) const
    {
        if(dict_data_.find(key) == dict_data_.end())
        {
            throw std::runtime_error("Invalid bencode: no key " + key);
        }
        return dict_data_.find(key)->second;
    }

    std::string getstr() const noexcept override
            {
                    std::string sub_ans = "";
            for(auto& el : dict_data_)
            {
                sub_ans += BenCode_String(el.first).getstr() + el.second->getstr();
            }


            return "d" + sub_ans + "e";
            }

};


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

void sha1(const char *data, size_t length, unsigned char *hash) {
    SHA1((const unsigned char *) data, length, hash);
}
std::string do_hash(const BenCode_Dictionary& dict){
    if (dict.dict_data_.find("info") == dict.dict_data_.end()) {
        throw std::runtime_error("Invalid bencode: no info dictionary");
    }
    std::string subres = dict.get("info")->getstr();
    unsigned char arr[20];
    sha1(subres.c_str(), subres.length(), arr);
    std::string result = "";
    for (int i = 0; i < 20; i++) {
        result += arr[i];
    }
    return result;
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

std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& nig){
    std::string s = dynamic_pointer_cast<BenCode_String>(dynamic_pointer_cast<BenCode_Dictionary>(nig->get("info"))->get("pieces"))->get_s();

    std::vector<std::string> result;
    for (int i = 0; i < s.length(); i += 20) {
        result.push_back(s.substr(i, 20));
    }
    return result;
}





TorrentFile LoadTorrentFile(const std::string& file) {
    std::ifstream z(file, std::ios::binary);
    std::stringstream b;
    b << z.rdbuf();
    std::string data = b.str();
    z.close();

    TorrentFile Ans;
    uint64_t position = 0;

    std::shared_ptr<BenCode_Dictionary> dict = std::dynamic_pointer_cast<BenCode_Dictionary>(parse_fucking_BenCode(data.c_str(),data.size(),position));
    Ans.announce = std::dynamic_pointer_cast<BenCode_String>(dict->get("announce"))->get_s();
    Ans.comment = std::dynamic_pointer_cast<BenCode_String>(dict->get("comment"))->get_s();
    Ans.pieceLength = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(dict->get("info"))->get("piece length"))->get_i();
    Ans.length = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(dict->get("info"))->get("length"))->get_i();
    Ans.pieceHashes = GetPieceHashes(dict);
    Ans.infoHash = GetInfoHash(*dict);
    return Ans;
}






std::string get_str_peers_dlya_daunov(std::string ben_code_string)
{
    uint64_t pos = 0;
    std::shared_ptr<BenCode_Dictionary> dict = std::dynamic_pointer_cast<BenCode_Dictionary>(parse_fucking_BenCode(ben_code_string.c_str(),ben_code_string.size(),pos));
    std::shared_ptr<BenCode_String> sub_res = std::dynamic_pointer_cast<BenCode_String>(dict->get("peers"));
    std::string res = sub_res->get_s();
    return res;
}




class TorrentTracker {
public:
    /*
     * url - адрес трекера, берется из поля announce в .torrent-файле
     */
    TorrentTracker(const std::string& url):
            url_(url),
            peers_()
    {}

    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port)
    {
        cpr::Response res = cpr::Get(
                cpr::Url{tf.announce},
                cpr::Parameters {
                        {"info_hash", tf.infoHash},
                        {"peer_id", peerId},
                        {"port", std::to_string(port)},
                        {"uploaded", std::to_string(0)},
                        {"downloaded", std::to_string(0)},
                        {"left", std::to_string(tf.length)},
                        {"compact", std::to_string(1)}
                },
                cpr::Timeout{20000}
        );

        std::string data = get_str_peers_dlya_daunov(res.text);
        //std::cout << data.size() << std::endl;
        for(int i = 0; i < data.size(); i+=6)
        {
            std::string ip = std::to_string(static_cast<uint8_t>(data[i])) + "." + std::to_string(static_cast<uint8_t>(data[i + 1])) + "."
                                            + std::to_string(static_cast<uint8_t>(data[i + 2])) + "." + std::to_string(static_cast<uint8_t>(data[i + 3]));
            int port = static_cast<uint8_t>(data[i + 4]) * 256 + static_cast<uint8_t>(data[i + 5]);
            peers_.push_back({ip,port});
        }
    }

    /*
     * Отдает полученный ранее список пиров
     */
    const std::vector<Peer>& GetPeers() const
    {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};

