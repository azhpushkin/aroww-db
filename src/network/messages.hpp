#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <memory>



enum MsgType {
    GetReqType = 'G',
    SetReqType = 'S',
    DropReqType = 'D',
    GetOkRespType = 'g',
    GetMissingRespType = 'm',
    UpdateOkRespType = 's',
    ErrorRespType = 'e',
};


class Message {
public:
    std::string pack_message();
    static std::unique_ptr<Message> unpack_message(std::string);
    virtual char get_flag() = 0;
private:
    virtual void pack_fields(std::stringstream& ss);
    virtual void unpack_fields(std::stringstream& ss);
};

class MsgGetReq: public Message {
public:
    std::string key;
    char get_flag() { return GetReqType; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};

class MsgSetReq: public Message {
public:
    std::string key;
    std::string value;
    char get_flag() { return SetReqType; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};

class MsgDropReq: public Message {
public:
    std::string key;
    char get_flag() { return DropReqType; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);    
};


class MsgGetOkResp: public Message {
public:
    std::string val;
    char get_flag() { return GetOkRespType; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};

class MsgGetMissingResp: public Message {
public:
    char get_flag() { return GetMissingRespType; };
};

class MsgUpdateOkResp: public Message {
public:
    char get_flag() { return UpdateOkRespType; };
};

class MsgErrorResp: public Message {
public:
    std::string error_msg;
    char get_flag() { return ErrorRespType; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss); 
};
