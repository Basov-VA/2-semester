#include "byte_tools.h"
#include "peer_connect.h"
#include "message.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>

using namespace std::chrono_literals;

PeerConnect::PeerConnect(const Peer& peer, const TorrentFile &tf, std::string selfPeerId) :
        tf_(tf),
        socket_(peer.ip,peer.port,500ms,500ms),
        selfPeerId_(selfPeerId),
        terminated_(false),
        choked_(true){}

void PeerConnect::Run() {
    while (!terminated_) {
        if (EstablishConnection()) {
            std::cout << "Connection established to peer" << std::endl;
            MainLoop();
        } else {
            std::cerr << "Cannot establish connection to peer" << std::endl;
            Terminate();
        }
    }
}


void PeerConnect::PerformHandshake() {
        this->socket_.EstablishConnection();
        std::string pstr = "BitTorrent protocol";
        std::string reserved = "00000000";
        std::string infohash_ = this->tf_.infoHash;
        std::string peerId = this->selfPeerId_;
        std::string handshake = ((char)19) + pstr + reserved + infohash_ + peerId;

        this->socket_.SendData(handshake);
        std::string ans_from_another = this->socket_.ReceiveData(68);
        if (ans_from_another.substr(28, 20) != this->tf_.infoHash) {
            throw std::runtime_error("InfoHashes don't match");
        }
        if (ans_from_another[0] != '\x13' || ans_from_another.substr(1, 19) != "BitTorrent protocol") {
            throw std::runtime_error("Handshake failed");
        }

}

bool PeerConnect::EstablishConnection() {
    try {
        PerformHandshake();
        ReceiveBitfield();
        SendInterested();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to establish connection with peer " << socket_.GetIp() << ":" <<
            socket_.GetPort() << " -- " << e.what() << std::endl;
        return false;
    }
}

void PeerConnect::ReceiveBitfield() {
        std::string recieved_data = this->socket_.ReceiveData();
        if((int)recieved_data[0] == 20)
        {
            recieved_data = this->socket_.ReceiveData();
        }
        MessageId id_ = static_cast<MessageId>((int) recieved_data[0]);
        if(id_ == MessageId::BitField){
            this->piecesAvailability_ = PeerPiecesAvailability(recieved_data.substr(1, recieved_data.size() - 1));
        }
        if (id_ == MessageId::Unchoke) {
            this->choked_ = false;
        }
}

void PeerConnect::Terminate() {
    std::cerr << "Terminate" << std::endl;
    terminated_ = true;
}

void PeerConnect::MainLoop() {
    /*
     * При проверке вашего решения на сервере этот метод будет переопределен.
     * Если вы провели handshake верно, то в этой функции будет работать обмен данными с пиром
     */
    std::cout << "Dummy main loop" << std::endl;
    Terminate();
}

PeerPiecesAvailability::PeerPiecesAvailability() {}

PeerPiecesAvailability::PeerPiecesAvailability(std::string bitfield):
    bitfield_(bitfield){}

bool PeerPiecesAvailability::IsPieceAvailable(size_t pieceIndex) const {
    return this->bitfield_[pieceIndex / 8] && (1 << (7 - (pieceIndex % 8)));
}

void PeerPiecesAvailability::SetPieceAvailability(size_t pieceIndex) {
    this->bitfield_[pieceIndex / 8] = this->bitfield_[pieceIndex / 8] | (1 << (7 - (pieceIndex % 8)));
}

size_t PeerPiecesAvailability::Size() const {
    int ans = 0;
    for(int i = 0; i < this->bitfield_.size(); ++i)
    {
        int byte = bitfield_[i];
        for(int j = 0; j < 8; ++j)
        {
            ans += (byte >> j) & 1;
        }
    }
    return ans;
}


void PeerConnect::SendInterested() {
    std::string ans = IntToBytes(1) + static_cast<char>(MessageId::Interested);
    this->socket_.SendData(ans);
}

