#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace {
    constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash):
index_(index),
length_(length),
hash_(std::move(hash)),
blocks_()
{
    if(length % BLOCK_SIZE == 0)
    {
        blocks_.resize(length / BLOCK_SIZE);
    }
    else
    {
        blocks_.resize(length / BLOCK_SIZE + 1);
    }
    for(size_t i = 0; i < blocks_.size(); ++i)
    {
        blocks_[i].length = std::min(BLOCK_SIZE, length - i * BLOCK_SIZE);
        blocks_[i].status = Block::Status::Missing;
        blocks_[i].offset = i * BLOCK_SIZE;
        blocks_[i].piece = index;
    }
}


bool Piece::HashMatches() const {
    return GetDataHash() == this->hash_;
}

Block* Piece::FirstMissingBlock() {
    for (auto& block : blocks_) {
        if (block.status == Block::Status::Missing) {
            return &block;
        }
    }
    return nullptr;
}

size_t Piece::GetIndex() const {
    return this->index_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data) {
size_t index_of_element = blockOffset/BLOCK_SIZE;
blocks_[index_of_element].data = std::move(data);
blocks_[index_of_element].status = Block::Status::Retrieved;
}

bool Piece::AllBlocksRetrieved() const{
    for(auto &block: blocks_)
    {
        if(block.status != Block::Status::Retrieved)
        {
            return false;
        }
    }
    return true;
}

std::string Piece::GetData() const {
    std::string data_ = "";
    for(const auto &block : blocks_)
    {
        data_ = data_ + block.data;
    }
    return data_;
}

std::string Piece::GetDataHash() const {
    return CalculateSHA1(GetData());
}

const std::string &Piece::GetHash() const {
    return this->hash_;
}

void Piece::Reset() {
    for(auto &block : blocks_)
    {
        block.status = Block::Status::Missing;
        block.data.clear();
    }
}
