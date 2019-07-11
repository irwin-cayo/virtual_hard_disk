#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class Sdisk {
        public:
                Sdisk(string diskname, int numberofblocks, int blocksize);
                int getBlock(int blockNumber, string& buffer);
                int putBlock(int blockNumber, string buffer);
                int getNumberOfBlocks(); // accessor
                int getBlockSize();      // accessor
        private:
                string diskName;         //file name of software-disk
                int numberOfBlocks;      // number of blocks on disk
                int blockSize;           // block size in bytes
};

Sdisk::Sdisk(string diskname, int numberofblocks, int blocksize) {
        //check if the disk already exists
        diskName = diskname;
        blockSize = blocksize;
        numberOfBlocks = numberofblocks;
        ifstream infile;
        infile.open(diskName.c_str());

        //if doesn't exist create and store disk info
        if (infile.good()) {
                cout<< "Disk already exists" << endl;
               infile.close();
               return;
        }
                //replace empty spaces with hashtags
                string emptyField = "#";
                ofstream outfile;
                outfile.open(diskName.c_str());
                for (int i = 0; i < (numberOfBlocks*blockSize)-1; i++){
                    outfile << emptyField;
                }
                outfile.close();

}

int Sdisk::getBlock(int blockNumber, string& buffer){
        fstream iofile(diskName.c_str());
        //buffer
        iofile.seekg(blockSize*blockNumber);
        buffer.clear();

        char x;
        //write to buffer
        for (int i = (blockNumber*blockSize); i < (blockSize*blockNumber)+ (blockSize-1); i++ ) {
                iofile.get(x);
                buffer.push_back(x);
        }
        iofile.close();
}

int Sdisk::putBlock(int blockNumber, string buffer){
        fstream iofile;
        iofile.open(diskName.c_str());
        //locate size
        iofile.seekg(blockNumber*blockSize);
        //write to disk
        for (int i = 0; i < buffer.size(); i++) {
                iofile << buffer[i];
        }
        iofile.close();
}


int Sdisk::getNumberOfBlocks(){
    return numberOfBlocks;
}

int Sdisk::getBlockSize(){
    return blockSize;
}

int main()
{
  Sdisk disk1("test1",16,32);
  string block1, block2, block3, block4, block5;
  for (int i=1; i<=32; i++) block1=block1+"1";
  for (int i=1; i<=32; i++) block2=block2+"2";
  disk1.putBlock(4,block1);
  disk1.getBlock(4,block3);
  cout << "Should be 32 1s : ";
  cout << block3 << endl;
  disk1.putBlock(8,block2);
  disk1.getBlock(8,block4);
  cout << "Should be 32 2s : ";
  cout << block4 << endl;
  disk1.getBlock(0, block5);
  cout << "data in block 0 : ";
  cout << block5 << endl;
  cout << "Should be 32 1s : ";
  cout << block3 << endl;
  disk1.putBlock(8,block2);
  disk1.getBlock(8,block4);
  cout << "Should be 32 2s : ";
  cout << block4 << endl;
  disk1.getBlock(0, block5);
  cout << "data in block 0 : ";
  cout << block5 << endl;
}
