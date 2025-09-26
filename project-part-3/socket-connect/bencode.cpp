#include "bencode.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <openssl/sha.h>

void sha1(const char *data, size_t length, unsigned char *hash) {
    SHA1((const unsigned char *) data, length, hash);
}

namespace Bencode {

    // ============================================================================
    // Реализация базового класса BenCode
    // ============================================================================

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

    // ============================================================================
    // Реализация класса Integer
    // ============================================================================
    
    BenCode_Integer::BenCode_Integer(int64_t data): data_(data){};

    int64_t BenCode_Integer::getData() const {
        return data_;
    }

    std::string BenCode_Integer::getStr() const {
        return "i" + std::to_string(data_) + "e";
    }

    std::shared_ptr<BenCode_Integer> BenCode_Integer::parse(const char* data, size_t length, size_t& pos) {
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

    // ============================================================================
    // Реализация класса String
    // ============================================================================

    BenCode_String::BenCode_String(std::string data) : data_(data) {}

    std::string BenCode_String::getData() const {
        return data_;
    }

    std::string BenCode_String::getStr() const {
        return std::to_string(data_.length()) + ":" + data_;
    }

    std::shared_ptr<BenCode_String> BenCode_String::parse(const char* data, size_t length, size_t& pos) {
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

    // ============================================================================
    // Реализация класса List
    // ============================================================================

    uint64_t BenCode_List::getSize() const {
        return list_data_.size();
    }

    std::shared_ptr<BenCode> BenCode_List::getValue(size_t  index) const {
        if(index >= list_data_.size()){
            throw std::runtime_error("Invalid bencode: index out of range");
        }
        return list_data_[index];
    }

    void BenCode_List::addValue(std::shared_ptr<BenCode> element) {
        list_data_.push_back(std::move(element));
    }

    std::string BenCode_List::getStr() const {
        std::string res = "";
        for(const auto& el : list_data_)
        {
            res += el->getStr();
        }
        return "l" + res + "e";
    }

    std::shared_ptr<BenCode_List> BenCode_List::parse(const char* data, size_t length, size_t& pos) {
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

    // ============================================================================
    // Реализация класса Dictionary
    // ============================================================================

    uint64_t BenCode_Dictionary::getSize() const {
        return dict_data_.size();
    }
    
    std::shared_ptr<BenCode> BenCode_Dictionary::getValue(const std::string &key) const {
        if(dict_data_.find(key) == dict_data_.end())
        {
            throw std::runtime_error("Invalid bencode: no key " + key);
        }
        return dict_data_.find(key)->second;
    }
    
    std::string BenCode_Dictionary::getStr() const {
        std::string sub_ans = "";
        for(const auto& el : dict_data_)
        {
            sub_ans += BenCode_String(el.first).getStr() + el.second->getStr();
        }

        return "d" + sub_ans + "e";
    }

    void BenCode_Dictionary::setValue(const std::string& key, std::shared_ptr<BenCode> value) {
        dict_data_[key] = std::move(value);
    }


    std::shared_ptr<BenCode_Dictionary> BenCode_Dictionary::parse(const char* data, size_t length, size_t& pos){
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
    



    std::vector<Peer> ParsePeers(std::string ben_code_string){
        size_t pos = 0;
        auto dict = BenCode_Dictionary::parse(ben_code_string.c_str(),ben_code_string.size(),pos);
        auto BenCodeString = std::static_pointer_cast<BenCode_String>(dict->getValue("peers"));
        std::string peersStr = BenCodeString->getStr();
        std::vector<Peer> res;
        for(int i = 0; i < peersStr.size(); i+= 6)
        {
            std::string ip = std::to_string(static_cast<uint8_t>(peersStr[i])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 1])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 2])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 3]));
            uint8_t port = (static_cast<uint8_t>(peersStr[i + 4]) << 8) + static_cast<uint8_t>(peersStr[i + 5]);
            res.emplace_back(ip, port);
        }
        return res;
    }

    std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& dict){
        std::string s = dynamic_pointer_cast<BenCode_String>(dynamic_pointer_cast<BenCode_Dictionary>(dict->getValue("info"))->getValue("pieces"))->getData();

        std::vector<std::string> result;
        for (int i = 0; i < s.length(); i += 20) {
            result.emplace_back(s.substr(i, 20));
        }
        return result;
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
}
