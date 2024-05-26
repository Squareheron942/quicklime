#include "sceneloader.h"
#include <3ds.h>
#include "3ds/os.h"
#include "3ds/services/cfgu.h"
#include "3ds/svc.h"
#include "3ds/synchronization.h"
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
#include "lights.h"
#include <filesystem>
#include <3ds.h>
#include "controls.h"
#include "scenemanager.h"
#include "debug.h"
#include "config.h"

// 
// erase(): 36767ms o3ds 51231ms n3ds ???????
// c str:   ms
// 
// 
// 
// 
// 
// 
// 


#define SCENELOADER_THREAD_STACK_SZ (32 * 1024)    // 32kB stack for audio thread

// helper functions for the loader

// need to pass in scene for the object constructor
void parseChildren(std::unique_ptr<Scene>& s, GameObject& object, std::string& input);
void parseChildrenAsync(std::unique_ptr<Scene>& s, GameObject& object, std::string& input);
void parseComponent(std::unique_ptr<Scene>& s, GameObject& object, std::string input);
void parseComponents(std::unique_ptr<Scene>& s, GameObject& object, std::string& input);
void parseScripts(std::unique_ptr<Scene>& s, GameObject& object, std::string& input);
GameObject *parseObject(std::unique_ptr<Scene>& s, std::string& input);
GameObject *parseObjectAsync(std::unique_ptr<Scene>& s, std::string& input, unsigned int size, float& progress);
void printSceneTree(GameObject& root, int indentlevel = 0);
void parseSceneTree(std::unique_ptr<Scene>& s, std::string& text);
void parseSceneTreeAsync(std::unique_ptr<Scene>& s, std::string& text, float& progress);

std::string readFile(std::ifstream& stream)
{
    std::stringstream str;
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

struct sceneLoadThreadParams {
	std::unique_ptr<Scene> s; // scene pointer we are writing to
	std::string name; // scene file name
	float progress; // progress value callback
	bool isdone;
	bool activate;
	LightEvent event;
};

void exceptionHandler2(void) {
	// uninstall handler
	uninstallExceptionHandler();

	register unsigned int lr asm("lr"); // might work? idk
	unsigned int lrval = lr;
	Console::error("Scene Load Thread Crashed.");
	Console::error("lr: %p", lrval);
	threadExit(0);
}

void sceneLoadThread(void* params) {
	// setup exception handler
	installExceptionHandler(exceptionHandler2);
	
	if (!params) { // don't need to delete p since it's nonexistent
		Console::error("Params are null");
		return;
	}
	TickCounter sceneloadtimecounter;
	
	osTickCounterStart(&sceneloadtimecounter);
	
	sceneLoadThreadParams& p = *(sceneLoadThreadParams*)params; // reference to not make a copy

	LightEvent_Init(&p.event, RESET_ONESHOT);
	
	std::ifstream scenefile;
	scenefile.open(p.name);
	Console::log("scene file is open: %s", scenefile.is_open() ? "true" : "false");

	std::string text = readFile(scenefile);

    Console::log("read file, length %lu", text.size());
    scenefile.close();

    // remove whitespace
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char x) { return std::isspace(x); }), text.end()); // remove all whitespace from text
    parseSceneTreeAsync(p.s, text, p.progress); // parse the whole object tree
    Console::log("%u objects", p.s->root->children.size());

    p.progress = 1;
    p.isdone = true;
    
    osTickCounterUpdate(&sceneloadtimecounter);

    Console::success("finished loading scene file");
    Console::success("in %lfms", osTickCounterRead(&sceneloadtimecounter));
    
    if (!p.activate) LightEvent_Wait(&p.event); // if the user doesn't want it activated immediately, wait for their signal
    
    SceneManager::setScene(p.s);
    
    delete (sceneLoadThreadParams*)params; // clean up the pointer
}

AsyncSceneLoadOperation SceneLoader::loadAsync(std::string name) {
	// check if scene file exists
    std::ifstream scenefile;
    scenefile.open(("romfs:/scenes/" + name + ".scene"));
    assert(scenefile.is_open());
    scenefile.close();

    // start of scene load async function

    // needs to be a raw pointer otherwise I would make it a smart pointer
    sceneLoadThreadParams* p = new sceneLoadThreadParams {
    	std::unique_ptr<Scene>(new Scene(name)),
     	("romfs:/scenes/" + name + ".scene"),
      	0.f,
       	false,
        true,
        LightEvent()
    };

    // Set the thread priority to the main thread's priority ...
    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then add 1, as higher number => lower actual priority ...
    priority += 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    // Start the thread
    threadCreate(sceneLoadThread, p, SCENELOADER_THREAD_STACK_SZ, priority, config::newmodel ? 2 : -1, true);

    return AsyncSceneLoadOperation{p->progress, p->isdone, p->event};
}

bool SceneLoader::load(std::string name) {
    std::ifstream scenefile;
    scenefile.open(("romfs:/scenes/" + name + ".scene"));

    if (!scenefile.is_open()) {
        Console::error("No scene file");
        return false;
    }

    std::unique_ptr<Scene> out(new Scene(name));

    std::string text = readFile(scenefile);

    Console::log("read file");
    scenefile.close();

    // remove whitespace
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char x) { return std::isspace(x); }), text.end()); // remove all whitespace from text

    // parse the whole object tree recursively
    parseSceneTree(out, text);

    Console::success("finished reading scene file");
    Console::log("%u objects", out->root->children.size());

    SceneManager::setScene(out);
    return true;
}

