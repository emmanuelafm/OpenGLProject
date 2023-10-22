/*
* CS330 - Module Five Milestone
* Credit to original author: Professor Brian Battersby, SNHU
* Student: Emmanuela Filev-Mihalak
* September 23, 2023
*/

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"     // Image loading Utility functions


// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshes.h"
#include "camera.h"


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "CS330 OpenGL Project - Emma"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;


	glm::vec2 gUVScale(5.0f, 5.0f);
	GLint gTexWrapMode = GL_REPEAT;
	// 
	// Shader program
	GLuint gSurfaceProgramId;
	GLuint gLightProgramId;

	// Texture
	GLuint gTextureIdCup;
	GLuint gTextureIdLid;
	GLuint gTextureIdTable;
	GLuint gTextureIdCandle;
	GLuint gTextureIdCandleLid;
	GLuint gTextureIdPencilBody;
	GLuint gTextureIdPencilTip;
	GLuint gTextureIdEraserHold;
	GLuint gTextureIdEraser;
	GLuint gTextureIdEnvelope;

	// camera
	Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	//Shape Meshes from Professor Brian
	Meshes meshes;

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
//void UCreateMesh(GLMesh &mesh);
//void UDestroyMesh(GLMesh &mesh);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool isPerspective = true;



/* Vertex Shader Source Code - successfully edited 10/5 using lighting.cpp */
const GLchar* surfaceVertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexFragmentNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;


