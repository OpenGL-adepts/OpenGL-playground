#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "shape.hpp"


class Skybox
{
    public:
        GLfloat cameraAperture, cameraShutterSpeed, cameraISO;
        Texture texSkybox;

        Skybox();
        ~Skybox();
        void setSkyboxTexture(const char* texPath);
        void renderToShader(Shader& shaderSkybox, glm::mat4& projection, glm::mat4& view);
        void setExposure(GLfloat aperture, GLfloat shutterSpeed, GLfloat iso);
};


#endif
