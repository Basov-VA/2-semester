#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <variant>
#include <list>
#include <map>
#include <sstream>
#include <memory>
#include <cstdint>
#include "peer.h"
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

namespace Bencode {
    std::shared_ptr<BenCode> parse_fucking_BenCode(const char* ben, uint64_t len, uint64_t &pos);
    std::vector<Peer> parsePeers(const std::string& PeersHashString);
    std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& nig);
    std::string GetInfoHash(const BenCode_Dictionary& FullInfo);

}
