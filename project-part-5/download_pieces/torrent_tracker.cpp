#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>

TorrentTracker::TorrentTracker(const std::string &url): url_(url) {}

std::string TorrentTracker::get_str_peers_dlya_daunov(std::string ben_code_string)
{
    uint64_t pos = 0;
    std::shared_ptr<BenCode_Dictionary> dict = std::dynamic_pointer_cast<BenCode_Dictionary>(Bencode::parse_fucking_BenCode(ben_code_string.c_str(),ben_code_string.size(),pos));
    std::shared_ptr<BenCode_String> sub_res = std::dynamic_pointer_cast<BenCode_String>(dict->get("peers"));
    std::string res = sub_res->get_s();
    return res;
}

void TorrentTracker::UpdatePeers(const TorrentFile &tf, std::string peerId, int port) {
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
    std::vector<Peer> ansi_ = Bencode::parsePeers(data);
    peers_ = std::move(ansi_);

}


const std::vector<Peer> &TorrentTracker::GetPeers() const {
    return peers_;
}
