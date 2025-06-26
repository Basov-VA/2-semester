#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile &tf) {
    int count_of_pieces = tf.length / tf.pieceLength;
    for (int i = 0; i < count_of_pieces; ++i) {
        int current_length = tf.pieceLength;
        if(i == count_of_pieces - 1)
        {
            current_length = tf.length % tf.pieceLength;
        }
        remainPieces_.push(std::make_shared<Piece>(i, current_length, tf.pieceHashes[i]));
    }
}



PiecePtr PieceStorage::GetNextPieceToDownload() {
    if(this->remainPieces_.size() == 0)
    {
        return nullptr;
    }
    else
    {
        PiecePtr current_front_Piece = remainPieces_.front();
        remainPieces_.pop();
        return current_front_Piece;
    }
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    if(piece->HashMatches() == false)
    {
        piece->Reset();
        std::cerr << "Hashe;s razniy";
        return;
    }
    SavePieceToDisk(piece);
    while (remainPieces_.size() != 0)
    {
        remainPieces_.pop();
    }
}

bool PieceStorage::QueueIsEmpty() const {
    return remainPieces_.size() == 0;
}

size_t PieceStorage::TotalPiecesCount() const {
    return remainPieces_.size();
}

void PieceStorage::SavePieceToDisk(PiecePtr piece) {
    // Эта функция будет переопределена при запуске вашего решения в проверяющей системе
    // Вместо сохранения на диск там пока что будет проверка, что часть файла скачалась правильно
    std::cout << "Downloaded piece " << piece->GetIndex() << std::endl;
    std::cerr << "Clear pieces list, don't want to download all of them" << std::endl;
    while (!remainPieces_.empty()) {
        remainPieces_.pop();
    }
}

void PieceStorage::AddiPieceToQueue(const PiecePtr &ptr) {
    this->remainPieces_.push(ptr);
}
