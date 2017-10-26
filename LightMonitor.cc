#include <stdexcept>

#include "LightMonitor.h"


using namespace Poco::Net;
using namespace Poco;
using namespace std;

LightMonitor::LightMonitor(string cfgFile)
{
    string ipaddr;
    int port;
    string user;

    jsonParser.reset();
    // read config file
    ifstream ifs(cfgFile.c_str());
    if (!ifs.good())
    {
        throw LightMonitorException("could not read config file"); 
    }

    // parse config file
    try
    {
        Poco::JSON::Object::Ptr cfg = jsonParser.parse(ifs).extract<Poco::JSON::Object::Ptr>();
        jsonParser.reset();
    
        ipaddr = cfg->getValue<string>("ipaddr");
        port = cfg->getValue<int>("port");
        user = cfg->getValue<string>("user");
        if (cfg->has("sleeptime"))
            sleepTime = cfg->getValue<unsigned int>("sleeptime");
        else
            sleepTime = 10;
    }
    catch (Poco::JSON::JSONException& e)
    {
        throw LightMonitorException("failed to parse config file. need ipaddr, port, and user");
    }

    // api resources built from this address
    stringstream ss;
    ss << "http://" << ipaddr << ":" << port << "/api/" << user;
    apiBase = ss.str();
}

string LightMonitor::httpGet(const string& _uri)
{
    try
    {
        // prepare session
        URI uri(_uri);
        HTTPClientSession session(uri.getHost(), uri.getPort());

        // prepare path
        string path(uri.getPathAndQuery());
        if (path.empty()) 
        {
            path = "/";
        }

        // send request
        HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        session.sendRequest(req);

        // get response
        HTTPResponse res;
        //cout << res.getStatus() << " " << res.getReason() << endl;

        // convert response
        istream& is = session.receiveResponse(res);
        istreambuf_iterator<char> eos;
        string ret(istreambuf_iterator<char>(is), eos);

        return ret;
    }
    catch (Exception &e)
    {
        //cerr << e.displayText() << endl;
        return "";
    }
}

Poco::JSON::Object::Ptr LightMonitor::parseJson(const string& resp)
{
    Poco::JSON::Object::Ptr ret = jsonParser.parse(resp).extract<Poco::JSON::Object::Ptr>();
    jsonParser.reset();
    return ret;
}


vector<string> LightMonitor::getCurLights()
{
    // queries HUE bridge for available lights
    // throws Poco::JSON::JSONException if httpGet fails or malformed response
    string url;
    string resp;
    Poco::JSON::Object::Ptr lightsJson;

    url = apiBase + "/lights";

    resp = httpGet(url);
    //cout << resp << endl;
    lightsJson = parseJson(resp);

    vector<string> ids;
    lightsJson->getNames(ids);

    return ids;
}

vector<string> LightMonitor::getLightIds()
{
    // gets vector of light ids from map of known lights
    vector<string> keys;
    map<string, Light>::iterator it;
    for (it = lights.begin(); it != lights.end(); it++)
    {
        keys.push_back(it->first);
    }

    return keys;
}

bool LightMonitor::idInLights(string& _id)
{
    // check if id is in map of known lights
    vector<string> ids = getLightIds();
    if (find(ids.begin(), ids.end(), _id) != ids.end())
        return true;
    else
        return false;
}

Light LightMonitor::lightFromId(string _id)
{
    // gets light resource and creates Light from state parameters
    // throws Poco::JSON::JSONException if httpGet fails or malformed response
    // throws invalid_argument exception if parsed parameters invalid
    string resp;
    vector<string> lightIds;
    string stateUrl;
    Poco::JSON::Object::Ptr lightState;
    Poco::JSON::Object::Ptr subState;

    stateUrl = apiBase + "/lights/" + _id;
    resp = httpGet(stateUrl);
    lightState = parseJson(resp);

    string name;
    bool on;
    int bri; 
    string id;
    
    name = lightState->getValue<string>("name");
    subState = lightState->getObject("state");
    on = subState->getValue<bool>("on");
    bri = subState->getValue<int>("bri");
    id = _id;

    Light ret = Light(name, on, id, bri);
    return ret;
}

void LightMonitor::displayLights()
{
    // print all lights in lights map to cout for debug
    map<string, Light>::iterator it;
    for (it = lights.begin(); it != lights.end(); it++)
    {
        cout << it->second.pretty() << endl;
    }
}

void LightMonitor::updateLight(string& _id, Light& newState)
{
    // print diffs between updated light and previous state, then update in lights map
    Light oldState = lights[_id];
    vector<Poco::JSON::Object::Ptr> diffs = oldState.diff(newState);
    vector<Poco::JSON::Object::Ptr>::iterator it;

    for (it = diffs.begin(); it != diffs.end(); it++)
    {
        // print diff to stdout
        (*it)->stringify(cout, 4);
        cout << endl;
    }
    lights[_id] = newState;
}

void LightMonitor::run()
{
    // main monitor loop
    vector<string> curLights;
    vector<string>::iterator it;
    while (true)
    {
        Poco::JSON::Array::Ptr newLights = new Poco::JSON::Array;
        // get lights available through API
        try{
            curLights = getCurLights();
        }
        catch (Poco::JSON::JSONException& e)
        {
            //cerr << e.displayText() << endl;
            cout << "HUE bridge down..." << endl;
            sleep(sleepTime);       
            continue;
        }
        // no current lights either means there is no lights or 
        // the hue bridge is down. either way, do nothing
        if (!curLights.empty())
        {
            for (it = curLights.begin(); it != curLights.end(); it++)
            {
                Light tmp;
                try{
                    tmp = Light(lightFromId(*it));
                }
                // malformed JSON request or http request failed
                catch (Poco::JSON::JSONException& e)
                {
                    //cerr << e.displayText() << endl;
                    sleep(sleepTime);       
                    continue;
                }
                // invalid value type in JSON response
                catch (Poco::SyntaxException& e)
                {
                    //cerr << e.displayText() << endl;
                    sleep(sleepTime);       
                    continue;
                }
                // failed to create Light from parsed JSON
                catch (const invalid_argument& e)
                {
                    //cerr << e.what() << endl;
                    sleep(sleepTime);       
                    continue;
                }

                if (!idInLights(*it))
                {
                    // add new light
                    lights[*it] = tmp;
                    newLights->add(tmp.toJson());
                }
                else
                {
                    // update existing light
                    updateLight(*it, tmp);
                }
            }
            if (newLights->size() != 0)
            {
                // print any new lights to stdout
                newLights->stringify(cout, 4);
                cout << endl;
            }
        }
        sleep(sleepTime);       
    }
}
