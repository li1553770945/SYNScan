#include <iostream>
#include <string>
#include <vector>
using namespace std;


uint32_t StringToIp(string str)
{
    uint32_t result = 0;
    vector <string> splits = split_string(str,'.');
    
    for(int i=0;i<4;i++)
    {
        result += atoi(splits[i].data()) << ((3-i) * 8);
    }
    return result;
}

string IpToString(uint32_t ip)
{
    string result;
    uint32_t mask = (1<<8) - 1;
    result += to_string( (ip>>24));
    result+=".";
    result += to_string( (ip>>16) & mask);
    result+=".";
    result += to_string( (ip>>8) & mask);
    result+=".";
    result += to_string( (ip) & mask);
    return result;

}