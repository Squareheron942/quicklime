#pragma once

#include <3ds.h>
#include <algorithm>
#include <citro3d.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace controls {
	// mapping for key name
	enum class key : unsigned char {
		KEY_A			 = 0,
		KEY_B			 = 1,
		KEY_X			 = 2,
		KEY_Y			 = 3,
		KEY_SELECT		 = 4,
		KEY_START		 = 5,
		KEY_DPAD_RIGHT	 = 6,
		KEY_DPAD_LEFT	 = 7,
		KEY_DPAD_UP		 = 8,
		KEY_DPAD_DOWN	 = 9,
		KEY_L			 = 10,
		KEY_R			 = 11,
		KEY_ZL			 = 12,
		KEY_ZR			 = 13,
		KEY_CSTICK_UP	 = 14,
		KEY_CSTICK_DOWN	 = 15,
		KEY_CSTICK_LEFT	 = 16,
		KEY_CSTICK_RIGHT = 17,
		KEY_CPAD_UP		 = 18,
		KEY_CPAD_DOWN	 = 19,
		KEY_CPAD_LEFT	 = 20,
		KEY_CPAD_RIGHT	 = 21,
		KEY_UP			 = 22,
		KEY_DOWN		 = 23,
		KEY_LEFT		 = 24,
		KEY_RIGHT		 = 25,
		KEY_TOUCH		 = 26
	};

	void update();
	void init();

	/**
	 * @brief Check if key was pressed this frame
	 *
	 * @param inputName Key ID to check
	 * @return true
	 * @return false
	 */
	bool getDown(key inputName);

	bool getRepeat(key inputName);

	/**
	 * @brief Check if key was held for at least 2 frames
	 *
	 * @param inputName Key ID to check
	 * @return true
	 * @return false
	 */
	bool getHeld(key inputName);

	/**
	 * @brief Check if key was released this frame
	 *
	 * @param inputName Key ID to check
	 * @return true
	 * @return false
	 */
	bool getUp(key inputName);

	/**
	 * @returns The internal gyro angular rate (in deg/s)
	 */
	const angularRate
	gyroRate(); // this way no one can modify the internal states
	/**
	 * @returns The internal gyro position (in radians)
	 */
	const C3D_FVec gyroPos();

	/**
	 * @brief Resets internal gyro state to given value
	 *
	 * @param[in] newPos Position to reset the gyro state to (in radians)
	 */
	void resetGyro(const C3D_FVec newPos);

	/**
	 * @brief Gets the position of the Circle Pad
	 *
	 * @returns const circlePosition The position of the Circle Pad
	 */
	const circlePosition circlePos();

	/**
	 * @brief
	 *
	 * @return const circlePosition The position of the C Stick
	 */
	const circlePosition cStickPos();

	float gyroDeadZone(), gyroSensitivity();
	void setGyroDeadZone(float min_radius),
		setGyroSensitivity(float multiplier);
}; // namespace controls