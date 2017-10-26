#include <stdexcept>

#include "Light.h"

#define BRI_MIN 1
#define BRI_MAX 254

using namespace std;

Light::Light(string _name, bool _on, string _id, int _bri) 
{
    name = _name;
    on = _on;
    id = _id;
    // Hue API docs say 0 and 255 get set to BRI_MIN and BRI_MAX, respectively
    if (_bri == 0)
        _bri = 1;
    else if (_bri == 255)
        _bri = 254;
    // throw exception if outside [BRI_MIN, BRI_MAX]
    if ((_bri < BRI_MIN) || (_bri > BRI_MAX))
        throw invalid_argument("bri outside allowable range");
    else
        brightness = mapBri(_bri);
}

int Light::mapBri(int bri)
{
    double bri_min = 1;
    double bri_max = 254;
    if (bri < bri_min)
        return 0;
    else if (bri > bri_max)
        return 100;
    else
        return int(100*(bri-bri_min)/(bri_max-bri_min));
}

Poco::JSON::Object Light::toJson()
{
    Poco::JSON::Object obj(true);
    obj.set("id", id);
    obj.set("name", name);
    obj.set("on", on);
    obj.set("brightness", brightness);

    return obj;
}

string Light::pretty()
{
    string ret;
    ostringstream os;

    Poco::JSON::Object obj(true);
    obj.set("id", id);
    obj.set("name", name);
    obj.set("on", on);
    obj.set("brightness", brightness);
    obj.stringify(os, 4);
    ret = os.str();

    return ret;
}

vector<Poco::JSON::Object::Ptr> Light::diff(Light& other)
{
    vector<Poco::JSON::Object::Ptr> ret;
    if (name != other.getName())
    {
        Poco::JSON::Object::Ptr tmp = new Poco::JSON::Object;
        tmp->set("id", id);
        tmp->set("name", other.getName());
        ret.push_back(tmp);
    }
    if (brightness != other.getBrightness())
    {
        Poco::JSON::Object::Ptr tmp = new Poco::JSON::Object;
        tmp->set("id", id);
        tmp->set("brightness", other.getBrightness());
        ret.push_back(tmp);
    }
    if (on != other.getOn())
    {
        Poco::JSON::Object::Ptr tmp = new Poco::JSON::Object;
        tmp->set("id", id);
        tmp->set("on", other.getOn());
        ret.push_back(tmp);
    }
    return ret;
}
