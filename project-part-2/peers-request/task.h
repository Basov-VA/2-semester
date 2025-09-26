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

class TorrentTracker {
public:

    TorrentTracker(const std::string& url):
    url_(url),
    peers_(){}

    std::string ParsePeers(std::string ben_code_string){
        size_t pos = 0;
        auto dict = BenCode_Dictionary::parse(ben_code_string.c_str(),ben_code_string.size(),pos);
        auto peersString = std::static_pointer_cast<BenCode_String>(dict->getValue("peers"));
        std::string res = peersString->getStr();
        return res;
    }

    void UpdatePeers(const TorrentFile& tf, std::string peerId, int port){
        cpr::Response res = cpr::Get(
            cpr::Url(tf.announce),
            cpr::Parameters{
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

        std::string peersStr = ParsePeers(res.text);

        for(int i = 0; i < peersStr.size(); i+= 6)
        {
            std::string ip = std::to_string(static_cast<uint8_t>(peersStr[i])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 1])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 2])) + '.' + 
                             std::to_string(static_cast<uint8_t>(peersStr[i + 3]));
            uint8_t port = (static_cast<uint8_t>(peersStr[i + 4]) << 8) + static_cast<uint8_t>(peersStr[i + 5]);
            peers_.emplace_back(ip, port);
        }
    }
    const std::vector<Peer>& GetPeers() const {
        return peers_;
    }

private:
    std::string url_;
    std::vector<Peer> peers_;
};

TorrentFile LoadTorrentFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buf;
    buf << file.rdbuf();
    std::string data = buf.str();
    file.close();

    TorrentFile TrFile;
    size_t position = 0;

    auto root_dict = BenCode_Dictionary::parse(data.c_str(),data.size(),position);
    TrFile.announce = std::dynamic_pointer_cast<BenCode_String>(root_dict->getValue("announce"))->getData();
    TrFile.comment = std::dynamic_pointer_cast<BenCode_String>(root_dict->getValue("comment"))->getData();
    TrFile.pieceLength = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(root_dict->getValue("info"))->getValue("piece length"))->getData();
    TrFile.length = std::dynamic_pointer_cast<BenCode_Integer>(std::dynamic_pointer_cast<BenCode_Dictionary>(root_dict->getValue("info"))->getValue("length"))->getData();
    TrFile.pieceHashes = GetPieceHashes(root_dict);
    TrFile.infoHash = GetInfoHash(*root_dict);
    return TrFile;
}

