#ifndef _LIGHT_H
#define _LIGHT_H

#include <Poco/JSON/Object.h>

#include <vector>
#include <string>
#include <sstream>

class Light
{
    public:
        // constructors
        Light(){};
        Light(std::string _name, bool _on, std::string _id, int _bri);
        Light(const Light& copy) : 
            name(copy.name), 
            on(copy.on), 
            id(copy.id), 
            brightness(copy.brightness) {};

        // public methods
        std::string pretty();
        Poco::JSON::Object toJson();
        std::vector<Poco::JSON::Object::Ptr> diff(Light& other);

        // variable access
        std::string getName(){return name;};
        std::string getId(){return id;};
        bool getOn(){return on;};
        int getBrightness(){return brightness;};

    private:
        int mapBri(int bri);

        // member variables
        std::string name;
        bool on;
        std::string id;
        int brightness;
};
#endif
