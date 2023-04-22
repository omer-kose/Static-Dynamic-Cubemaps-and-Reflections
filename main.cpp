//OpenGL Core Libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//My headers
#include "Utilities.h"
#include "Shader.h"
#include "Camera.h"


//Utility Headers
#include <iostream>
#include <vector>
#include <memory>


//HW2 Specific Header
#include "Mesh.h"



//Camera Properties
glm::vec3 cameraPosition; //Always fixed to car
glm::mat4 view;
Camera camera(glm::vec3(0.0f, 0.0f, 2.0f));


//Light Properties (For now, we only have directional light)
glm::vec3 lightDir = glm::vec3(0.0, -100.0, 0.0);
glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);

//Time parameters
double deltaTime = 0.0;
double lastFrame = 0.0;

//Window
GLFWwindow* window;



//Ground Mesh Properties
Mesh groundMesh;
Shader groundShader;
float groundScale = 500.0f;
glm::vec3 groundOffset = glm::vec3(0.0f, -1.08f, 0.0f);
GLuint groundTextureID;


//Skybox Properties
Mesh skyboxMesh;
Shader skyboxShader;
GLuint skyboxTextureID;


//Car Properties
Mesh carBodyMesh;
Mesh carTiresMesh;
Mesh carWindowsMesh;
Shader carBodyShader;
Shader carTiresShader;
Shader carWindowsShader;
glm::vec3 carPosition = glm::vec3(0.0f);
glm::vec3 carFront = glm::vec3(0.0f, 0.0f, -1.0f);
float carSpeed = 0.0f;
float gasAcceleration = 5.0f;
float drag = 3.0f;
float carRotationOffset = 90.0f; //The loaded model has a rotation offset
float carRotationAngle = 90.0f;
float carRotationFactor = 15.0f;

//Texture Loaders
GLuint textureFromFile(const char* filePath, bool verticalFlip)
{
    //Generate the texture
    GLuint textureId;
    glGenTextures(1, &textureId);

    //Load the data from the file
    stbi_set_flip_vertically_on_load(verticalFlip);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filePath, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        //Bind and send the data
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //Configure params
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

    }
    else
    {
        std::cout << "Texture failed to load on path: " << filePath << std::endl;
        stbi_image_free(data);
    }

    return textureId;
}


GLuint loadCubemap(const std::vector<std::string>& faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (int i = 0; i < faces.size(); ++i)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void updateDeltaTime()
{
	double currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

//Callback function in case of resizing the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//Function that will process the inputs, such as keyboard inputs
//In this program this function handles the car movement.
void processCarMovement(GLFWwindow* window)
{
	//If pressed glfwGetKey return GLFW_PRESS, if not it returns GLFW_RELEASE
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

    
    float acceleration = 0.0f;
    float nitro; //YEAH
	//If shift is pressed move the camera faster
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        nitro = 2.0f;
    }
    else
    {
        nitro = 1.0f;
    }
        
	//Car movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        acceleration += (float)deltaTime * nitro * gasAcceleration;
    }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        acceleration -= (float)deltaTime * nitro * gasAcceleration;
    }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        carRotationAngle += (float)deltaTime * carRotationFactor;
    }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        carRotationAngle -= (float)deltaTime * carRotationFactor;
    }

    //Update car vectors
    float carRotationRadian = glm::radians(carRotationAngle);
    carFront = glm::vec3((float)cos(carRotationRadian), 0.0f, (float)(-sin(carRotationRadian)));
    //Apply Drag if car is moving
    if(fabs(carSpeed) > deltaTime)
    {
        if(carSpeed > 0)
        {
            acceleration -= (float)deltaTime * drag;
        }
        else
        {
            acceleration += (float)deltaTime * drag;
        }
    }
    carSpeed += acceleration;
    carPosition += (float)deltaTime * (carSpeed * carFront);
}


int setupDependencies()
{
	glfwInit();
	//Specify the version and the OpenGL profile. We are using version 3.3
	//Note that these functions set features for the next call of glfwCreateWindow
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

	//Create the window object
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window", NULL, NULL);
	if (window == nullptr)
	{
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    
	//Specify the actual window rectangle for renderings.
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Register our size callback funtion to GLFW.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//GLFW will capture the mouse and will hide the cursor
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Configure Global OpenGL State
	glEnable(GL_DEPTH_TEST);
	return 0;
}


void renderMesh(const Mesh& mesh, Shader& shader)
{
    shader.use();
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 PV = projection * view;
    shader.setMat4("PVM", PV * model);
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_TRIANGLES, 3 * mesh.getNumTriangles(), GL_UNSIGNED_INT, 0);
}


void renderGround(const Mesh& mesh, Shader& shader)
{
    shader.use();
    //glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    //Lay the surface, scale it up and move it down a bit (it will be moved down so that the car tires will be on the ground)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, groundOffset);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(groundScale));
    glm::mat4 PV = projection * view;
    shader.setMat4("PVM", PV * model);
    //Texture Uniforms
    shader.setFloat("ground_scale", groundScale);
    shader.setInt("ground_texture", 0);
    //Bind the ground texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTextureID);
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_TRIANGLES, 3 * mesh.getNumTriangles(), GL_UNSIGNED_INT, 0);
}


