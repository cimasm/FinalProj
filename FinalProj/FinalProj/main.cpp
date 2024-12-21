#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <iostream>
#include <skybox/skybox.h>
#include <building/building.h>
#include <road/road.h>
#include <windmill/blades.h>
#include <windmill/windmill.h>

#define _USE_MATH_DEFINES

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// OpenGL camera view parameters
static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 300.0f;

// For mouse
double lastX = 512, lastY = 384;  // Starting mouse position (center of the screen)
float yaw = -90.0f, pitch = 0.0f; // Initialize orientation angles
bool firstMouse = true;           // To avoid sudden jumps at the start

// For spire maybe if needed
Building* spire;

int main(void) {
    // Initialise GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

	// Set the mouse callback
	glfwSetCursorPosCallback(window, mouse_callback);

	// Hide cursor and lock its position in the centre
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

	// Create skybox
    Skybox skybox{};
    skybox.initialize();

	// Create road
	Road road;
	road.initialize(glm::vec3(30.0f, 0.0f, 20.0f), glm::vec3(4, 0, 6), "../FinalProj/road/road.jpg");


	// Create inner buildings and intersection
	std::vector<Building> buildings;

	int gridSizeX = 4;     // Number of buildings along x-axis
	int gridSizeZ = 8;     // Number of buildings along z-axis
	float spacing = 50.0f; // Spacing between buildings
	float outerSpacing = 60.0f; // Spacing between buildings

	// Available textures
	const char* textures[] = {
		"../FinalProj/building/facade0.jpg",
		"../FinalProj/building/facade1.jpg",
		"../FinalProj/building/facade3.jpg",
		"../FinalProj/building/facade4.jpg"
	};

	float constant = 75.0f;
	for (int i = 0; i < gridSizeX; ++i) {
		for (int j = 0; j < gridSizeZ; ++j) {
			// Generate random dimensions for building
			float width = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));
			float height = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));
			float depth = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));

			// Align buildings in grid
			float posX = i * spacing - (gridSizeX * spacing) / 2;
			float posZ = j * spacing - (gridSizeZ * spacing) / 2;

			if(i > gridSizeX / 2 - 1) { posX += constant; }
			if(j > gridSizeZ / 2 - 1) { posZ += constant; }

			// Select random building texture
			int textureIndex = rand() % 4;

			Building b;
			b.initialize(glm::vec3(posX, 0.0f, posZ), glm::vec3(width, height, depth), textures[textureIndex]); // Keep Y position at 0

			buildings.push_back(b);
		}
	}


	// Outer square dimensions
	int outerRingSize = 1; // Extra buildings in each direction beyond the grid

	// Outer square of buildings
	for (int i = -outerRingSize; i <= gridSizeX + outerRingSize; ++i) {
		for (int j = -outerRingSize; j <= gridSizeZ + outerRingSize; ++j) {
			// Skip positions within the original grid
			if (i >= 0 && i <= gridSizeX && j >= 0 && j <= gridSizeZ)  {
				continue;
			}

			// Generate random dimensions for building
			float width = 30 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (1)));
			float height = 25 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));
			float depth = 30  + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (1)));

			float posX = i * outerSpacing - (gridSizeX * spacing) / 2.0f;
			float posZ = j * outerSpacing - (gridSizeZ * spacing) / 2.0f;

			int textureIndex = rand() % 4;
			Building b;
			b.initialize(glm::vec3(posX, 0.0f, posZ), glm::vec3(width, height, depth), textures[textureIndex]);
			buildings.push_back(b);
		}
	}

	// Create spire and blades
	Windmill windmill;
	windmill.initialize(glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(6.0f, 200.0f, 6.0f));

	Building b;
	b.initialize(glm::vec3(10.0f, 395.0f, 10.0f), glm::vec3(6.0f, 6.0f, 20.0f), "../FinalProj/building/white.jpg");
	buildings.push_back(b);

	Blades blades;
	blades.initialize(glm::vec3(10.0f, 400.0f, 20.0f), glm::vec3(80.0f, 80.0f, 80.0f));


    // Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 90;
    glm::float32 zNear = 0.1f;
    glm::float32 zFar = 800.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	glEnable(GL_DEPTH_TEST);
    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMatrix = glm::lookAt(eye_center, lookat, up);

        // Convert view matrix to homogenous coordinates for skybox to have it always appear infinitely far away from camera
        glm::mat4 viewMatrixHomogenous = glm::mat4(glm::mat3(viewMatrix));

    	// Create a combined VP matrix for the skybox
        glm::mat4 vph = projectionMatrix * viewMatrixHomogenous;

    	// Create a combined VP matrix for the buildings
    	glm::mat4 vp = projectionMatrix * viewMatrix;

    	glDepthMask(GL_FALSE); // Disable depth writes
        // Render the skybox with this vp matrix
        skybox.render(vph);
    	glDepthMask(GL_TRUE);  // Re-enable depth writes

    	// render road
    	road.render(vp);

    	// render buildings
    	for(Building b : buildings) {
    		b.render(vp);
    	}

    	// render spire and blades
    	windmill.render(vp);
    	blades.render(vp);



        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    // Clean up
    skybox.cleanup();
	for(Building b : buildings) {
		b.cleanup();
	}


	road.cleanup();
	skybox.cleanup();
	windmill.cleanup();
	blades.cleanup();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	// Speed of movement
	float speed = 5.0f;

	// Calculate the lookat direction
	glm::vec3 lookDirection = glm::normalize(lookat - eye_center);

	// Calculate the right vector (perpendicular to lookDirection and up)
	glm::vec3 rightDirection = glm::normalize(glm::cross(lookDirection, up));


    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        viewAzimuth = 0.5f;
        viewPolar = 0.5f;
        eye_center.y = viewDistance * cos(viewPolar);
        eye_center.x = viewDistance * cos(viewAzimuth);
        eye_center.z = viewDistance * sin(viewAzimuth);
    	lookat = glm::vec3(0, 0, 0);
        std::cout << "Reset." << std::endl;
    }

    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
    	eye_center += lookDirection * speed;
    	lookat += lookDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
    	eye_center -= lookDirection * speed;
    	lookat -= lookDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
    	eye_center -= rightDirection * speed;
    	lookat -= rightDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
    	eye_center += rightDirection * speed;
    	lookat += rightDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Initialize the starting position of the mouse
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Calculate the mouse offset since the last frame
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos; // Reversed because y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	// Sensitivity factor to control mouse movement speed
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Update yaw (left/right) and pitch (up/down)
	yaw += xOffset;
	pitch += yOffset;

	// Clamp pitch to avoid flipping the camera
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	// Update the camera's direction
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookat = eye_center + glm::normalize(front);
}