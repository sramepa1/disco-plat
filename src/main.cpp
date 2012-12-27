
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    
    // TODO: maybe we will need more args...
    if(argc < 2) {
        cerr << "usage: disco-plat port [address]" << endl;        
        return 1;
    }
    
    return 0;
}
