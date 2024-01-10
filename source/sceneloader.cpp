#include "sceneloader.h"
#include "scene.h"
#include <memory>
#include <fstream>
#include <string>
#include <cctype>
#include "gameobject.h"
#include "componentmanager.h"
#include "console.h"
#include "base64.hpp"
#include "slmdlloader.h"
#include <type_traits>
#include <iostream>

// helper functions for the loader

// need to pass in scene for the object constructor
void parseChildren(std::unique_ptr<Scene>& s, GameObject& object, std::string& input);
void parseComponent(GameObject& object, std::string input);
void parseComponents(GameObject& object, std::string& input);
void parseScripts(GameObject& object, std::string& input);
GameObject *parseObject(std::unique_ptr<Scene>& s, std::string& input);
void printSceneTree(GameObject& root, int indentlevel = 0);
void parseSceneTree(std::unique_ptr<Scene>& s, std::string& text);

std::string readFile(std::string name)
{
    std::stringstream str;
    std::ifstream stream(("romfs:/scenes/" + name + ".scene"));
    if(stream.is_open())
    {
        while(stream.peek() != EOF)
        {
            str << (char) stream.get();
        }
        stream.close();
        return str.str();
    }
    return "";
}

std::unique_ptr<Scene> SceneLoader::load(std::string name) {
    std::ifstream scenefile;
    scenefile.open(("romfs:/scenes/" + name + ".scene"));

    if (!scenefile.is_open()) {
        printf("no scene file");
        svcSleepThread(5000000000);
        return NULL;
    }
    std::unique_ptr<Scene> out(new Scene(name));

    std::string text = readFile(name);

    Console::log("read file\n"); 
    scenefile.close();

    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char x) { return std::isspace(x); }), text.end()); // remove all whitespace from text
    
    parseSceneTree(out, text); // parse the whole object tree 
    // printSceneTree(*out->root);
    Console::success("finished reading scene file");
    Console::log("%u objects", out->root->children.size());

    Camera::mainTop->sceneRoot = out->root;
    
    out->awake();

	out->start();

    return out;
}

void printSceneTree(GameObject& root, int indentlevel) {
    Console::log("%.*s%s", indentlevel * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | ", root.name.c_str());
    for (GameObject* obj : root.children) {
        if (!obj) continue;
        printSceneTree(*obj, indentlevel + 1);
    }
}

void parseSceneTree(std::unique_ptr<Scene>& s, std::string& text) {
    s->root = parseObject(s, text); 
}

void parseChildren(std::unique_ptr<Scene>& s, GameObject& object, std::string& input) {
    unsigned int n = 0;
    while(input[0] != ']' && input.size() > 0) {
        GameObject* obj = parseObject(s, input);
        ++n;
        s->objects.push_back(obj);
        object.addChild(*s->objects.back());
    }
    input = input.substr(input.find(']') + 1);
    if (input[0] == ',') input = input.substr(1); 
}

GameObject *parseObject(std::unique_ptr<Scene>& s, std::string& input) { 
    GameObject* object = new GameObject(s->reg);
    object->name = input.substr(0, input.find('['));
    input = input.substr(input.find('[') + 1); // go to start of objects

    for (int i = 0; i < 3; i++) {
        std::string attr = input.substr(0, input.find('[')); // isolate the name of the first part
        input = input.substr(input.find('[') + 1); // go to start of that section
        
        if (attr == "components") parseComponents(*object, input);
        else if (attr == "scripts") parseScripts(*object, input);
        else if (attr == "children") parseChildren(s, *object, input);
        else break;
    }

    input = input.substr(input.find(']') + 1);
    if (input[0] == ',') input = input.substr(1);

   
    return object;
}

void parseScripts(GameObject& object, std::string& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value 
            ComponentManager::addScript(input.substr(0, input.find(',')).c_str(), object);
            input = input.substr(input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there 
            ComponentManager::addScript(input.substr(0, input.find(']')).c_str(), object);
            input = input.substr(input.find(']') + 1); // go to the end of the section
            break;
        }
    }
    input = input.substr(input.find(',') + 1); // go to end of section
}

void parseComponents(GameObject& object, std::string& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value
            parseComponent(object, input.substr(0, input.find(','))); 
            input = input.substr(input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there
            parseComponent(object, input.substr(0, input.find(']'))); 
            input = input.substr(input.find(']') + 1); // go to the end of the section
            break;
        }
    }
    
    input = input.substr(input.find(',') + 1);
}

void parseComponent(GameObject& object, std::string input) {
    std::string componentname = input.substr(0, input.find('{'));

    const void* bindata = NULL;
    std::string base64data;
    if (input.find('}') - input.find('{') - 1 > 0) {
        base64data = base64::from_base64(input.substr(input.find('{') + 1, input.find('}') - input.find('{') - 1));
        bindata = base64data.c_str(); // get pointer to contained data
    }

    if (componentname == "mesh") { // it needs to be handled differently since models are loaded differently
        std::string s((const char*)bindata); 
        mdlLoader::addModel("romfs:/" + s, object);
    } else {
        ComponentManager::addComponent(componentname.c_str(), object, bindata);
    }
}