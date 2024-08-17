#pragma once

#include "gameobject.h"
#include <entt/entt.hpp>
#include <string>

class Script {

  protected:
	GameObject *owner;
	template <class T> inline T *GetComponent() {
		return owner->reg.try_get<T>(owner->id);
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