#pragma once

#include <entt/entt.hpp>
#include <string>

class GameObject;

class Script {
	entt::registry &parentComponents;
	entt::entity parentID;

  protected:
	GameObject *owner;
	template <class T> inline T *GetComponent() {
		return parentComponents.try_get<T>(parentID);
	}

	GameObject *find(std::string object);

  public:
	bool enabled = true;
	Script(GameObject &owner);
	void SetEnabled(bool enabled);
	virtual ~Script(){};
	virtual void Awake(void){};
	virtual void Start(void){};
	virtual void FixedUpdate(void){};
	virtual void Update(void){};
	virtual void LateUpdate(void){};
	virtual void OnDisable(void){};
	virtual void OnEnable(void){};
};