void renderSkybox(const Mesh& mesh, Shader& shader)
{
    glDepthFunc(GL_LEQUAL);
    shader.use();
    //glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
    glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 PV = projection * glm::mat4(glm::mat3(view));
    //Set uniforms
    shader.setMat4("PV", PV);
    //Set skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_TRIANGLES, 3 * mesh.getNumTriangles(), GL_UNSIGNED_INT, 0);
    glDepthFunc(GL_LESS);
}

void renderCar(const Mesh& mesh, Shader& shader)
{
    shader.use();
    //glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, carPosition);
    model = glm::rotate(model, glm::radians(carRotationAngle + carRotationOffset), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 PV = projection * view;
    shader.setMat4("PVM", PV * model);
    glBindVertexArray(mesh.getVAO());
    glDrawElements(GL_TRIANGLES, 3 * mesh.getNumTriangles(), GL_UNSIGNED_INT, 0);
}


//Set the camera with respect to car
void updateCamera(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) //From Left
    {
        glm::vec3 offset = glm::vec3(carFront.z, 0.0f, -carFront.x);
        cameraPosition = carPosition - 10.0f * offset + glm::vec3(0.0f, 3.0f, 0.0f);
        view = glm::lookAt(cameraPosition, carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) //From Right
    {
        glm::vec3 offset = glm::vec3(-carFront.z, 0.0f, carFront.x);
        cameraPosition = carPosition - 10.0f * offset + glm::vec3(0.0f, 3.0f, 0.0f);
        view = glm::lookAt(cameraPosition, carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) //From Back
    {
        cameraPosition = carPosition - 10.0f * carFront + glm::vec3(0.0f, 3.0f, 0.0f);
        view = glm::lookAt(cameraPosition, carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) //From Front
    {
        cameraPosition = carPosition + 10.0f * carFront + glm::vec3(0.0f, 3.0f, 0.0f);
        view = glm::lookAt(cameraPosition, carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
        return;
    }
    
    //If no input
    cameraPosition = carPosition - 10.0f * carFront + glm::vec3(0.0f, 3.0f, 0.0f);
    view = glm::lookAt(cameraPosition, carPosition, glm::vec3(0.0f, 1.0f, 0.0f));
}


void loadScene()
{
    //Ground Mesh
    groundMesh = Mesh("hw2_support_files/obj/ground.obj");
    groundShader = Shader("Shaders/groundShader/ground_shader_vertex.glsl",
                          "Shaders/groundShader/ground_shader_fragment.glsl");
    
    groundTextureID = textureFromFile("hw2_support_files/ground_texture_sand.jpg", false);
    
    //The Skybox
    skyboxMesh = Mesh("hw2_support_files/obj/cube.obj");
    skyboxShader = Shader("Shaders/skyboxShader/skybox_shader_vertex.glsl",
                          "Shaders/skyboxShader/skybox_shader_fragment.glsl");
    
    std::vector<std::string> faces
    {
        std::string("hw2_support_files/skybox_texture_sea/right.jpg"),
        std::string("hw2_support_files/skybox_texture_sea/left.jpg"),
        std::string("hw2_support_files/skybox_texture_sea/top.jpg"),
        std::string("hw2_support_files/skybox_texture_sea/bottom.jpg"),
        std::string("hw2_support_files/skybox_texture_sea/front.jpg"),
        std::string("hw2_support_files/skybox_texture_sea/back.jpg"),
    };
    
    skyboxTextureID = loadCubemap(faces);
    
    
    //The Cybertruck
    carBodyMesh = Mesh("hw2_support_files/obj/cybertruck/cybertruck_body.obj");
    carTiresMesh = Mesh("hw2_support_files/obj/cybertruck/cybertruck_tires.obj");
    carWindowsMesh = Mesh("hw2_support_files/obj/cybertruck/cybertruck_windows.obj");
    carBodyShader = Shader("Shaders/carShader/carbody_shader_vertex.glsl",
                          "Shaders/carShader/carbody_shader_fragment.glsl");
    carTiresShader = Shader("Shaders/carShader/cartires_shader_vertex.glsl",
                          "Shaders/carShader/cartires_shader_fragment.glsl");
    carWindowsShader = Shader("Shaders/carShader/carwindows_shader_vertex.glsl",
                          "Shaders/carShader/carwindows_shader_fragment.glsl");
    
    
}


void renderScene()
{
    renderGround(groundMesh, groundShader);
    //Render the car parts
    renderCar(carBodyMesh, carBodyShader);
    renderCar(carTiresMesh, carTiresShader);
    renderCar(carWindowsMesh, carWindowsShader);
    //Rendering the skybox last.
    renderSkybox(skyboxMesh, skyboxShader);
}



int main()
{
	setupDependencies();
    
    loadScene();
    
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		//Update deltaTime
		updateDeltaTime();
		// input
		processCarMovement(window);
        //Set the camera
        updateCamera(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderScene();
        
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}
