#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>

TorrentFile LoadTorrentFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buf;
    buf << file.rdbuf();
    std::string data = buf.str();
    file.close();

    TorrentFile TrFile;
    size_t position = 0;

    auto root_dict = Bencode::BenCode_Dictionary::parse(data.c_str(),data.size(),position);
    TrFile.announce = std::dynamic_pointer_cast<Bencode::BenCode_String>(root_dict->getValue("announce"))->getData();
    TrFile.comment = std::dynamic_pointer_cast<Bencode::BenCode_String>(root_dict->getValue("comment"))->getData();
    TrFile.pieceLength = std::dynamic_pointer_cast<Bencode::BenCode_Integer>(std::dynamic_pointer_cast<Bencode::BenCode_Dictionary>(root_dict->getValue("info"))->getValue("piece length"))->getData();
    TrFile.length = std::dynamic_pointer_cast<Bencode::BenCode_Integer>(std::dynamic_pointer_cast<Bencode::BenCode_Dictionary>(root_dict->getValue("info"))->getValue("length"))->getData();
    TrFile.pieceHashes = GetPieceHashes(root_dict);
    TrFile.infoHash = GetInfoHash(*root_dict);
    return TrFile;
}


