#include <cstdbool>


typedef struct {
    char* host;
    char* port;

    int socket_fd;
} ArowwDB;

typedef struct {
    bool is_ok;
    char* value;
    char* error_msg;

} ArowwResult;


extern "C" ArowwDB* aroww_init(char* host, char* port);
extern "C" void aroww_close(ArowwDB* db);

extern "C" ArowwResult* aroww_get(ArowwDB* db, char* key);
extern "C" ArowwResult* aroww_set(ArowwDB* db, char* key, char* value);
extern "C" ArowwResult* aroww_drop(ArowwDB* db, char* key);
extern "C" void aroww_free_result(ArowwResult* res);