#include "sceneloader.h"
#include <3ds.h>
#include "scene.h"
#include <memory>
#include <fstream>
#include <string>
#include <string_view>
#include <cctype>
#include "gameobject.h"
#include "componentmanager.h"
#include "console.h"
#include "base64.hpp"
#include "slmdlloader.h"
#include <type_traits>
#include <iostream>
#include "scenemanager.h"
#include "config.h"
#include "exceptions.h"
#include "sl_assert.h"
#include <stdlib.h>


#define SCENELOADER_THREAD_STACK_SZ (32 * 1024)    // 32kB stack for scene loader thread
#define THREAD_YIELD svcSleepThread(0)

// helper functions for the loader

template <class T> using unique_ptr_aligned = 
	std::unique_ptr<T, decltype(&free)>;
template<class T>
unique_ptr_aligned<T> aligned_uptr(size_t align, size_t size) {
	return unique_ptr_aligned<T>(
		static_cast<T*>(aligned_alloc(align, size)),
		&free
	);
}

auto readFile(const std::string& filename)
{
	std::ifstream in(("romfs:/scenes/" + filename + ".scene"), std::ios::in | std::ios::binary);
	Console::log(("romfs:/scenes/" + filename + ".scene").c_str());
	if (in)
	{ // only works with c++11 or higher, lower versions don't guarantee contiguous string data
		unsigned long size;
		in.seekg(0, std::ios::end);
		size = in.tellg();
		auto c = aligned_uptr<char>(0x1000, size);
		in.seekg(0, std::ios::beg);
		in.read(c.get(), size);
		in.close();
		char* pc = c.get(), *pd = pc;
		do { while (std::isspace(*pc)) pc++; } while ((*pd++ = *pc++)); // remove whitespace
		Console::log("loaded scene file, length %lu", size);
		return c;
	}
	auto r = aligned_uptr<char>(alignof(char), 0);
	r.reset(nullptr);
    return r;
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
	// installExceptionHandler(exceptionHandler2);

	if (!params) {
		Console::error("Params are null");
		return;
	}
	TickCounter sceneloadtimecounter;

	osTickCounterStart(&sceneloadtimecounter);

	sceneLoadThreadParams& p = *(sceneLoadThreadParams*)params; // reference to not make a copy
	
	LightEvent_Init(&p.event, RESET_ONESHOT);

	auto textstr{readFile(p.name)};
	ASSERT(textstr != nullptr, "Invalid scene file");
    std::string_view text{textstr.get()};

    SceneLoader::parseObjectAsync(p.s, text, text.size(), p.progress); // parse the whole object tree
    p.s->root = &p.s->objects.front();
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
    	std::make_unique<Scene>(name),
        name,
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
    threadCreate(sceneLoadThread, p, SCENELOADER_THREAD_STACK_SZ, priority, -1, true);

    return AsyncSceneLoadOperation{&p->progress, &p->isdone, &p->event};
}

bool SceneLoader::load(std::string name) {
	std::unique_ptr<Scene> out = std::make_unique<Scene>(name);

    auto textstr{readFile(name)};
    ASSERT(textstr != nullptr, "Invalid scene file");
    std::string_view text{textstr.get()};
    Console::success("read scene file");
    std::string_view t{text};
    
    // parse the whole object tree recursively
    parseObject(out, t);
    out->root = &out->objects.back();
    Console::success("finished reading scene file");

    SceneManager::setScene(out);
    return true;
}

