#include "transform.h"
#include "componentmanager.h"
#include "threads.h"
#include <cmath>

namespace ql {
	transform::transform()
		: _position({{1, 0, 0, 0}}), _rotation({{1, 0, 0, 0}}),
		  _scale({{1, 1, 1, 1}}){};
	transform::transform(GameObject &parent, const void *data)
		: _position({{1, 0, 0, 0}}), _rotation({{1, 0, 0, 0}}),
		  _scale({{1, 1, 1, 1}}) {
		if (data) {
			_position = (*(transform *)data).position;
			_rotation = (*(transform *)data).rotation;
			_scale	  = (*(transform *)data).scale;
		}
	}

	transform::transform(C3D_FVec position, C3D_FQuat rotation, C3D_FVec scale)
		: _position(position), _rotation(rotation), _scale(scale){};

	transform &transform::operator=(transform &&other) {
		_position = other._position;
		_rotation = other._rotation;
		_scale	  = other._scale;
		_l		  = other._l;
		return *this;
	}

	void transform::translate(C3D_FVec offset) {
		LightLock_Guard l(_l);
		_position.x += offset.x;
		_position.y += offset.y;
		_position.z += offset.z;
	}

	void transform::rotate(C3D_FVec offset, bool local) {
		LightLock_Guard l(_l);
		_rotation = Quat_RotateZ(rotation, offset.z, local);
		_rotation = Quat_RotateX(rotation, offset.x, local);
		_rotation = Quat_RotateY(rotation, offset.y, local);
	}

	void transform::rotateX(float angle, bool local) {
		LightLock_Guard l(_l);
		_rotation = Quat_RotateX(rotation, angle, local);
	}
	void transform::rotateY(float angle, bool local) {
		LightLock_Guard l(_l);
		_rotation = Quat_RotateY(rotation, angle, local);
	}
	void transform::rotateZ(float angle, bool local) {
		LightLock_Guard l(_l);
		_rotation = Quat_RotateZ(rotation, angle, local);
	}

	void transform::setRotation(C3D_FVec attitude, bool local) {
		LightLock_Guard l(_l);
		_rotation =
			Quat_FromPitchYawRoll(attitude.x, attitude.y, attitude.z, local);
	}

	void transform::rotate(C3D_FVec offset, C3D_FVec origin) {
		LightLock_Guard l(_l);
		C3D_FVec posoffset = {{position.x - origin.x, position.y - origin.y,
							   position.z - origin.z}};
		C3D_FQuat rot =
			Quat_FromPitchYawRoll(offset.x, offset.y, offset.z, false);
		_position = Quat_CrossFVec3(rot, posoffset);
		_position.x += origin.x;
		_position.y += origin.y;
		_position.z += origin.z;
		_rotation = Quat_Multiply(rot, rotation);
	}

	C3D_FQuat transform::slerp(C3D_FQuat qa, C3D_FQuat qb, double t) {
		// quaternion to return
		C3D_FQuat qm;

		// Calculate angle between them.
		float cosHalfTheta =
			qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;

		// if qa=qb or qa=-qb then theta = 0 and we can return qa
		if (abs(cosHalfTheta) >= 1.0) {
			qm.w = qa.w;
			qm.x = qa.x;
			qm.y = qa.y;
			qm.z = qa.z;
			return qm;
		}
		// Calculate temporary values.
		float halfTheta	   = acosf(cosHalfTheta);
		float sinHalfTheta = sqrtf(1.0 - cosHalfTheta * cosHalfTheta);
		// if theta = 180 degrees then result is not fully defined
		// we could rotate around any axis normal to qa or qb
		if (fabs(sinHalfTheta) < 0.001) { // fabs is floating point absolute
			qm.w = (qa.w * 0.5 + qb.w * 0.5);
			qm.x = (qa.x * 0.5 + qb.x * 0.5);
			qm.y = (qa.y * 0.5 + qb.y * 0.5);
			qm.z = (qa.z * 0.5 + qb.z * 0.5);
			return qm;
		}
		float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
		float ratioB = sinf(t * halfTheta) / sinHalfTheta;

		// calculate Quaternion.
		qm.w		 = (qa.w * ratioA + qb.w * ratioB);
		qm.x		 = (qa.x * ratioA + qb.x * ratioB);
		qm.y		 = (qa.y * ratioA + qb.y * ratioB);
		qm.z		 = (qa.z * ratioA + qb.z * ratioB);
		return qm;
	}

	C3D_FVec transform::eulerAngles() const {
		C3D_FVec angles;
		C3D_FQuat q		= rotation;

		// pitch (x-axis rotation)
		float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
		float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		angles.x		= atan2f(sinr_cosp, cosr_cosp);

		// yaw (y-axis rotation)
		float sinp		= std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
		float cosp		= std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
		angles.y		= 2 * atan2f(sinp, cosp) - M_PI / 2;

		// roll (z-axis rotation)
		float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
		float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		angles.z		= atan2f(siny_cosp, cosy_cosp);

		return angles;
	}

	transform::operator C3D_Mtx() const {
		C3D_Mtx m, rot;

		Mtx_FromQuat(&rot, rotation);
		Mtx_Identity(&m);

		Mtx_Translate(&m, position.x, position.y, position.z,
					  true);					  // translate
		Mtx_Multiply(&m, &rot, &m);				  // rotate
		Mtx_Scale(&m, scale.x, scale.y, scale.z); // scale

		return m;
	}

	transform::~transform() {}

	COMPONENT_REGISTER(transform)
} // namespace ql