#define MSG_BUF_SIZE 1024

enum ReqType {
    GET = 'G',
    SET = 'S',
    DROP = 'D'
};

struct Req {
    char type;
    char key_len;
    char* key;
    char value_len;
    char* value;
};


enum RespType {
    GET_OK = 'G',
    GET_MISSING = 'M',
    UPDATE_OK = 'U',
    ERR = 'E'
};

struct Resp {
    char type;
    char data_len;
    char* data;
};

Req* alloc_request();
char* pack_request(Req*);
Req* unpack_request(char*);
void free_request(Req*);

Resp* alloc_response();
char* pack_response(Resp*);
Resp* unpack_response(char*);
void free_response(Resp*);