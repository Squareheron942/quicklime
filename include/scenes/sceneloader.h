#pragma once

#include <3ds.h>
#include <memory>
#include <string>
#include <string_view>

namespace ql {
	struct AsyncSceneLoadOperation {
		const float *progress		= NULL; // read only reference
		const bool *isDone			= NULL; // read only reference
		LightEvent *activationEvent = NULL; // rw reference
	};

	class Scene;
	class GameObject;

	class SceneLoader {
		SceneLoader() = delete;
		friend void sceneLoadThread(void *params);
		static void parseChildren(std::unique_ptr<Scene> &s, GameObject &object,
								  std::string_view &input);
		static void parseChildrenAsync(std::unique_ptr<Scene> &s,
									   GameObject &object,
									   std::string_view &input,
									   unsigned int size, float &progress);
		static void parseComponent(std::unique_ptr<Scene> &s,
								   GameObject &object, std::string_view input);
		static void parseComponents(std::unique_ptr<Scene> &s,
									GameObject &object,
									std::string_view &input);
		static void parseScripts(std::unique_ptr<Scene> &s, GameObject &object,
								 std::string_view &input);
		static void parseObject(std::unique_ptr<Scene> &s,
								std::string_view &input);
		static void parseObjectAsync(std::unique_ptr<Scene> &s,
									 std::string_view &input, unsigned int size,
									 float &progress);
		static void printSceneTree(GameObject &root, int indentlevel = 0);

	  public:
		/**
		 * @brief Load scene from file
		 *
		 * @param file Path of .scene file to read
		 * @return std::unique_ptr<Scene> Pointer to the scene instance
		 */
		static bool load(std::string file);

		/**
		 * @brief Load scene from file. The returned unique_ptr must be kept
		 * alive until progress = 1.
		 *
		 * @param file Path of .scene file to read
		 * @return AsyncSceneLoadOperation Information about scene load
		 * progress. If scene not found, progress will be -1
		 */
		static AsyncSceneLoadOperation loadAsync(std::string file);
	};
} // namespace ql
