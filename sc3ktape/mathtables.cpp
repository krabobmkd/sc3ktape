#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;

int saveVector(string filename,const vector<uint8_t>&v)
{
    ofstream ofs(filename, ios::binary| ios::out);
    if(!ofs.good())
    {
        cout << "can't write file" << filename << endl;
        return 1;
    }
    ofs.write((const char*)v.data(),v.size());
    return 0;
}

int main(int argc, char **argv)
{
    // 2.2sec ->128 frames
    // div prj table for 16 pos unsigned on one dim:
    // 16x128 ->2048?
    // but can be recomputed with just the 16 value:
    // to have a 128c table as resource in file.

    // 16x128 in ram:
    // -> compute tables for 8:4:2:1 by shifts
    // -> compute 3 ,5,6,7 by succs adds.
    vector<uint8_t> divtable(128,0);
    for(size_t i=0;i<128;i++)
    {
        // max more zoomed should be 128 and even more 127 !
        const float thisXvalue = 16.0f;
        const float thisFOV = 64.0f;
        float x = (thisXvalue *7.99f*thisFOV) / ((float)i+64.0f);
         divtable[127-i] = (uint8_t)x;
        cout << "i: "<< i <<  "  v: " << (int) divtable[127-i] << endl;

    }
    int res = saveVector("projection128.cbin",divtable);
    if(!res) return res;

    return 0;
}
