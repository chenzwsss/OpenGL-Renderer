#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class RenderCamera {
    private:
        float fov;
        float z_near, z_far;

        void updateViewMatrix() {
            glm::mat4 rot_m = glm::mat4(1.0f);
            glm::mat4 trans_m;

            rot_m = glm::rotate(rot_m, glm::radians(rotation.x * (flip_y ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
            rot_m = glm::rotate(rot_m, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rot_m = glm::rotate(rot_m, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            glm::vec3 translation = position;
            if (flip_y) {
                translation.y *= -1.0f;
            }
            trans_m = glm::translate(glm::mat4(1.0f), translation);

            if (type == camera_type::firstperson)
            {
                matrices.view = rot_m * trans_m;
            }
            else
            {
                matrices.view = trans_m * rot_m;
            }

            view_pos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

            updated = true;
        }

    public:
        enum camera_type { lookat, firstperson };
        camera_type type = camera_type::lookat;

        glm::vec3 rotation = glm::vec3();
        glm::vec3 position = glm::vec3();
        glm::vec3 view_pos = glm::vec4();

        float rotation_speed = 1.0f;
        float movement_speed = 1.0f;

        bool updated = false;
        bool flip_y = false;

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
            return z_near;
        }

        float getFarClip() {
            return z_far;
        }

        void setPerspective(float fov, float aspect, float z_near, float z_far) {
            this->fov = fov;
            this->z_near = z_near;
            this->z_far = z_far;
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
            if (flip_y) {
                matrices.perspective[1][1] *= -1.0f;
            }
        };

        void updateAspectRatio(float aspect) {
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
            if (flip_y) {
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

        void setRotationSpeed(float rotation_speed) {
            this->rotation_speed = rotation_speed;
        }

        void setMovementSpeed(float movement_speed) {
            this->movement_speed = movement_speed;
        }

        void update(float delta_time) {
            updated = false;
            if (type == camera_type::firstperson) {
                if (moving()) {
                    glm::vec3 cam_front;
                    cam_front.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                    cam_front.y = sin(glm::radians(rotation.x));
                    cam_front.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                    cam_front = glm::normalize(cam_front);

                    float move_speed = delta_time * movement_speed;

                    if (keys.up)
                        position += cam_front * move_speed;
                    if (keys.down)
                        position -= cam_front * move_speed;
                    if (keys.left)
                        position -= glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
                    if (keys.right)
                        position += glm::normalize(glm::cross(cam_front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
                }
            }
            updateViewMatrix();
        };
};

#endif