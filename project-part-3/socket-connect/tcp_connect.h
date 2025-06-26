#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include "byte_tools.h"
#include <string>
#include <chrono>
#include <cstdint>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Обертка над низкоуровневой структурой сокета.
 */
enum class SocketStatus{
    Disconnected,
    Connected,
    Closed

};

class TcpConnect {
public:


    TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout);
    ~TcpConnect();

    /*
     * Установить tcp соединение.
     * Если соединение занимает более `connectTimeout` времени, то прервать подключение и выбросить исключение.
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man7/socket.7.html
     * - https://man7.org/linux/man-pages/man2/connect.2.html
     * - https://man7.org/linux/man-pages/man2/fcntl.2.html (чтобы включить неблокирующий режим работы операций)
     * - https://man7.org/linux/man-pages/man2/select.2.html
     * - https://man7.org/linux/man-pages/man2/setsockopt.2.html
     * - https://man7.org/linux/man-pages/man2/close.2.html
     * - https://man7.org/linux/man-pages/man3/errno.3.html
     * - https://man7.org/linux/man-pages/man3/strerror.3.html
     */
    void EstablishConnection();

    /*
     * Послать данные в сокет
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/send.2.html
     */
    void SendData(const std::string &data) const {
        const char *buf = data.c_str();
        int size_ = data.size();
        while(size_ > 0)
        {
            int send_ = send(sock_,buf,size_,0);
            if(send_ < 0)
            {
                throw std::runtime_error("Failed to send data");
            }
            buf += send_;
            size_ -= send_;
        }
    }

    std::string ReceiveDataWithDynamicSize(){
        char lenbuf[4];
        if (!ReceiveAll(lenbuf, 4)) {
            return "";
        }

        int len = BytesToInt(lenbuf);
        if (len <= 0) {
            return "";
        }

        std::string data(len, 0);
        char* buf = &data[0];
        if (!TcpConnect::ReceiveAll(buf, len)) {
            return "";
        }

        return data;
    }

    /*
     * Прочитать данные из сокета.
     * Если передан `bufferSize`, то прочитать `bufferSize` байт.
     * Если параметр `bufferSize` не передан, то сначала прочитать 4 байта, а затем прочитать количество байт, равное
     * прочитанному значению.
     * Первые 4 байта (в которых хранится длина сообщения) интерпретируются как целое число в формате big endian,
     * см https://wiki.theory.org/BitTorrentSpecification#Data_Types
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/poll.2.html
     * - https://man7.org/linux/man-pages/man2/recv.2.html
     */
    std::string ReceiveData(size_t bufferSize = 0) const {
        if (bufferSize == 0) {
            return ReceiveDynamicData();
        } else {
            return ReceiveFixedData(bufferSize);
        }
    }



    /*
     * Закрыть сокет
     */
    void CloseConnection();

    const std::string& GetIp() const;
    int GetPort() const;

    int WaitForSocket(int sockfd, std::chrono::milliseconds timeout);
    void SetBlocking(int sockfd);
    void SetNonBlocking(int sockfd);
    std::string ReceiveDynamicData() const {
        char lenbuf[4];
        if (!ReceiveAll(lenbuf, 4)) {
            return "";
        }

        int len = BytesToInt(lenbuf);
        if (len <= 0) {
            return "";
        }

        std::string data(len, 0);
        char* buf = &data[0];
        if (!ReceiveAll(buf, len)) {
            return "";
        }

        return data;
    }

    std::string ReceiveFixedData(size_t bufferSize) const {
        std::string data(bufferSize, 0);
        char* buf = &data[0];
        if (!ReceiveAll(buf, bufferSize)) {
            return "";
        }

        return data;
    }
    bool ReceiveAll(char* buffer, size_t size) const {
        while (size > 0) {
            struct timeval timeout;
            timeout.tv_sec = readTimeout_.count() / 1000;
            timeout.tv_usec = (readTimeout_.count() % 1000) * 1000;

            if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                throw std::runtime_error("Failed to set socket timeout");
            }

            int n = recv(sock_, buffer, size, 0);
            if (n < 0) {
                throw std::runtime_error("Failed to receive data");
            }
            if (n == 0) {
                return false;
            }

            buffer += n;
            size -= n;
        }
        return true;
    }

    struct sockaddr_in CreateSocketAddress() const;
    int StartConnection(const struct sockaddr_in& address) const;
private:


    const std::string ip_;
    const int port_;
    std::chrono::milliseconds connectTimeout_, readTimeout_;
    int sock_;
    SocketStatus status_;
};
