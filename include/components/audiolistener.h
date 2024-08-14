#pragma once

class GameObject;
class AudioListener {
	friend class AudioManager;
	friend class AudioSource;
	GameObject *parent;
	bool active;

  public:
	AudioListener(GameObject &owner, const void *data);
};