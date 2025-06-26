#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>

TorrentFile LoadTorrentFile(const std::string& filename)
{
    std::ifstream z(filename, std::ios::binary);
    std::stringstream b;
    b << z.rdbuf();
    std::string data = b.str();
    z.close();

    TorrentFile Ans;
    uint64_t position = 0;

    std::shared_ptr<BenCode_Dictionary> dict = std::dynamic_pointer_cast<BenCode_Dictionary>(Bencode::parse_fucking_BenCode(data.c_str(),data.size(),position));
    Ans.announce = std::dynamic_pointer_cast<BenCode_String>(dict->get("announce"))->get_s();
    Ans.comment = std::dynamic_pointer_cast<BenCode_String>(dict->get("comment"))->get_s();
    Ans.pieceLength = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(dict->get("info"))->get("piece length"))->get_i();
    Ans.length = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(dict->get("info"))->get("length"))->get_i();
    Ans.pieceHashes = Bencode::GetPieceHashes(dict);
    Ans.infoHash = Bencode::GetInfoHash(*dict);
    return Ans;
}


