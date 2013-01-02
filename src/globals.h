#ifndef GLOBALS_H
#define GLOBALS_H

class Network;
class Synchronization;
class Repository;

extern Network* networkModule;
extern Repository* repo;


#define DEFAULT_PORT 1234

#define ERR_INVALID_ARGUMENTS 1
#define ERR_COMMAND_TERMINATE 2

#define USAGE_INFO "usage: disco-plat port [address]"



// slotA meaning as flag at INSTANCE_ANNOUNCEMENT message type
#define BLOB_SA_IA_NONE 0
#define BLOB_SA_IA_INIT_RESUME 1

#define BLOB_CID_GLOBAL_ID 0

#endif // GLOBALS_H
