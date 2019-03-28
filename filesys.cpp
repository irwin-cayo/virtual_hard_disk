//Irwin Cayo
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "Sdisk.h"
using namespace std;


class filesys: public Sdisk {
        public: 
        filesys(string, int, int);
        int fsclose();
        int fssynch();
        int newfile(string file);
        int rmfile(string file);
        int getfirstblock(string file);
        int addblock(string file, string block);
        int delblock(string file, int blocknumber);
        int readblock(string file, int blocknumber, string& buffer);
        int writeblock(string file, int blocknumber, string buffer);
        int nextblock(string file, int blocknumber);
        static vector<string> block(string buffer, int b);
        
        private:
        bool checkBlock(int, string);
        int createRoot();
        int createFat();
        int rootsize;                    // max number of enrties in root
        int fatsize;                     // number of blocks occupied by FAT
        vector<string> filename;         // filenames in ROOT
        vector<int> firstblock;          // firstblocks in ROOT
        vector<int> fat;                 // FAT
};

vector<string> filesys::block(string buffer, int b) {
// blocks the buffer into a list of blocks of size b

vector<string> blocks;
int numberofblocks=0;

if (buffer.length() % b == 0) 
   { numberofblocks= buffer.length()/b;
   }
else 
   { numberofblocks= buffer.length()/b +1;
   }

string tempblock;
for (int i=0; i<numberofblocks; i++)
    { tempblock=buffer.substr(b*i,b);
      blocks.push_back(tempblock);
    }

int lastblock=blocks.size()-1;

for (int i=blocks[lastblock].length(); i<b; i++)
    { blocks[lastblock]+="#";
    }

return blocks;

}
    
filesys::filesys(string diskName, int numberOfBlocks, int blockSize): Sdisk(diskName, numberOfBlocks, blockSize) {
    //cout << "filesys constructor executed\n";
	rootsize = getBlockSize() / 11;
	int bytesRequired = getNumberOfBlocks() * 5;
	fatsize = bytesRequired / (getBlockSize() + ((bytesRequired % getBlockSize()) > 0)); // +1
	string bufferCheck;
	getBlock(1, bufferCheck);
	    if (bufferCheck[0] == '#'){
	        //filesystem does not exist, create a default root directory
	        cout << "creating defualt" << endl;
	        for (int i = 0; i < rootsize; i++) {
	            filename.push_back("xxxxx");
	            firstblock.push_back(0);
	        }
	        // create default FAT
	        fat.push_back(fatsize+2);
	        fat.push_back(1);
	        for (int i = 0; i < fatsize; i++) {
	            fat.push_back(1);
	        }
	        for (int i = fatsize +2; i < getNumberOfBlocks(); i++) {
	            fat.push_back(i + 1);
	        }
	        fat[getNumberOfBlocks() -1] = 0;
	    }
	    else {
	        cout << "file system already exists" << endl;
	        
	        string buffer1; //root
	        string buffer2; //FAT
	        string temp;
	        getBlock(1, buffer1);  //get root into buffer1
	        for (int i = 0; i < fatsize; i++) {                                 
	            getBlock(2 + i, temp);        //FAT into temp
	            buffer2 += temp;              //entire FAT into buffer2 
	        } 
	        istringstream ibuffer1;
	        istringstream ibuffer2;
	        ibuffer1.str(buffer1);
	        ibuffer2.str(buffer2);
	        
	        //read root from sdisk
	        string temp1; 
	        int temp2;
	        for (int i = 0; i < rootsize; i++) {
	            ibuffer1 >> temp1 >> temp2;
	            filename.push_back(temp1);
	            firstblock.push_back(temp2);
	        }
	        
	        //read FAT from sdisk
	        int n;
	        for ( int i = 0; i < getNumberOfBlocks(); i++) {
	            ibuffer2 >> n;
	            fat.push_back(n);
	        }
	        
	    }
	  fssynch();
}



int filesys::fssynch() {
    string bufferRoot, bufferFAT;
    
    ostringstream rstream;
    for (int i = 0; i < rootsize; i++) {
        rstream << filename[i] << " " << firstblock[i] << " ";
    }
    bufferRoot = rstream.str();  // enitre root stored in bufferRoot
    //block bufferRoot
    
    //write bufferRoot to disk
    vector<string> blocks1 = block(bufferRoot, getBlockSize());
    putBlock(1, blocks1[0]);
    
    
    //create buffer for FAT
    ostringstream FATstream;
    for (int i = 0; i < rootsize; i++) {
        FATstream << fat[i] << " ";
    }
    bufferFAT = FATstream.str();
    //write bufferFAT to disk
    vector<string> blocks2 = block(bufferFAT, getBlockSize());
    for (int i = 0; i < blocks2.size(); i++) {
        putBlock(2 + i, blocks2[i]);
    }
}

