#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class render_camera {
    private:
        float fov;
        float znear, zfar;

        void update_view_matrix() {
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

        float get_near_clip() {
            return znear;
        }

        float get_far_clip() {
            return zfar;
        }

        void set_perspective(float fov, float aspect, float znear, float zfar) {
            this->fov = fov;
            this->znear = znear;
            this->zfar = zfar;
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
            if (flipY) {
                matrices.perspective[1][1] *= -1.0f;
            }
        };

        void update_aspect_ratio(float aspect) {
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
            if (flipY) {
                matrices.perspective[1][1] *= -1.0f;
            }
        }

        void set_position(glm::vec3 position) {
            this->position = position;
            update_view_matrix();
        }

        void set_rotation(glm::vec3 rotation) {
            this->rotation = rotation;
            update_view_matrix();
        }

        void rotate(glm::vec3 delta) {
            this->rotation += delta;
            update_view_matrix();
        }

        void set_translation(glm::vec3 translation) {
            this->position = translation;
            update_view_matrix();
        };

        void translate(glm::vec3 delta) {
            this->position += delta;
            update_view_matrix();
        }

        void set_rotation_speed(float rotationSpeed) {
            this->rotationSpeed = rotationSpeed;
        }

        void set_movement_speed(float movementSpeed) {
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
            update_view_matrix();
        };
};

#endif