void printSceneTree(GameObject& root, int indentlevel) {
    Console::log("%.*s%s", indentlevel * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | ", root.name.c_str());
    for (GameObject* obj : root.children) {
        if (!obj) continue; // make sure children aren't empty references
        printSceneTree(*obj, indentlevel + 1);
    }
}

void parseSceneTree(std::unique_ptr<Scene>& s, std::string& text) {
    s->root = parseObject(s, text);
}

void parseSceneTreeAsync(std::unique_ptr<Scene>& s, std::string& text, float& progress) {
	unsigned int size = text.size();
    s->root = parseObjectAsync(s, text, size, progress);
}

void parseChildren(std::unique_ptr<Scene>& s, GameObject& object, std::string& input) {
    unsigned int n = 0;
    while(input[0] != ']' && input.size() > 0) {
        GameObject* obj = parseObject(s, input);
        ++n;
        s->objects.push_back(obj);
        object.addChild(*s->objects.back());
    }
    input = input.erase(0, input.find(']') + 1);
    if (input[0] == ',') input = input.erase(0, 1);
}

void parseChildrenAsync(std::unique_ptr<Scene>& s, GameObject& object, std::string& input, unsigned int size, float& progress) {
    unsigned int n = 0;
    while(input[0] != ']' && input.size() > 0) {
        GameObject* obj = parseObjectAsync(s, input, size, progress);
        ++n;
        s->objects.push_back(obj);
        object.addChild(*s->objects.back());
    }
    input = input.erase(0, input.find(']') + 1);
    if (input[0] == ',') input = input.erase(0, 1);
}

GameObject *parseObject(std::unique_ptr<Scene>& s, std::string& input) {
    GameObject* object = new GameObject(s->reg);
    object->name = input.substr(0, input.find('['));
    input = input.erase(0, input.find('[') + 1); // go to start of objects

    for (int i = 0; i < 3; i++) {
        std::string attr = input.substr(0, input.find('[')); // isolate the name of the first part
        input = input.erase(0, input.find('[') + 1); // go to start of that section

        if (attr == "components") parseComponents(s, *object, input);
        else if (attr == "scripts") parseScripts(s, *object, input);
        else if (attr == "children") parseChildren(s, *object, input);
        else break;
    }

    input = input.erase(0, input.find(']') + 1);
    if (input[0] == ',') input = input.erase(0, 1);


    return object;
}

GameObject *parseObjectAsync(std::unique_ptr<Scene>& s, std::string& input, unsigned int size, float& progress) {
    GameObject* object = new GameObject(s->reg);
    object->name = input.substr(0, input.find('['));
    input = input.erase(0, input.find('[') + 1); // go to start of objects
    for (int i = 0; i < 3; i++) {
        std::string attr = input.substr(0, input.find('[')); // isolate the name of the first part
        input = input.erase(0, input.find('[') + 1); // go to start of that section

        // allows any order, as well as omission
        if (attr == "components") parseComponents(s, *object, input);
        else if (attr == "scripts") parseScripts(s, *object, input);
        else if (attr == "children") parseChildrenAsync(s, *object, input, size, progress);
        else break;
    }

    input = input.erase(0, input.find(']') + 1);
    if (input[0] == ',') input = input.erase(0, 1);

    progress = 1 - (((float)input.size()) / size);

    Console::log("Progress %f", progress);
    svcSleepThread(0); // yield thread so others can continue
    return object;
}

void parseScripts(std::unique_ptr<Scene>& s, GameObject& object, std::string& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value
            ComponentManager::addScript(input.substr(0, input.find(',')).c_str(), object);
            input = input.erase(0, input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there
            ComponentManager::addScript(input.substr(0, input.find(']')).c_str(), object);
            input = input.erase(0, input.find(']') + 1); // go to the end of the section
            break;
        }
    }
    input = input.erase(0, input.find(',') + 1); // go to end of section
}

void parseComponents(std::unique_ptr<Scene>& s, GameObject& object, std::string& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value
            parseComponent(s, object, input.substr(0, input.find(',')));
            input = input.erase(0, input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there
            parseComponent(s, object, input.substr(0, input.find(']')));
            input = input.erase(0, input.find(']') + 1); // go to the end of the section
            break;
        }
    }

    input = input.erase(0, input.find(',') + 1);
}

void parseComponent(std::unique_ptr<Scene>& s, GameObject& object, std::string input) {
    std::string componentname = input.substr(0, input.find('{'));

    const void* bindata = NULL;
    std::string base64data;
    if (input.find('}') - input.find('{') - 1 > 0) {
        base64data = base64::from_base64(input.substr(input.find('{') + 1, input.find('}') - input.find('{') - 1)) + '\0';
        bindata = base64data.c_str(); // get pointer to contained data
    }

    if (componentname == "mesh") { // it needs to be handled differently since models are loaded differently
        std::string name_s((const char*)bindata);
        mdlLoader::addModel("romfs:/" + name_s, object);
    } else {
        ComponentManager::addComponent(componentname.c_str(), object, bindata);
        if (componentname == "Camera") s->cameras.emplace_back(object.getComponent<Camera>()); //TODO allow components to access scene they are attached to
    }
}