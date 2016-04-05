#pragma once

#include "graphicsmath.h"

class Transform
{
public:
	Transform()
	{
		m_scale = Vector3f(1.0f);
		m_position = Vector3f();
		m_rotation = Vector3f();
	}

	void Scale(float x, float y, float z)
	{
		m_scale.x = x;
		m_scale.y = y;
		m_scale.z = z;
	}

	void Position(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
	}

	void Rotation(float x, float y, float z)
	{
		m_rotation.x = x;
		m_rotation.y = y;
		m_rotation.z = z;
	}

	const Matrix4f* GetWorldTransform()
	{
		Matrix4f ScaleTransform;
		Matrix4f RotationTransform;
		Matrix4f PositionTransform;

		ScaleTransform.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z);
		RotationTransform.InitRotateTransform(m_rotation.x, m_rotation.y, m_rotation.z);
		PositionTransform.InitTranslationTransform(m_position.x, m_position.y, m_position.z);

		m_worldTransformation = PositionTransform * RotationTransform * ScaleTransform;

		return &m_worldTransformation;
	}

	void SetPerspective(float FOV, float Width, float Height, float zNear, float zFar)
	{
		m_perspective.FOV = FOV;
		m_perspective.width = Width;
		m_perspective.height = Height;
		m_perspective.zNear = zNear;
		m_perspective.zFar = zFar;
	}

	Matrix4f* GetWorldPerspectiveTransformation()
	{
		Matrix4f PerspectiveTransform;
		PerspectiveTransform.InitPerspectiveProjection(m_perspective);

		GetWorldTransform();

		m_worldPerspectiveTransformation = PerspectiveTransform * m_worldTransformation;

		return &m_worldPerspectiveTransformation;
	}

private:
	Vector3f m_scale;
	Vector3f m_position;
	Vector3f m_rotation;

	Perspective m_perspective;

	Matrix4f m_worldTransformation;
	Matrix4f m_worldPerspectiveTransformation;
};