#include <map>
#include <string>
#include <fstream>
#include <iostream>

#include "econf.hpp"

int main(int argc, char* argv[]){
    std::map<std::string, std::string> map;

    std::string config = "a=1\nb=2\nc=3\n#a\n;1\n";

    std::ifstream stream("config.txt");

    if(stream.good()){
        std::string line;
        config = "";
        while(std::getline(stream, line)){
            config += line + "\n";
        }
    }

    stream.close();

    MXPSQL::ECONF::readconfig(config, map);

    for(auto& x : map){
        std::string name = x.first;
        std::string value = x.second;
        std::cout << "name: " << name << " | value: " << value << '\n';
    }

    MXPSQL::ECONF::loadenv(config);
}