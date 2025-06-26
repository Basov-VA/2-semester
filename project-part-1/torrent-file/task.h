#pragma once

#include <string>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <iostream>
#include<cstdio>
#include <cstdint>
#include <any>
#include<string>
#include <utility>
#include <memory>

void sha1(const char *data, size_t length, unsigned char *hash) {
    SHA1((const unsigned char *) data, length, hash);
}

struct TorrentFile {
    std::string announce;
    std::string comment;
    std::vector<std::string> pieceHashes;
    size_t pieceLength;
    size_t length;
    std::string name;
    std::string infoHash;
};

/*
 * Функция парсит .torrent файл и загружает информацию из него в структуру `TorrentFile`. Как устроен .torrent файл, можно
 * почитать в открытых источниках (например http://www.bittorrent.org/beps/bep_0003.html).
 * После парсинга файла нужно также заполнить поле `infoHash`, которое не хранится в файле в явном виде и должно быть
 * вычислено. Алгоритм вычисления этого поля можно найти в открытых источника, как правило, там же,
 * где описание формата .torrent файлов.
 * Данные из файла и infoHash будут использованы для запроса пиров у торрент-трекера. Если структура `TorrentFile`
 * была заполнена правильно, то трекер найдет нужную раздачу в своей базе и ответит списком пиров. Если данные неверны,
 * то сервер ответит ошибкой.
 */

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
    if(x == 'l') {
        res = std::make_shared<BenCode_List>();
        while (ben[pos] != 'e') {
            std::dynamic_pointer_cast<BenCode_List>(res)->list_data_.push_back(parse_fucking_BenCode(ben, len, pos));
        }
        ++pos;
        return res;
    }
        else if(x == 'i')
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
