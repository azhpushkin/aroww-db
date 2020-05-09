#include <cstdlib>
#include <cstring>

#include "messages.hpp"


char* write_str(char* dest, char* src, char len) {
    dest[0] = len;
    ++dest;
    
    strncpy(dest, src, len);
    return dest + len;
}

char* read_str(char* from, char** target_str, char* target_len) {
    *target_len = from[0];
    ++from;
    
    *target_str = (char*)malloc(sizeof(char) * (*target_len));
    strncpy(*target_str, from, (*target_len));
    return from + (*target_len);
}

// REQUEST
char* pack_request(Req* req) {
    char* buf = (char*)malloc(sizeof(char) * MSG_BUF_SIZE);
    buf[0] = req->type;

    char* tmp = write_str(buf+1, req->key, req->key_len);
    if (req->value != NULL) {
        write_str(tmp, req->value, req->value_len);
    }

    return buf;
}
Req* unpack_request(char* buf) {
    Req* req = (Req*) malloc (sizeof(Req));
    req->type = buf[0];
    req->key_len = 0;
    req->key = NULL;
    req->value_len = 0;
    req->value = NULL;
    
    
    
    char* tmp = read_str(buf+1, &(req->key), &(req->key_len));
    if (tmp[0] > 0) {
        read_str(tmp, &(req->value), &(req->value_len));
    }
    return req;
}
void free_request(Req* req) {
    free(req->key);
    free(req->value);
    free(req);
}

// RESPONSE

char* pack_response(Resp* resp) {
    char* buf = (char*)malloc(sizeof(char) * MSG_BUF_SIZE);
    buf[0] = resp->type;

    if (resp->data_len > 0) {
        write_str(buf+1, resp->data, resp->data_len);
    }
    return buf;
}
Resp* unpack_response(char* buf) {
    Resp* resp = (Resp*) malloc (sizeof(Resp));
    resp->type = buf[0];
    resp->data_len = 0;
    resp->data = NULL;
    
    read_str(buf+1, &(resp->data), &(resp->data_len));
    return resp;
}
void free_response(Resp* resp) {
    free(resp->data);
    free(resp);
}