#include "Synchronization.h"

#include "../build/Interface.h"

using namespace std;
using namespace disco_plat;

Synchronization::Synchronization()
{
    leftNb = &networkModule->getMyLeftInterface();
    rightNb = &networkModule->getMyRightInterface();

}

void Synchronization::newResult(const char* data, int dataLenght) {

    blob message;
    message.sourceNode = networkModule->getMyID();
    message.computationID = computationID;
    message.messageType = RESULT;

    blob::_data_seq blobData(dataLenght);

    for(int i = 0; i < dataLenght; ++i) {
        blobData[i] = data[i];
    }

    message.data = blobData;
    message.dataLength = dataLenght;

    rightNb->Boomerang(message);
}



