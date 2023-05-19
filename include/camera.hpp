#ifndef CAMERA_H
#define CAMERA_H

//#include <glad/glad.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <vector>
//
//// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
//enum Camera_Movement {
//    FORWARD,
//    BACKWARD,
//    LEFT,
//    RIGHT
//};
//
//// Default camera values
//const float YAW = -90.0f;
//const float PITCH = 0.0f;
//const float SPEED = 2.5f;
//const float SENSITIVITY = 0.1f;
//const float ZOOM = 45.0f;
//
//
//// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
//class Camera
//{
//public:
//    // camera Attributes
//    glm::vec3 Position;
//    glm::vec3 Front;
//    glm::vec3 Up;
//    glm::vec3 Right;
//    glm::vec3 WorldUp;
//    // euler Angles
//    float Yaw;
//    float Pitch;
//    // camera options
//    float MovementSpeed;
//    float MouseSensitivity;
//    float Zoom;
//
//    // constructor with vectors
//    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
//    {
//        Position = position;
//        WorldUp = up;
//        Yaw = yaw;
//        Pitch = pitch;
//        updateCameraVectors();
//    }
//    // constructor with scalar values
//    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
//    {
//        Position = glm::vec3(posX, posY, posZ);
//        WorldUp = glm::vec3(upX, upY, upZ);
//        Yaw = yaw;
//        Pitch = pitch;
//        updateCameraVectors();
//    }
//
//    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
//    glm::mat4 GetViewMatrix()
//    {
//        return glm::lookAt(Position, Position + Front, Up);
//    }
//
//    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
//    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
//    {
//        float velocity = MovementSpeed * deltaTime;
//        if (direction == FORWARD)
//            Position += Front * velocity;
//        if (direction == BACKWARD)
//            Position -= Front * velocity;
//        if (direction == LEFT)
//            Position -= Right * velocity;
//        if (direction == RIGHT)
//            Position += Right * velocity;
//    }
//
//    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
//    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
//    {
//        xoffset *= MouseSensitivity;
//        yoffset *= MouseSensitivity;
//
//        Yaw += xoffset;
//        Pitch += yoffset;
//
//        // make sure that when pitch is out of bounds, screen doesn't get flipped
//        if (constrainPitch)
//        {
//            if (Pitch > 89.0f)
//                Pitch = 89.0f;
//            if (Pitch < -89.0f)
//                Pitch = -89.0f;
//        }
//
//        // update Front, Right and Up Vectors using the updated Euler angles
//        updateCameraVectors();
//    }
//
//    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
//    void ProcessMouseScroll(float yoffset)
//    {
//        Zoom -= (float)yoffset;
//        if (Zoom < 1.0f)
//            Zoom = 1.0f;
//        if (Zoom > 45.0f)
//            Zoom = 45.0f;
//    }
//
//private:
//    // calculates the front vector from the Camera's (updated) Euler Angles
//    void updateCameraVectors()
//    {
//        // calculate the new Front vector
//        glm::vec3 front;
//        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//        front.y = sin(glm::radians(Pitch));
//        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//        Front = glm::normalize(front);
//        // also re-calculate the Right and Up vector
//        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//        Up = glm::normalize(glm::cross(Right, Front));
//    }
//};

/*
* Basic camera class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
	private:
		float fov;
		float znear, zfar;

		void updateViewMatrix() {
			glm::mat4 rotM = glm::mat4(1.0f);
			glm::mat4 transM;

			rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::vec3 translation = position;
			if (flipY) {
				translation.y *= -1.0f;
			}
			transM = glm::translate(glm::mat4(1.0f), translation);

			if (type == CameraType::firstperson)
			{
				matrices.view = rotM * transM;
			}
			else
			{
				matrices.view = transM * rotM;
			}

			viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

			updated = true;
		}

	public:
		enum CameraType { lookat, firstperson };
		CameraType type = CameraType::lookat;

		glm::vec3 rotation = glm::vec3();
		glm::vec3 position = glm::vec3();
		glm::vec3 viewPos = glm::vec4();

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		bool updated = false;
		bool flipY = false;

		struct {
			glm::mat4 perspective;
			glm::mat4 view;
		} matrices;

		struct {
			bool left = false;
			bool right = false;
			bool up = false;
			bool down = false;
		} keys;

		bool moving() {
			return keys.left || keys.right || keys.up || keys.down;
		}

		float getNearClip() {
			return znear;
		}

		float getFarClip() {
			return zfar;
		}

		void setPerspective(float fov, float aspect, float znear, float zfar) {
			this->fov = fov;
			this->znear = znear;
			this->zfar = zfar;
			matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
			if (flipY) {
				matrices.perspective[1][1] *= -1.0f;
			}
		};

		void updateAspectRatio(float aspect) {
			matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
			if (flipY) {
				matrices.perspective[1][1] *= -1.0f;
			}
		}

		void setPosition(glm::vec3 position) {
			this->position = position;
			updateViewMatrix();
		}

		void setRotation(glm::vec3 rotation) {
			this->rotation = rotation;
			updateViewMatrix();
		}

		void rotate(glm::vec3 delta) {
			this->rotation += delta;
			updateViewMatrix();
		}

		void setTranslation(glm::vec3 translation) {
			this->position = translation;
			updateViewMatrix();
		};

		void translate(glm::vec3 delta) {
			this->position += delta;
			updateViewMatrix();
		}

		void setRotationSpeed(float rotationSpeed) {
			this->rotationSpeed = rotationSpeed;
		}

		void setMovementSpeed(float movementSpeed) {
			this->movementSpeed = movementSpeed;
		}

		void update(float deltaTime) {
			updated = false;
			if (type == CameraType::firstperson) {
				if (moving()) {
					glm::vec3 camFront;
					camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
					camFront.y = sin(glm::radians(rotation.x));
					camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
					camFront = glm::normalize(camFront);

					float moveSpeed = deltaTime * movementSpeed;

					if (keys.up)
						position += camFront * moveSpeed;
					if (keys.down)
						position -= camFront * moveSpeed;
					if (keys.left)
						position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
					if (keys.right)
						position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				}
			}
			updateViewMatrix();
		};

		// Update camera passing separate axis data (gamepad)
		// Returns true if view or position has been changed
		bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime) {
			bool retVal = false;

			if (type == CameraType::firstperson) {
				// Use the common console thumbstick layout		
				// Left = view, right = move

				const float deadZone = 0.0015f;
				const float range = 1.0f - deadZone;

				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
				camFront.y = sin(glm::radians(rotation.x));
				camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * movementSpeed * 2.0f;
				float rotSpeed = deltaTime * rotationSpeed * 50.0f;

				// Move
				if (fabsf(axisLeft.y) > deadZone) {
					float pos = (fabsf(axisLeft.y) - deadZone) / range;
					position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}
				if (fabsf(axisLeft.x) > deadZone) {
					float pos = (fabsf(axisLeft.x) - deadZone) / range;
					position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}

				// Rotate
				if (fabsf(axisRight.x) > deadZone) {
					float pos = (fabsf(axisRight.x) - deadZone) / range;
					rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
				if (fabsf(axisRight.y) > deadZone) {
					float pos = (fabsf(axisRight.y) - deadZone) / range;
					rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
			} else {
				// todo: move code from example base class for look-at
			}

			if (retVal) {
				updateViewMatrix();
			}

			return retVal;
		}
};

#endif