#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iomanip>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <iostream>
#include <skybox/skybox.h>
#include <building/building.h>
#include <render/shader.h>
#include <road/road.h>
#include <windmill/blades.h>
#include <windmill/windmill.h>
#include <model/model.h>
#define _USE_MATH_DEFINES



static GLFWwindow *window;
int windowWidth = 1024;
int windowHeight = 768;
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
double lastX = 512, lastY = 384;  // Starting mouse positionq1
float yaw = 180.0f, pitch = 0.0f; // Initial orientation angle

// Rotate cars
bool rotateCars = false;
bool moveParkedCar = false;
// ---------------------------------------------------------------------------------------------------------------------

// Lighting setup
const glm::vec3 globalLightPosition(200.0f, 600.0f, 200.0f);
glm::vec3 globalLightIntensity(1.0f, 1.0f, 1.0f);  // White light
glm::mat4 lightSpaceMatrix;

// Shadow mapping
static glm::vec3 lightUp(0, 0, 1);
static int shadowMapWidth = 0;
static int shadowMapHeight = 0;

static float depthNear = 200.0f;
static float depthFar = 800.0f;

// OpenGL buffers
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint indexBufferID;
GLuint colorBufferID;
GLuint normalBufferID;

// Shader variable IDs
GLuint depthMapShaders;     // s.vert, s.frag
GLuint shadowRenderShaders; // s2.vert, s2.frag
GLuint lightSpaceMatrixID;  // used to transform vertices into light space
GLuint depthMapID;          // ^ those are stored in depthMapID shadow map
GLuint mvpMatrixID;         // transforms geometry for rendering
GLuint lightPositionID;
GLuint lightIntensityID;

// Helper flag and function to save depth maps for debugging
static bool saveDepth = true;

// This function retrieves and stores the depth map of the default frame buffer
// or a particular frame buffer (indicated by FBO ID) to a PNG image.
static void saveDepthTexture(GLuint fbo, std::string filename) {
	int width = shadowMapWidth;
	int height = shadowMapHeight;
	if (shadowMapWidth == 0 || shadowMapHeight == 0) {
		width = windowWidth;
		height = windowHeight;
	}
	int channels = 3;

	std::vector<float> depth(width * height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_DEPTH_COMPONENT);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::vector<unsigned char> img(width * height * 3);
	for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

	stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}