int filesys::newfile(string file) {
    for (int i = 0; i < filename.size(); i++) {
        if (filename[i] == "xxxxx") {
            filename[i] = file;
            firstblock[i] = 0;
            fssynch();
            return 1;
        }
    }
    cout << "no room on disk" << endl;
    return 0;
}

int filesys::rmfile(string file) {
    for (int i = 0; i < filename.size(); i++) {
        if (filename[i] == file) {
            filename[i] = "xxxxx";
            fssynch();
            return 1;
        }
    }
    return 0;
}

int filesys::getfirstblock(string file) {
    for (int i = 0; i < filename.size(); i++) {
        if (filename[i] == file) {
            return firstblock[i];
        }
    }
    return 0;
}

int filesys::addblock(string file, string block) {
    //add block to end of file and returns block #
    int allocate = fat[0];
    if (allocate == 0) {
        cout << "You are out of space" << endl;
        return -1;
    }
    fat[0] = fat[fat[0]];
    fat[allocate] = 0;
    int fblock = getfirstblock(file);
    if (fblock == -1) {
        cout << "no file" << endl;
        return 0;
    }
    if (fblock == 0) {
        for (int i = 0; i < filename.size(); i++) {
            if (filename[i] == file) {
                firstblock[i] = allocate;
                break;
            }
        }
    } else {
        while(fat[fblock] != 0) {
            fblock = fat[fblock];
        }
        fat[fblock] = allocate;
    }
    fssynch();
    putBlock(allocate, block);
    return allocate;
}

bool filesys::checkBlock(int bnum, string file) {
    int fblock = getfirstblock(file);
    while (fblock != 0) {
        if (fblock == bnum) {
            return true;
        }
        fblock = fat[fblock];
    }
    return false;
}

int filesys::delblock(string file, int blocknumber) {
    if ( checkBlock(blocknumber, file) == false ) {
        cout << "No can do" << endl;
        return 0;
    }
    int fblock = getfirstblock(file);
    if (fblock == blocknumber) {
        for (int i = 0; i < filename.size(); i++) {
            if(filename[i] == file) {
                firstblock[i] = fat[firstblock[i]];
                break;
            }
        }
    }
    else { 
        while (fat[fblock] != blocknumber) {
            fblock = fat[fblock];
        }
        fat[fblock] = fat[fat[fblock]];
    }
    fat[blocknumber] = fat[0];
    fat[0] = blocknumber;
    fssynch();
    return 1;
    
}

int filesys::fsclose() {
    fssynch();
}

int filesys::readblock(string file, int blocknumber, string& buffer) {
    int curBlock = getfirstblock(file);

    // File Does Not Exist
    if (curBlock == 0) {
        return 0;
    }
    
    // Loop Through All Blocks Owned By File
    for ( curBlock ; curBlock != 0; curBlock = fat[curBlock]) {
        if (curBlock == blocknumber) {
            return getBlock(blocknumber, buffer);
        }
    }    
	return 0;
}

int filesys::writeblock(string file, int blocknumber, string buffer) {
    int curBlock = getfirstblock(file);

    // File Does Not Exist
    if (curBlock == 0)
        return 0;

    // Loop Through File Blocks Looking For blockNum
    for ( curBlock ; curBlock != 0; curBlock = fat[curBlock]) {
        if (curBlock == blocknumber) {
            return putBlock(blocknumber, buffer);
        }
    }
	return 0;
}

int filesys::nextblock(string file, int blocknumber) {
    int curBlock = getfirstblock(file);

    // File Does Not Exist
    if (curBlock == 0)
        return 0;

    // Search File For Given Block Number
    for ( curBlock; curBlock != 0; curBlock = fat[curBlock])
        if (curBlock == blocknumber)
            // Returns The Block Number Of The Succeeding Block
            return fat[curBlock];

    // Handles blockNum = 0, end of blocks
    return 0;
}

int main() {
Sdisk disk1("disk1",256,128);
  filesys fsys("disk1",256,128);
  fsys.newfile("file1");
  fsys.newfile("file2");
  string bfile1;
  string bfile2;

  for (int i=1; i<=1024; i++)
     {
       bfile1+="1";
     }

  vector<string> blocks =fsys.block(bfile1,128); 

  int blocknumber=0;

  for (int i=0; i< blocks.size(); i++)
     {
       blocknumber=fsys.addblock("file1",blocks[i]);
     }

  fsys.delblock("file1",fsys.getfirstblock("file1"));

  for (int i=1; i<=2048; i++)
     {
       bfile2+="2";
     }

  blocks= fsys.block(bfile2,128); 

  for (int i=0; i< blocks.size(); i++)
     {
       blocknumber=fsys.addblock("file2",blocks[i]);
     }

  fsys.delblock("file2",blocknumber);
}
