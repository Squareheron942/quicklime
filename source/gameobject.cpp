#include "gameobject.h"
#include "3ds/synchronization.h"
#include "console.h"
#include "scene.h"
#include "script.h"
#include "threads.h"
#include "camera.h"

ql::GameObject::GameObject(std::string name, Scene &s)
	: s(s), reg(s.reg), id(s.reg.create()), name(name) {
	LightLock_Init(&_scriptL);
	Camera::cameraObjectListDirty = true;
}

ql::GameObject::GameObject(GameObject &&other)
	: _scriptL(other._scriptL), s(other.s), children(other.children),
	  scripts(std::move(other.scripts)), parent(other.parent), reg(other.reg),
	  id(other.id), name(other.name) {
	other.parent = nullptr;
	other.id	 = entt::entity{entt::null};
	other.scripts.clear();
	other.children.clear();
}

void ql::GameObject::Awake() {
	LightLock_Guard l(_scriptL);
	for (auto &s : scripts) {
		s->Awake();
		s->SetEnabled(true);
	}
}

void ql::GameObject::Start() {
	LightLock_Guard l(_scriptL);
	for (auto &s : scripts)
		if (s->enabled)
			s->Start();
}

void ql::GameObject::Update() {
	LightLock_Guard l(_scriptL);
	for (auto &s : scripts)
		if (s->enabled)
			s->Update();
}

void ql::GameObject::LateUpdate() {
	LightLock_Guard l(_scriptL);
	for (auto &s : scripts)
		if (s->enabled)
			s->LateUpdate();
}

void ql::GameObject::FixedUpdate() {
	LightLock_Guard l(_scriptL);
	for (auto &s : scripts)
		if (s->enabled)
			s->FixedUpdate();
}

ql::GameObject *ql::GameObject::r_search(std::string name) {
	GameObject *out = NULL;
	for (GameObject *child : children) {
		if (child->name == name)
			return child;
		out = child->r_search(name);
		if (out)
			return out;
	}
	return out;
}

ql::GameObject *ql::GameObject::find(std::string name) {
	/**
	 *     in front of name will search top down
	 * /   in front of name will only search root (then find children based on
	 * '/' "subdirectories")
	 * ./  in front of name will only search children
	 * ../ in front of name will only search children of parent (do ../../ to
	 * get level above etc)
	 */
	if (name[0] == '.') {
		if (name[1] == '.') {
			if (name[2] == '/') { // find in parent's children
				if (parent) {
					/**
					 * Go back one level by telling parent to search its
					 * children change beginning to './' and tell parent to find
					 * it
					 */
					return parent->find(name.substr(1));
				} else
					Console::warn("Object %s not found");
				return nullptr;
			}
		} else if (name[1] == '/') // find in own children
		{
			std::stringstream ss;
			ss << name.substr(2); // remove the './'
			std::string subname;
			GameObject *out = this; // start at current spot

			while (std::getline(ss, subname, '/') && out) { // for each level
				GameObject *temp = out;
				for (GameObject *obj : out->children) {
					if (obj->name == subname) {
						out = obj;
						break;
					}
				}
				if (temp == out)
					return nullptr; // if a child was not found in one level, it
									// was not found so return nothing
			}

			return out;
		}
	} else if (name[0] == '/') // find at root
	{
		GameObject *root = this;
		while (root->parent)
			root = root->parent;

		return root->find(name.substr(1));
	} else // find anywhere
	{
		GameObject *root = this;
		while (root->parent)
			root = root->parent; // get root of all objects (that way we don't
								 // need to store this at all times)

		int i		  = 0;
		bool hasChild = false;

		while (name[i] != 0) {
			if (name[i] == '/') {
				hasChild = true;
				break;
			}
			i++;
		}

		root = root->r_search(
			name.substr(0, i)); // recursively search for the first item

		if (root && hasChild)
			root = root->find("./" + name.substr(i));

		return root;
	}
	return nullptr;
};

ql::GameObject::~GameObject() {
	if (parent)
		parent->removeChild(this);
	if (id != entt::null)
		reg.destroy(id);
	Camera::cameraObjectListDirty = true;
}