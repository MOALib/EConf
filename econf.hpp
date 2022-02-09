#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <exception>

#ifndef MXPSQL_ECONF_HPP
#define MXPSQL_ECONF_HPP

#ifdef _WIN32
#include <windows.h>
#endif

using std::istringstream;
        

namespace MXPSQL::ECONF {

    // Exception namespace
    namespace EXCEPTION{
        class ConfigException : public std::exception {
            public:
                ConfigException(const std::string& msg) : msg(msg) {}
                const char* what() const noexcept override { return msg.c_str(); }
            private:
                std::string msg;
        };

        class InvalidConfigSyntaxException : public ConfigException {
            public:
                InvalidConfigSyntaxException(const std::string& msg) : ConfigException(msg) {}
        };

        class ConfigEnvSetupException : public ConfigException {
            public:
                ConfigEnvSetupException(const std::string& msg) : ConfigException(msg) {}
        };
    }

    // Internal namespace
    namespace Internal{
        std::string getOS(){
            #ifdef _WIN32
                return "win";
            #elif __linux__
                return "linux";
            #elif __APPLE__
                return "apple";
            #else
                return "unknown";
            #endif
        }

        class LanguageParser{
            private:
            std::string line;
            std::istringstream* iss;

            std::vector<std::string> env_tokenizer(std::string line, std::string delimiter){
                std::vector<std::string> tokens;

                auto delimiterPos = line.find(delimiter);

                if (delimiterPos != std::string::npos) {
                    std::string key = line.substr(0, delimiterPos);
                    std::string value = line.substr(delimiterPos + 1);
                    tokens.push_back(key);
                    tokens.push_back(value);
                }
                else{
                    // throw EXCEPTION::InvalidConfigSyntaxException("Invalid syntax in config file");
                    // tokens = nullptr;

                    throw std::runtime_error("Key Error");
                }

                return tokens;
            }

            void commentRm(std::string& line, std::string comment){
                auto commentPos = line.find(comment);
                if (commentPos != std::string::npos) {
                    line = line.substr(0, commentPos);
                }
            }

            public:
            LanguageParser(std::string conf){
                iss = new std::istringstream(conf);
            }

            void parse(std::unordered_map<std::string, std::string>& data){
                while (getline(*iss, line)) {
                    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

                    // check if comment at beginning of line or empty line
                    if (line[0] == '#' || line[0] == ';' || line.empty()) {
                        continue;
                    } else if (line.find('#')) {
                        commentRm(line, "#");
                    }

                    std::istringstream iss2(line);
                    std::string strr;
                    while (getline(iss2, strr, ',')) {

                        std::vector<std::string> tokens;
                        
                        try{
                            tokens = env_tokenizer(strr, "=");

                            std::string key = tokens.at(0);
                            std::string value = tokens.at(1);

                            data[key] = value;
                        }
                        catch(std::runtime_error){
                            throw EXCEPTION::InvalidConfigSyntaxException("Invalid syntax in config file");
                        }



                        /* auto delimiterPos = strr.find("=");
                        auto name         = strr.substr(0, delimiterPos);
                        std::string value      = strr.substr(delimiterPos + 1);
                        // std::cout << name << " " << value << '\n';
                        data[name] = value; */
                    }
                }
            }

            void dispose(){
                delete iss;
            }
        };

        /* void parse(std::string& line, std::istringstream& iss, std::unordered_map<std::string, std::string>& data){

            while (getline(iss, line)) {
                line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

                // check if comment at beginning of line or empty line
                if (line[0] == '#' || line[0] == ';' || line.empty()) {
                    continue;
                } else if (line.find('#')) {
                    line = line.substr(0, line.find('#'));
                }

                std::istringstream iss2(line);
                std::string strr;
                while (getline(iss2, strr, ',')) {
                    auto delimiterPos = strr.find("=");

                    if (delimiterPos != std::string::npos) {
                        std::string key = strr.substr(0, delimiterPos);
                        std::string value = strr.substr(delimiterPos + 1);
                        data[key] = value;
                    }
                    else{
                        throw EXCEPTION::InvalidConfigSyntaxException("Invalid syntax in config file");
                    }

                    /* auto delimiterPos = strr.find("=");
                    auto name         = strr.substr(0, delimiterPos);
                    std::string value      = strr.substr(delimiterPos + 1);
                    // std::cout << name << " " << value << '\n';
                    data[name] = value;
                }

            }
        } */

        template<typename T, typename T2>
        std::map<T, T2> unordered_map2map(std::unordered_map<T, T2>& data){
            std::map<T, T2> map;
            for(auto& x : data){
                map[x.first] = x.second;
            }
            return map;
        }
    }

    // namespace aliasing internally
    namespace Internal = MXPSQL::ECONF::Internal;
    namespace Exception = MXPSQL::ECONF::EXCEPTION;

    // config and env functions
    void readconfig(std::string conf, std::unordered_map<std::string, std::string>& data) {
        std::string line;
        std::istringstream iss(conf);
        Internal::LanguageParser* lparser = new Internal::LanguageParser(conf);
        // Internal::parse(line, iss, data);

        lparser->parse(data);

        delete lparser;
    }

    void readconfig(std::string conf, std::map<std::string, std::string>& data){
        std::unordered_map<std::string, std::string> tmpmap;

        readconfig(conf, tmpmap);

        data = Internal::unordered_map2map(tmpmap);
    }

    void loadenv(std::string env){
        std::map<std::string, std::string> tmpmap;

        readconfig(env, tmpmap);

        for(auto& x : tmpmap){
            std::string name = x.first;
            std::string value = x.second;

            std::string env = std::string(std::string(name) + std::string("=") + std::string(value));

            int rc = 0;

            if(Internal::getOS() == "win"){
                rc = _putenv_s(name.c_str(), value.c_str());
            }
            else{
                rc = putenv(env.c_str());
            }

            if(rc != 0){
                throw Exception::ConfigEnvSetupException("Error setting environment variable");
            }

            if(rc != 0){
                rc = 0;
            }

            char* env_p = getenv(name.c_str());

            if(env_p == nullptr || env_p == NULL){
                rc = 1;
            }

            if(rc != 0){
                throw Exception::ConfigEnvSetupException("Error setting environment variable");
            }
        }
    }

}

#endif