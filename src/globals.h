#ifndef GLOBALS_H
#define GLOBALS_H

class Network;
class Synchronization;
class Repository;

extern Network* networkModule;
extern Synchronization* currentSyncModule;  // TODO: move responsibility to a getter in Repository?
extern Repository* repo;

#endif // GLOBALS_H
