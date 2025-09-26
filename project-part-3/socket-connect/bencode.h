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

void sha1(const char *data, size_t length, unsigned char *hash);

namespace Bencode {
    class BenCode
    {
    public:
        virtual ~BenCode() = default;
        virtual std::string getStr() const = 0;
        static std::shared_ptr<BenCode> parse(const char* data, size_t length, size_t& pos);
        //virtual const int64_t getData() const = 0;
        //virtual const std::string getData() const = 0;
    };

    class BenCode_Integer : public BenCode
    {
    private:
        int64_t data_;
    public:
        BenCode_Integer(int64_t data);
        
        int64_t getData() const;

        std::string getStr() const override;

        static std::shared_ptr<BenCode_Integer> parse(const char* data, size_t length, size_t& pos);
    };

    class BenCode_String : public BenCode
    {
    private:
        std::string data_;
    public:
        BenCode_String(std::string data);

        std::string getData() const;

        std::string getStr() const override;

        static std::shared_ptr<BenCode_String> parse(const char* data, size_t length, size_t& pos);
    };

    class BenCode_List : public BenCode
    {
    private:
        std::vector<std::shared_ptr<BenCode>> list_data_;
    public:
        uint64_t getSize() const;

        std::shared_ptr<BenCode> getValue(size_t  index) const;

        void addValue(std::shared_ptr<BenCode> element);

        std::string getStr() const;

        static std::shared_ptr<BenCode_List> parse(const char* data, size_t length, size_t& pos);

    };

    class BenCode_Dictionary: public BenCode
    {
    private:
        std::map<std::string, std::shared_ptr<BenCode>> dict_data_;
    public:
        uint64_t getSize() const;

        std::shared_ptr<BenCode> getValue(const std::string &key) const;

        std::string getStr() const override;

        void setValue(const std::string& key, std::shared_ptr<BenCode> value);

        static std::shared_ptr<BenCode_Dictionary> parse(const char* data, size_t length, size_t& pos);
    };

    std::vector<std::string> GetPieceHashes(const std::shared_ptr<BenCode_Dictionary>& dict);
    
    std::vector<Peer> ParsePeers(const std::string ben_code_string);

    std::string GetInfoHash(const BenCode_Dictionary& FullInfo);
}