//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexFragmentNormal = mat3(transpose(inverse(model))) * vertexNormal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code - successfully edited 10/5 using lighting.cpp  */
const GLchar* surfaceFragmentShaderSource = GLSL(440,
	in vec3 vertexFragmentNormal;
in vec3 vertexFragmentPos;
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec4 objectColor;
uniform vec3 ambientColor;
uniform vec3 light1Color;
uniform vec3 light1Position;
uniform vec3 light2Color;
uniform vec3 light2Position;
uniform vec3 light3Color;
uniform vec3 light3Position;
uniform vec3 light4Color;
uniform vec3 light4Position;
uniform vec3 light5Color;
uniform vec3 light5Position;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;
uniform bool ubHasTexture;
uniform float ambientStrength = 0.3f; // Set ambient or global lighting strength
uniform float specularIntensity1 = 0.5f;
uniform float highlightSize1 = 4.0f;
uniform float specularIntensity2 = 0.5f;
uniform float highlightSize2 = 4.0f;
uniform float specularIntensity3= 0.5f;
uniform float highlightSize3 = 4.0f;
uniform float specularIntensity4 = 0.5f;
uniform float highlightSize4 = 4.0f;
uniform float specularIntensity5 = 0.2f;
uniform float highlightSize5 = 2.0f;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting
	vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

	//**Calculate Diffuse lighting**
	vec3 norm = normalize(vertexFragmentNormal); // Normalize vectors to 1 unit

	// light 1
	vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color
	// light 2
	vec3 light2Direction = normalize(light2Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color
	// light 3
	vec3 light3Direction = normalize(light3Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact3 = max(dot(norm, light3Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse3 = impact3 * light3Color; // Generate diffuse light color
	// light 4
	vec3 light4Direction = normalize(light4Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact4 = max(dot(norm, light4Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse4 = impact4 * light4Color; // Generate diffuse light color
	// light 5
	vec3 light5Direction = normalize(light5Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact5 = max(dot(norm, light5Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse5 = impact5 * light5Color; // Generate diffuse light color

	//**Calculate Specular lighting**
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	//light 1
	vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
	float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
	vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;
	//light 2
	vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;
	//light 3
	vec3 reflectDir3 = reflect(-light3Direction, norm);// Calculate reflection vector
	float specularComponent3 = pow(max(dot(viewDir, reflectDir3), 0.0), highlightSize3);
	vec3 specular3 = specularIntensity3 * specularComponent3 * light3Color;
	//light 4
	vec3 reflectDir4 = reflect(-light4Direction, norm);// Calculate reflection vector
	float specularComponent4 = pow(max(dot(viewDir, reflectDir4), 0.0), highlightSize4);
	vec3 specular4 = specularIntensity4 * specularComponent4 * light4Color;
	//light 4
	vec3 reflectDir5 = reflect(-light5Direction, norm);// Calculate reflection vector
	float specularComponent5 = pow(max(dot(viewDir, reflectDir5), 0.0), highlightSize5);
	vec3 specular5 = specularIntensity5 * specularComponent5 * light5Color;

	//**Calculate phong result**
	//Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate);
	vec3 phong1;
	vec3 phong2;
	vec3 phong3;
	vec3 phong4;
	vec3 phong5;

	if (ubHasTexture == true)
	{
		phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
		phong3 = (ambient + diffuse3 + specular3) * textureColor.xyz;
		phong4 = (ambient + diffuse4 + specular4) * textureColor.xyz;
		phong5 = (ambient + diffuse5 + specular5) * textureColor.xyz;
	}
	else
	{
		phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
		phong3 = (ambient + diffuse3 + specular3) * objectColor.xyz;
		phong4 = (ambient + diffuse4 + specular4) * objectColor.xyz;
		phong5 = (ambient + diffuse5 + specular5) * objectColor.xyz;
	}

	fragmentColor = vec4(phong1 + phong2 + phong3 + phong4 + phong5, 1.0); // Send lighting results to GPU
	//fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
);


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Light Object Shader Source Code - added 10/5 using lighting.cpp. textures loading, no light source yet*/
const GLchar* lightVertexShaderSource = GLSL(330,
	layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
);
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Light Object Shader Source Code*/
const GLchar* lightFragmentShaderSource = GLSL(330,
	out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0); // set all 4 vector values to 1.0
}
);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	//UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
	meshes.CreateMeshes();

	// Create the shader program
	if (!UCreateShaderProgram(surfaceVertexShaderSource, surfaceFragmentShaderSource, gSurfaceProgramId))
		return EXIT_FAILURE;

	if (!UCreateShaderProgram(lightVertexShaderSource, lightFragmentShaderSource, gLightProgramId))
		return EXIT_FAILURE;

	// Load texture 
	const char* texFilename1 = "resources/textures/cuptexture.jpg";
	if (!UCreateTexture(texFilename1, gTextureIdCup))
	{
		cout << "Failed to load texture " << texFilename1 << endl;
	}
	const char* texFilename2 = "resources/textures/lid.png";
	if (!UCreateTexture(texFilename2, gTextureIdLid))
	{
		cout << "Failed to load texture " << texFilename2 << endl;
	}
	const char* texFilename3 = "resources/textures/galaxy.png";
	if (!UCreateTexture(texFilename3, gTextureIdTable))
	{
		cout << "Failed to load texture " << texFilename3 << endl;
	}
	const char* texFilename4 = "resources/textures/candle1.png";
	if (!UCreateTexture(texFilename4, gTextureIdCandle))
	{
		cout << "Failed to load texture " << texFilename4 << endl;
	}
	const char* texFilename5 = "resources/textures/candlelid.jpg";
	if (!UCreateTexture(texFilename5, gTextureIdCandleLid))
	{
		cout << "Failed to load texture " << texFilename5 << endl;
	}
	const char* texFilename6 = "resources/textures/pencil.jpg";
	if (!UCreateTexture(texFilename6, gTextureIdPencilBody))
	{
		cout << "Failed to load texture " << texFilename6 << endl;
	}
	const char* texFilename7 = "resources/textures/penciltip.jpg";
	if (!UCreateTexture(texFilename7, gTextureIdPencilTip))
	{
		cout << "Failed to load texture " << texFilename7 << endl;
	}
	const char* texFilename8 = "resources/textures/eraserhold.jpg";
	if (!UCreateTexture(texFilename8, gTextureIdEraserHold))
	{
		cout << "Failed to load texture " << texFilename8 << endl;
	}
	const char* texFilename9 = "resources/textures/eraser.jpg";
	if (!UCreateTexture(texFilename9, gTextureIdEraser))
	{
		cout << "Failed to load texture " << texFilename9 << endl;
	}
	const char* texFilename10 = "resources/textures/envelope.png";
	if (!UCreateTexture(texFilename10, gTextureIdEnvelope))
	{
		cout << "Failed to load texture " << texFilename10 << endl;
	}

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureIdCup);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gTextureIdLid);
	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 2);
	glActiveTexture(GL_TEXTURE2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gTextureIdCandle);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gTextureIdCandleLid);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gTextureIdPencilBody);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gTextureIdPencilTip);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, gTextureIdEraserHold);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, gTextureIdEraser);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gTextureIdEnvelope);


	glUniform2f(glGetUniformLocation(gSurfaceProgramId, "uvScale"), gUVScale.x, gUVScale.y);


	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gCamera.Position = glm::vec3(0.0f, 2.0f, 2.0f);
	gCamera.Front = glm::vec3(0.0, -1.0, -2.0f);
	gCamera.Up = glm::vec3(0.0, 1.0, 0.0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	//UDestroyMesh(gMesh);
	meshes.DestroyMeshes();


	// Release shader program
	UDestroyShaderProgram(gSurfaceProgramId);
	UDestroyShaderProgram(gLightProgramId);


	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		isPerspective = !isPerspective;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}