void renderSceneFromLight(GLuint depthFBO, std::vector<Building> buildings) {
	// Bind the FBO for rendering the depth map
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Configure the light-space matrix
	glm::mat4 lightProjection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, depthNear, depthFar);
	glm::mat4 lightView = glm::lookAt(globalLightPosition, glm::vec3(10.0f, 0.0f, 10.0f), lightUp);
	lightSpaceMatrix = lightProjection * lightView;

	// Render depth map
	for (Building &b : buildings) {
		b.render_first_pass(lightSpaceMatrix, depthMapShaders, globalLightPosition, globalLightIntensity);
	}

	// Unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Reset viewport to the window size
	glViewport(0, 0, windowWidth, windowHeight);
}



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
    window = glfwCreateWindow(windowWidth, windowHeight, "Final Proj", NULL, NULL);
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

	// Prepare shadow map size for shadow mapping. Usually this is the size of the window itself, but on some platforms like Mac this can be 2x the size of the window. Use glfwGetFramebufferSize to get the shadow map size properly.
	glfwGetFramebufferSize(window, &shadowMapWidth, &shadowMapHeight);

	// Load shaders
	depthMapShaders = LoadShadersFromFile("../FinalProj/s.vert", "../FinalProj/s.frag");
	if (depthMapShaders == 0) {
		std::cerr << "Failed to load depthMap shaders." << std::endl;
	}
	shadowRenderShaders = LoadShadersFromFile("../FinalProj/s2.vert", "../FinalProj/s2.frag");
	if (shadowRenderShaders == 0) {
		std::cerr << "Failed to load shadowRender shaders." << std::endl;
	}


	// Create the FBO
	GLuint depthFBO;
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

	// Create depth texture
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	// Define texture properties and allocate memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	// Set texture parameters for depth texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture to FBO’s depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	// Ensure that we don't use any color buffer for this FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: Framebuffer is not complete!" << std::endl;
	}


	// Create objects in the scene

	// Create skybox
    Skybox skybox{};
    skybox.initialize();

	// Create road
	Road road;
	road.initialize(glm::vec3(30.0f, 5.0f, 20.0f), glm::vec3(4, 0, 6), "../FinalProj/road/pavement.jpg");

	// Create inner buildings and intersection
	std::vector<Building> buildings;

	int gridSizeX = 4;          // Number of buildings along x-axis
	int gridSizeZ = 8;          // Number of buildings along z-axis
	float spacing = 50.0f;      // Spacing between buildings
	float outerSpacing = 60.0f; // Spacing between buildings

	// Textures
	const char* textures[] = {
		"../FinalProj/building/modern0.jpg",
		"../FinalProj/building/modern1.jpg",
		"../FinalProj/building/modern2.jpg",
		"../FinalProj/building/modern3.jpg"
	};

	float constant = 75.0f;
	for (int i = 0; i < gridSizeX; ++i) {
		for (int j = 0; j < gridSizeZ; ++j) {
			// Generate random dimensions for building
			float width = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));
			float height = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (40 - 1)));
			float depth = 20 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5 - 1)));

			// Align buildings in grid
			float posX = i * spacing - (gridSizeX * spacing) / 2;
			float posZ = j * spacing - (gridSizeZ * spacing) / 2;

			if(i > gridSizeX / 2 - 1) { posX += constant; }
			if(j > gridSizeZ / 2 - 1) { posZ += constant; }

			// Select random building texture
			int textureIndex = rand() % 4;

			Building b;
			b.initialize(glm::vec3(posX, 0.0f, posZ), glm::vec3(width, height, depth), textures[textureIndex]);
			buildings.push_back(b);
		}
	}


	// Outer square of buildings
	int outerRingSize = 1; // Extra buildings in each direction beyond the grid

	for (int i = -outerRingSize; i <= gridSizeX + outerRingSize; ++i) {
		for (int j = -outerRingSize; j <= gridSizeZ + outerRingSize; ++j) {
			// Skip positions within the original grid
			if (i >= 0 && i <= gridSizeX && j >= 0 && j <= gridSizeZ)  {
				continue;
			}

			float width = 30 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (1)));
			float height = 35 + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (40 - 1)));
			float depth = 30  + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (1)));

			float posX = i * outerSpacing - (gridSizeX * spacing) / 2.0f;
			float posZ = j * outerSpacing - (gridSizeZ * spacing) / 2.0f;

			int textureIndex = rand() % 4;
			Building b;
			b.initialize(glm::vec3(posX, 0.0f, posZ), glm::vec3(width, height, depth), textures[textureIndex]);
			buildings.push_back(b);
		}
	}

	// Create spire (blades, windmill base and connector)
	Windmill windmill;
	windmill.initialize(glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(6.0f, 250.0f, 6.0f));

	Blades blades;
	blades.initialize(glm::vec3(10.0f, 500.0f, 20.0f), glm::vec3(80.0f, 80.0f, 80.0f));

	Building b;
	b.initialize(glm::vec3(10.0f, 495.0f, 10.0f), glm::vec3(6.0f, 6.0f, 20.0f), "../FinalProj/building/white.jpg");
	buildings.push_back(b);


	// Create boulder
	Model boulder;
	if (!boulder.loadModel("../FinalProj/model/concrete_road_barrier_02_4k.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	boulder.setPosition(glm::vec3(350.0f, -115.0f, 30.0f));
	boulder.setScale(glm::vec3(600.0f));
	boulder.setRotation(90, glm::vec3(0.0f, 1.0f, 0.0f));
	boulder.setRotation(275, glm::vec3(1.0f, 0.0f, 0.0f));

	// Create trees
	Model tree;
	if (!tree.loadModel("../FinalProj/model/shrub_04_2k.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	tree.setPosition(glm::vec3(290, -10.0f, 400));
	tree.setScale(glm::vec3(1000.0f));

	Model tree2;
	if (!tree2.loadModel("../FinalProj/model/shrub_04_2k.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	tree2.setPosition(glm::vec3(-120, -10.0f, 420));
	tree2.setRotation(180, glm::vec3(0.0f, 1.0f, 0.0f));
	tree2.setScale(glm::vec3(800.0f));

	Model tree3;
	if (!tree3.loadModel("../FinalProj/model/shrub_04_2k.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	tree3.setPosition(glm::vec3(290, -10.0f, -360));
	tree3.setScale(glm::vec3(1000.0f));

	Model tree4;
	if (!tree4.loadModel("../FinalProj/model/shrub_04_2k.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	tree4.setPosition(glm::vec3(-190, -10.0f, -380));
	tree4.setRotation(180, glm::vec3(0.0f, 1.0f, 0.0f));
	tree4.setScale(glm::vec3(1000.0f));


	// Create cars
	Model car1;
	if (!car1.loadModel("../FinalProj/model/scene2.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	car1.setPosition(glm::vec3(-400, 200.0f, -800));
	car1.setScale(glm::vec3(20.0f));

	Model car2;
	if (!car2.loadModel("../FinalProj/model/scene2.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	car2.setPosition(glm::vec3(800, 200.0f, -100.0f));
	car2.setRotation(270, glm::vec3(0.0f, 1.0f, 0.0f));
	car2.setScale(glm::vec3(20.0f));

	Model car3;
	if (!car3.loadModel("../FinalProj/model/scene2.gltf")) {
		std::cerr << "Failed to load model!" << std::endl;
		return -1;
	}
	car3.setPosition(glm::vec3(-285, -32.0f, 285.0f));
	car3.setScale(glm::vec3(20.0f));


	// Camera setup
	eye_center = glm::vec3(400.0f, 400.0f, 600.0f);

    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 90;
    glm::float32 zNear = 0.1f;
    glm::float32 zFar = 1200.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float fTime = 0.0f; // Time for measuring fps
	unsigned long frames = 0;

	glEnable(GL_DEPTH_TEST);
    do
    {
    	// Render from the light's perspective to generate depth map
    	renderSceneFromLight(depthFBO, buildings);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	// Update time
    	double currentTime = glfwGetTime();
    	float deltaTime = float(currentTime - lastTime);
    	lastTime = currentTime;


        viewMatrix = glm::lookAt(eye_center, lookat, up);

        // Convert view matrix to homogenous coordinates for skybox to have it always appear infinitely far away from camera
        glm::mat4 viewMatrixHomogenous = glm::mat4(glm::mat3(viewMatrix));

    	// Create a combined VP matrix for the skybox
        glm::mat4 vph = projectionMatrix * viewMatrixHomogenous;

    	// Create a combined VP matrix for the buildings
    	glm::mat4 vp = projectionMatrix * viewMatrix;

    	if (saveDepth) {
    		std::string filename = "depth_camera.png";
    		saveDepthTexture(depthFBO, filename);
    		std::cout << "Depth texture saved to " << filename << std::endl;
    		saveDepth = false;
    	}

    	glActiveTexture(GL_TEXTURE1); // Use texture unit 1 for depth map
    	glBindTexture(GL_TEXTURE_2D, depthTexture);

    	glDepthMask(GL_FALSE); // Disable depth writes
        skybox.render(vph);        // Render skybox
    	glDepthMask(GL_TRUE);  // Re-enable depth writes


    	// Render road
    	road.render(vp);

    	// Render buildings
    	 for (Building& b : buildings) {
    	 	glUseProgram(shadowRenderShaders);
    	 	glUniform3fv(glGetUniformLocation(shadowRenderShaders, "lightPosition"), 1, &globalLightPosition[0]);
    	 	glUniform3fv(glGetUniformLocation(shadowRenderShaders, "lightIntensity"), 1, &globalLightIntensity[0]);
    	 	glUniform3fv(glGetUniformLocation(shadowRenderShaders, "viewPosition"), 1, &eye_center[0]);
    	 	glUniformMatrix4fv(glGetUniformLocation(shadowRenderShaders, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    	 	depthMapID = glGetUniformLocation(shadowRenderShaders, "depthMap");
    	 	glUniform1i(depthMapID, depthFBO);

    	 	b.render_second_pass(vp, shadowRenderShaders);
    	 }

    	// Render boulder
    	boulder.render(vp, eye_center);

    	// Render trees
    	tree.render(vp, eye_center);
    	tree2.render(vp, eye_center);
    	tree3.render(vp, eye_center);
    	tree4.render(vp, eye_center);

    	// Render cars
    	car1.render(vp, eye_center);
    	car1.translate(glm::vec3(0.0f,0.0f, 0.05f));
    	car2.render(vp, eye_center);
    	car2.translate(glm::vec3(0.0f,0.0f, 0.02f));
    	car3.render(vp, eye_center);

    	if (rotateCars) {
    		car1.setRotation(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    		car2.setRotation(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    		rotateCars = false;
    	}

    	if (moveParkedCar) {
    		car3.translate(glm::vec3(0.0f,0.0f, 0.03f));
    	}


    	// Render spire
    	windmill.render(vp);
    	blades.render(vp);


    	// FPS tracking
    	// Count number of frames over a few seconds and take average
    	frames++;
    	fTime += deltaTime;
    	if (fTime > 2.0f) {
    		float fps = frames / fTime;
    		frames = 0;
    		fTime = 0;

    		std::stringstream stream;
    		stream << std::fixed << std::setprecision(2) << "Final Project | (FPS): " << fps;
    		glfwSetWindowTitle(window, stream.str().c_str());
    	}


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
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	// Speed of movement
	float speed = 5.0f;

	// Calculate the lookat direction
	glm::vec3 lookDirection = glm::normalize(lookat - eye_center);

	// Calculate the right vector (perpendicular to lookDirection and up)
	glm::vec3 rightDirection = glm::normalize(glm::cross(lookDirection, up));

    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    	eye_center += lookDirection * speed;
    	lookat += lookDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    	eye_center -= lookDirection * speed;
    	lookat -= lookDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    	eye_center -= rightDirection * speed;
    	lookat -= rightDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
    	eye_center += rightDirection * speed;
    	lookat += rightDirection * speed; // Move look-at point to keep orientation
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		rotateCars = true;

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		moveParkedCar = true;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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