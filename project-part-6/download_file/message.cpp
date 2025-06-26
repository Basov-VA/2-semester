#include "message.h"
#include "byte_tools.h"


Message Message::Init(MessageId id, const std::string &payload) {
    Message Message_;
    Message_.id = id;
    Message_.payload = payload;
    Message_.messageLength = sizeof(uint32_t) + sizeof(uint8_t) + payload.size();
    return Message_;

}

Message Message::Parse(const std::string &messageString) {
    Message Message_;
    if(messageString.empty())
    {
        Message_.id = MessageId::KeepAlive;
        Message_.messageLength = 0;
        Message_.payload = "";
        return Message_;
    }
    Message_.id = static_cast<MessageId>(messageString[0]);
    Message_.messageLength = static_cast<size_t>(messageString.size());
    Message_.payload = messageString.substr(1,Message_.messageLength - 1);
    return Message_;
}


std::string Message::ToString() const {
    std::string ans;
    if(this->id == MessageId::KeepAlive)
    {
        return "\x00\x00\x00\x00";
    }
    ans += IntToBytes(this->messageLength);
    ans += static_cast<char>(this->id);
    ans += this->payload;
    return ans;
}