#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>

TorrentTracker::TorrentTracker(const std::string &url): url_(url) {}

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
    peers_ = std::move(Bencode::ParsePeers(res.text));

}

const std::vector<Peer> &TorrentTracker::GetPeers() const {
    return peers_;
}