// Functioned called to render a frame
void URender()
{
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint objectColorLoc;
	GLint viewPosLoc;
	GLint ambStrLoc;
	GLint ambColLoc;
	GLint light1ColLoc;
	GLint light1PosLoc;
	GLint light2ColLoc;
	GLint light2PosLoc;
	GLint light3ColLoc;
	GLint light3PosLoc;
	GLint light4ColLoc;
	GLint light4PosLoc;
	GLint light5ColLoc;
	GLint light5PosLoc;
	GLint objColLoc;
	GLint specInt1Loc;
	GLint highlghtSz1Loc;
	GLint specInt2Loc;
	GLint highlghtSz2Loc;
	GLint specInt3Loc;
	GLint highlghtSz3Loc;
	GLint specInt4Loc;
	GLint highlghtSz4Loc;
	GLint specInt5Loc;
	GLint highlghtSz5Loc;
	GLint uHasTextureLoc;
	bool ubHasTextureVal;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;


	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transforms the camera
	view = gCamera.GetViewMatrix();

	// Creates a orthographic projection
	//projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	if (isPerspective) {
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else {
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}

	// Set the shader to be used
	glUseProgram(gSurfaceProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gSurfaceProgramId, "model");
	viewLoc = glGetUniformLocation(gSurfaceProgramId, "view");
	projLoc = glGetUniformLocation(gSurfaceProgramId, "projection");
	viewPosLoc = glGetUniformLocation(gSurfaceProgramId, "viewPosition");
	ambStrLoc = glGetUniformLocation(gSurfaceProgramId, "ambientStrength");
	ambColLoc = glGetUniformLocation(gSurfaceProgramId, "ambientColor");
	light1ColLoc = glGetUniformLocation(gSurfaceProgramId, "light1Color");
	light1PosLoc = glGetUniformLocation(gSurfaceProgramId, "light1Position");
	light2ColLoc = glGetUniformLocation(gSurfaceProgramId, "light2Color");
	light2PosLoc = glGetUniformLocation(gSurfaceProgramId, "light2Position");
	light3ColLoc = glGetUniformLocation(gSurfaceProgramId, "light3Color");
	light3PosLoc = glGetUniformLocation(gSurfaceProgramId, "light3Position");
	light4ColLoc = glGetUniformLocation(gSurfaceProgramId, "light4Color");
	light4PosLoc = glGetUniformLocation(gSurfaceProgramId, "light4Position");
	light5ColLoc = glGetUniformLocation(gSurfaceProgramId, "light5Color");
	light5PosLoc = glGetUniformLocation(gSurfaceProgramId, "light5Position");
	objColLoc = glGetUniformLocation(gSurfaceProgramId, "objectColor");
	specInt1Loc = glGetUniformLocation(gSurfaceProgramId, "specularIntensity1");
	highlghtSz1Loc = glGetUniformLocation(gSurfaceProgramId, "highlightSize1");
	specInt2Loc = glGetUniformLocation(gSurfaceProgramId, "specularIntensity2");
	highlghtSz2Loc = glGetUniformLocation(gSurfaceProgramId, "highlightSize2");
	specInt3Loc = glGetUniformLocation(gSurfaceProgramId, "specularIntensity3");
	highlghtSz3Loc = glGetUniformLocation(gSurfaceProgramId, "highlightSize3");
	specInt4Loc = glGetUniformLocation(gSurfaceProgramId, "specularIntensity4");
	highlghtSz4Loc = glGetUniformLocation(gSurfaceProgramId, "highlightSize4");
	specInt5Loc = glGetUniformLocation(gSurfaceProgramId, "specularIntensity5");
	highlghtSz5Loc = glGetUniformLocation(gSurfaceProgramId, "highlightSize5");
	uHasTextureLoc = glGetUniformLocation(gSurfaceProgramId, "ubHasTexture");
	objectColorLoc = glGetUniformLocation(gSurfaceProgramId, "uObjectColor");


	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the camera view location
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);
	//set ambient lighting strength
	glUniform1f(ambStrLoc, 0.4f);
	//set ambient color
	glUniform3f(ambColLoc, 0.1f, 0.1f, 0.1f);
	glUniform3f(light1ColLoc, 0.45f, 0.45f, 0.45f);
	glUniform3f(light1PosLoc, -9.0f, 3.0f, 8.0f);
	glUniform3f(light2ColLoc, 0.6f, 0.4f, 0.4f);
	glUniform3f(light2PosLoc, 9.0f, 3.0f, 8.0f);
	glUniform3f(light3ColLoc, 0.45f, 0.45f, 0.45f);
	glUniform3f(light3PosLoc, -5.0f, 3.0f, -6.0f);
	glUniform3f(light4ColLoc, 0.6f, 0.4f, 0.4f);
	glUniform3f(light4PosLoc, 5.0f, 3.0f, -6.0f);
	// top light
	glUniform3f(light4ColLoc, 0.3f, 0.3f, 0.3f);
	glUniform3f(light4PosLoc, 0.0f, 10.0f, 0.0f);
	//set specular intensity
	glUniform1f(specInt1Loc, .3f);
	glUniform1f(specInt2Loc, .3f);
	glUniform1f(specInt3Loc, .1f);
	glUniform1f(specInt4Loc, .1f);
	//set specular highlight size
	glUniform1f(highlghtSz1Loc, 1.0f);
	glUniform1f(highlghtSz2Loc, 1.0f);
	glUniform1f(highlghtSz3Loc, 1.0f);
	glUniform1f(highlghtSz4Loc, 1.0f);

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);


	// Activate the VBOs contained within the mesh's VAO
	// Desk Pad
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// point to table texture slot

	glBindTexture(GL_TEXTURE_2D, gTextureIdTable);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(4.0f, 1.0f, 7.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Color
	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.1f, 0.1f, 0.1f, 0.1f);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);



	// Activate the VBOs contained within the mesh's VAO
	// Coffee cup
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);


	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 0);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.7f, 1.4f, 0.7f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.41f, 4.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Color
	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.85f, 0.85f, 0.85f, 0.85f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);        //bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);        //top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);    //sides
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);



	// Activate the VBOs contained within the mesh's VAO
	// Coffee Lid
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);


	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 1);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.7f, 0.3f, 0.7f));
	// 2. Rotate the object
	rotation = glm::rotate(0.2f, glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.39f, 4.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	// Color
	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.9f, 0.9f, 0.9f, 0.9f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);        //bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);        //top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);    //sides
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Activate the VBOs contained within the mesh's VAO
	// Outer ring of lid
	glBindVertexArray(meshes.gTorusMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 1);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.7f, 0.7f, 0.7f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.41f, 4.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Color
	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.82f, 0.82f, 0.82f, 0.82f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Candle Bottom
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 3);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-2.0f, -1.0f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);



	// Activate the VBOs contained within the mesh's VAO
	// Candle lid
	glBindVertexArray(meshes.gCylinderMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 4);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.1f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-2.0f, 0.0f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Activate the VBOs contained within the mesh's VAO
	// Pencil - body
	glBindVertexArray(meshes.gCylinderMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 5);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 2.0f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -0.95f, 1.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Activate the VBOs contained within the mesh's VAO
	// Pencil - tip
	glBindVertexArray(meshes.gConeMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 6);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.2f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -0.95f, 1.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 1.0f, 0.0f, 1.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_STRIP, 36, 108);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Activate the VBOs contained within the mesh's VAO
	// Pencil - silver eraser holder
	glBindVertexArray(meshes.gCylinderMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 7);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.2f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -0.95f, 3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	// Pencil - eraser
	glBindVertexArray(meshes.gSphereMesh.vao);

	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 8);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.05f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -0.95f, 3.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Activate the VBOs contained within the mesh's VAO
	// Envelope
	glBindVertexArray(meshes.gBoxMesh.vao);
	
	glUniform1i(glGetUniformLocation(gSurfaceProgramId, "uTexture"), 9);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(3.0f, 0.01f, 1.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.5f, -0.97f, -2.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gSurfaceProgramId, objectColorLoc, 0.5f, 0.5f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);


	// Set the shader to be used
	glUseProgram(gLightProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gLightProgramId, "model");
	viewLoc = glGetUniformLocation(gLightProgramId, "view");
	projLoc = glGetUniformLocation(gLightProgramId, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPyramid4Mesh.vao);

	// 1. Scales the object by 2
	scale = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
	// 2. Rotates shape by 15 degrees in the x axis
	rotation = glm::rotate(-0.2f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-5.0f, 3.0f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// 1. Scales the object by 2
	scale = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
	// 2. Rotates shape by 15 degrees in the x axis
	rotation = glm::rotate(-0.2f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(9.0f, 3.0f, 9.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// 1. Scales the object by 2
	scale = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
	// 2. Rotates shape by 15 degrees in the x axis
	rotation = glm::rotate(-0.2f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(-5.0f, 3.0f, -6.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// 1. Scales the object by 2
	scale = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
	// 2. Rotates shape by 15 degrees in the x axis
	rotation = glm::rotate(-0.2f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(5.0f, 3.0f, -6.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	// 1. Scales the object by 2
	scale = glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));
	// 2. Rotates shape by 15 degrees in the x axis
	rotation = glm::rotate(-0.2f, glm::vec3(1.0, 0.0f, 0.0f));
	// 3. Place object at the origin
	translation = glm::translate(glm::vec3(0.0f, 10.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	glBindVertexArray(0);

	glUseProgram(0);


	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}



// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}