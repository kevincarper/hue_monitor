#ifndef _LIGHTMONITOR_H
#define _LIGHTMONITOR_H
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "Light.h"

// custom exception for errors in LightMonitor constructor
class LightMonitorException : public std::runtime_error
{
    public:
        LightMonitorException(std::string msg) : std::runtime_error(msg) {};
};

class LightMonitor
{
    public:
        // constructor
        LightMonitor(std::string cfgFile="config.json");

        // public methods
        std::vector<std::string> getCurLights();
        Light lightFromId(std::string _id);
        void updateLight(std::string& _id, Light& newState);
        void displayLights();
        bool idInLights(std::string& _id);
        std::vector<std::string> getLightIds();
        void run();

    private:
        // private methods
        std::string httpGet(const std::string& _uri);
        Poco::JSON::Object::Ptr parseJson(const std::string& resp);
        std::string getApiBase() {return apiBase;};

        // member variables
        Poco::JSON::Parser jsonParser;
        std::map<std::string, Light> lights; 
        std::string apiBase;
        unsigned int sleepTime;
};
#endif
