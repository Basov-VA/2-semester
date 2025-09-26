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
#include <cstdio>
#include <cstdint>
#include <any>
#include <string>
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

class BenCode
{
public:
    virtual ~BenCode() = default;
    virtual std::string getStr() const = 0;
    static std::shared_ptr<BenCode> parse(const char* data, size_t length, size_t& pos);
    //virtual const int64_t getData() const = 0;
    //virtual const std::string getData() const = 0;
};

class BenCode_List : public BenCode
{
private:
    std::vector<std::shared_ptr<BenCode>> list_data_;
public:

    uint64_t getSize() const noexcept
    {
        return list_data_.size();
    }

    std::shared_ptr<BenCode> getValue(size_t  index) const
    {
        if(index >= list_data_.size()){
            throw std::runtime_error("Invalid bencode: index out of range");
        }
        return list_data_[index];
    }

    void addValue(std::shared_ptr<BenCode> element) {
        list_data_.push_back(std::move(element));
    }

    std::string getStr() const noexcept
    {
        std::string res = "";
        for(const auto& el : list_data_)
        {
            res += el->getStr();
        }
        return "l" + res + "e";
    }

    static std::shared_ptr<BenCode_List> parse(const char* data, size_t length, size_t& pos){
        if (pos >= length || data[pos] != 'l') {
            throw std::runtime_error("Expected list 'l' at position " + std::to_string(pos) + " while parsing list values");
        }

        pos++;
        auto list = std::make_shared<BenCode_List>();
        while(pos < length && data[pos] != 'e')
        {
            auto new_element = BenCode::parse(data, length, pos);
            list->addValue(std::move(new_element));
        }

        if(pos >= length)
        {
            throw std::runtime_error("Unterminated list: missing 'e'");
        }
        pos++;
        return list;
    }

};

class BenCode_Integer : public BenCode
{
private:
    int64_t data_;
public:
    BenCode_Integer(int64_t data): data_(data){}
    
    int64_t getData() const noexcept //override
    {
        return data_;
    }

    std::string getStr() const noexcept
    {
        return "i" + std::to_string(data_) + "e";
    }

    static std::shared_ptr<BenCode_Integer> parse(const char* data, size_t length, size_t& pos) {
        if (pos >= length || data[pos] != 'i') {
            throw std::runtime_error("Expected integer 'i' at position " + std::to_string(pos) + " while parsing integer value");
        }
        pos++;
        const size_t start_pos = pos;
        
        while (pos < length && data[pos] != 'e') {
            if (pos == start_pos && data[pos] == '-') {
                pos++;
                continue;
            }
            if (!std::isdigit(data[pos])) {
                throw std::runtime_error("Invalid integer character");
            }
            pos++;
        }
        
        if (pos >= length) throw std::runtime_error("Unterminated integer");
        
        std::string number_str(data + start_pos, pos - start_pos);
        pos++;
        
        if (number_str.empty()) throw std::runtime_error("Empty integer");
        if (number_str == "-0") throw std::runtime_error("Negative zero");
        if (number_str.size() > 1 && number_str[0] == '0') throw std::runtime_error("Leading zero");
        
        return std::make_shared<BenCode_Integer>(std::stoll(number_str));
    }
};

class BenCode_String : public BenCode
{
private:
    std::string data_;
public:
    BenCode_String(std::string data) : data_(data) {}

    std::string getData() const noexcept//override
    {
        return data_;
    }

    std::string getStr() const noexcept override
    {
        return std::to_string(data_.length()) + ":" + data_;
    }

    static std::shared_ptr<BenCode_String> parse(const char* data, size_t length, size_t& pos) {
        if (pos >= length || !std::isdigit(data[pos])) {
            throw std::runtime_error("Expected string at position " + std::to_string(pos) + " while parsing string value");
        }
        
        const size_t start_pos = pos;
        while (pos < length && data[pos] != ':') {
            if (!std::isdigit(data[pos])) throw std::runtime_error("Invalid string length");
            pos++;
        }
        
        if (pos >= length) throw std::runtime_error("Unterminated string length");
        
        std::string length_str(data + start_pos, pos - start_pos);
        size_t str_length = std::stoull(length_str);
        pos++;
                
        std::string value(data + pos, str_length);
        pos += str_length;
        
        return std::make_shared<BenCode_String>(value);
    }
};

struct BenCode_Dictionary: public BenCode
{
private:
    std::map<std::string, std::shared_ptr<BenCode>> dict_data_;
public:
    uint64_t getSize() const noexcept
    {
        return dict_data_.size();
    }

    std::shared_ptr<BenCode> getValue(const std::string &key) const{
        if(dict_data_.find(key) == dict_data_.end())
        {
            throw std::runtime_error("Invalid bencode: no key " + key);
        }
        return dict_data_.find(key)->second;
    }

    std::string getStr() const noexcept override{
        std::string sub_ans = "";
        for(const auto& el : dict_data_)
        {
            sub_ans += BenCode_String(el.first).getStr() + el.second->getStr();
        }

        return "d" + sub_ans + "e";
    }

    void setValue(const std::string& key, std::shared_ptr<BenCode> value) {
        dict_data_[key] = std::move(value);
    }

    static std::shared_ptr<BenCode_Dictionary> parse(const char* data, size_t length, size_t& pos){
        if(pos >= length || data[pos] != 'd'){
            throw std::runtime_error("Expected dictionary 'd' at position " + std::to_string(pos));
        }

        pos++;
        auto dict = std::make_shared<BenCode_Dictionary>();

        while(pos < length && data[pos] != 'e')
        {
            auto key_string = BenCode_String::parse(data, length, pos);
            if (!key_string) {
                throw std::runtime_error("Dictionary key must be a string at position " + std::to_string(pos));
            }

            if (pos >= length) {
                throw std::runtime_error("Unexpected end of input in dictionary");
            }
            auto value_obj = BenCode::parse(data, length, pos);
            dict->setValue(key_string->getData(), std::move(value_obj));
        }
        if (pos >= length) {
            throw std::runtime_error("Unterminated dictionary: missing 'e'");
        }

        pos++;
        return dict;
    }

};


std::shared_ptr<BenCode> BenCode::parse(const char* data, size_t length, size_t& pos) {
    if (pos >= length) {
        throw std::runtime_error("Unexpected end of input at position " + std::to_string(pos));
    }
    
    const char current_char = data[pos];
    
    switch (current_char) {
        case 'i':
            return BenCode_Integer::parse(data, length, pos);
        case 'l':
            return BenCode_List::parse(data, length, pos);
        case 'd':
            return BenCode_Dictionary::parse(data, length, pos);
        default:
            if (std::isdigit(current_char)) {
                return BenCode_String::parse(data, length, pos);
            }
            throw std::runtime_error(std::string("Unexpected character '") + current_char + 
                                   "' at position " + std::to_string(pos));
    }
}

std::string GetInfoHash(const BenCode_Dictionary& FullInfo) {
    std::string info = FullInfo.getValue("info")->getStr();

    unsigned char hash_arr[20];
    SHA1((const unsigned char *)info.c_str(), info.length(), hash_arr);
    std::string res = "";
    for (int i = 0; i < 20; i++) {
        res += hash_arr[i];
    }

    return res;
}

std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& dict){
    std::string s = dynamic_pointer_cast<BenCode_String>(dynamic_pointer_cast<BenCode_Dictionary>(dict->getValue("info"))->getValue("pieces"))->getData();

    std::vector<std::string> result;
    for (int i = 0; i < s.length(); i += 20) {
        result.push_back(s.substr(i, 20));
    }
    return result;
}



