#pragma once

class GameObject;
class AudioListener {
	friend class AudioManager;
	friend class AudioSource;
	GameObject* parent;
	bool active;
};