void SceneLoader::printSceneTree(GameObject& root, int indentlevel) {
    Console::log("%.*s%s", indentlevel * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | ", root.name.c_str());
    for (GameObject* obj : root.children) {
        if (!obj) continue; // make sure children aren't empty references
        printSceneTree(*obj, indentlevel + 1);
    }
}

void SceneLoader::parseChildren(std::unique_ptr<Scene>& s, GameObject& object, std::string_view& input) {
    unsigned int n = 0;
    while(input[0] != ']' && input.size() > 0) {
        parseObject(s, input);
        ++n;
        object.addChild(s->objects.back());
    }
    input.remove_prefix(input.find(']') + 1);
    if (input[0] == ',') input.remove_prefix(1);
}

void SceneLoader::parseChildrenAsync(std::unique_ptr<Scene>& s, GameObject& object, std::string_view& input, unsigned int size, float& progress) {
    unsigned int n = 0;
    while(input[0] != ']' && input.size() > 0) {
        parseObjectAsync(s, input, size, progress);
        ++n;
        object.addChild(s->objects.back());
    }
    input.remove_prefix(input.find(']') + 1);
    if (input[0] == ',') input.remove_prefix(1);
}

void SceneLoader::parseObject(std::unique_ptr<Scene>& s, std::string_view& input) {
	s->objects.emplace_back(s->reg, std::string{input.substr(0, input.find('['))}, *s.get());
    GameObject& object = s->objects.back();
    
    input.remove_prefix(input.find('[') + 1); // go to start of objects

    for (int i = 0; i < 3; i++) {
        std::string attr {input.substr(0, input.find('['))}; // isolate the name of the first part
        input.remove_prefix(input.find('[') + 1); // go to start of that section

        if (attr == "components") parseComponents(s, object, input);
        else if (attr == "scripts") parseScripts(s, object, input);
        else if (attr == "children") parseChildren(s, object, input);
        else break;
    }

    input.remove_prefix(input.find(']') + 1);
    if (input[0] == ',') input.remove_prefix(1);
}

void SceneLoader::parseObjectAsync(std::unique_ptr<Scene>& s, std::string_view& input, unsigned int size, float& progress) {
    THREAD_YIELD; // yield thread so the loader doesn't hog the cpu
    s->objects.emplace_back(s->reg, std::string{input.substr(0, input.find('['))}, *s.get());
    GameObject& object = s->objects.back();
    input.remove_prefix(input.find('[') + 1); // go to start of objects
    for (int i = 0; i < 3; i++) {
        std::string attr {input.substr(0, input.find('['))}; // isolate the name of the first part
        input.remove_prefix(input.find('[') + 1); // go to start of that section

        // allows any order, as well as omission
        if (attr == "components") parseComponents(s, object, input);
        else if (attr == "scripts") parseScripts(s, object, input);
        else if (attr == "children") parseChildrenAsync(s, object, input, size, progress);
        else break;
    }

    input.remove_prefix(input.find(']') + 1);
    if (input[0] == ',') input.remove_prefix(1);

    progress = 1 - (((float)input.size()) / size);
    printf("\e[s\e[24;%uH%c\e[u", (int)size * 40, '-');
    // Console::log("Progress %f", progress);
}

void SceneLoader::parseScripts(std::unique_ptr<Scene>& s, GameObject& object, std::string_view& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value
            ComponentManager::addScript(std::string(input.substr(0, input.find(']'))).c_str(), object);
            input.remove_prefix(input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there
            ComponentManager::addScript(std::string(input.substr(0, input.find(']'))).c_str(), object);
            input.remove_prefix(input.find(']') + 1); // go to the end of the section
            break;
        }
    }
    input.remove_prefix(input.find(',') + 1); // go to end of section
}

void SceneLoader::parseComponents(std::unique_ptr<Scene>& s, GameObject& object, std::string_view& input) {
    while (input[0] != ']') { // ] means end of the section
        if (input.find(',') < input.find(']')) { // this means there is a , closer than a ] so there must be at least another value
            parseComponent(s, object, input.substr(0, input.find(',')));
            input.remove_prefix(input.find(',') + 1); // go to next one
        }
        else { // this means there is only one in there
            parseComponent(s, object, input.substr(0, input.find(']')));
            input.remove_prefix(input.find(']') + 1); // go to the end of the section
            break;
        }
    }

    input.remove_prefix(input.find(',') + 1);
}

void SceneLoader::parseComponent(std::unique_ptr<Scene>& s, GameObject& object, std::string_view input) {
    std::string componentname {input.substr(0, input.find('{'))};

    // deserialize base64 data
    const void* bindata = NULL;
    std::string base64data;
    if (input.find('}') - input.find('{') - 1 > 0) {
        base64data = base64::from_base64(input.substr(input.find('{') + 1, input.find('}') - input.find('{') - 1)) + '\0';
        bindata = base64data.c_str(); // get pointer to contained data
    }

    ComponentManager::addComponent(componentname.c_str(), object, bindata);
}