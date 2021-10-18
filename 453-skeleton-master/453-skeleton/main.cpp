#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		transformationMatrix(1.0f) // This constructor sets it as the identity matrix
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

    float v1, v2, theta, scaling_factor, target_theta, target_v1, target_v2;
    glm::vec3 direction;

	glm::vec3 position;
	glm::mat4 transformationMatrix;
};

/*

Struct used to keep track of states changed by callbacks. The mouse coordinates are used by the rotation functionality and the up pressed and down pressed is for the translations. Reset is used for when a player hits the j key and the entire game is reset. The functions stateChanged and stateReset are used in the render loop. They check the state of user input and reset default bools respectively.

*/
struct State {
    glm::vec2 mouse_coordinates;
    bool state_changed = true;
    bool mouse_clicked = false;
    bool up_pressed = false;
    bool reset = false;
    bool down_pressed = false;

    bool stateChanged()
    {
        if (mouse_clicked == true || down_pressed == true || up_pressed == true) return true;
        else return false;
    }

    void stateReset()
    {
        bool state_changed = false;
        bool mouse_clicked = false;
        bool up_pressed = false;
        bool reset = false;
        bool down_pressed = false;
    }
};

/*

The callbacks are how the user is able to interact with the game. The up key and down key will move the player along the direction vector they currently are on. A mouse click will change the direction vector of the player and the j key will reset the game to default values. All these values are kept track of using a State struct which tracks the coordinates of the mouse as well as whether the up and down keys are being held down. 

*/
class MyCallbacks : public CallbackInterface {

public:
    MyCallbacks(ShaderProgram &shader, int screen_width, int screen_height) : shader(shader), screen_dimensions(screen_width, screen_height){ }

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}

        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            state.up_pressed = true;
            state.state_changed = true;
        }

        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            state.down_pressed = true;
            state.state_changed = true;
        }

        if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
        {
            state.up_pressed = false;
            state.state_changed = true;
        }

        if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
        {
            state.down_pressed = false;
            state.state_changed = true;
        }

        if (key == GLFW_KEY_J && action == GLFW_PRESS)
        {
            state.reset = true;
        }

        if (key == GLFW_KEY_J && action == GLFW_RELEASE)
        {
            state.reset = false;
        }
    }

    virtual void mouseButtonCallback(int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            state.mouse_clicked = true;
            state.state_changed = true;
        }
    }

    virtual void cursorPosCallback(double xpos, double ypos)
    {
        state.mouse_coordinates = glm::vec2(xpos, ypos);
        state.mouse_coordinates = state.mouse_coordinates / (screen_dimensions - 1.0f);
        state.mouse_coordinates *= 2;
        state.mouse_coordinates -= 1;
        state.mouse_coordinates.y = -state.mouse_coordinates.y;
    }

    void stateHandled()
    {
        state.mouse_clicked = false;
    }

    const State& getState()
    {
        return state;
    }

private:
	ShaderProgram& shader;
    State state;
    glm::vec2 screen_dimensions;
};

/*

These are the positional coordinates of every object initialised in the game before they've been changed by the matrices of the vertex shader.

*/

CPU_Geometry objectGeom() {

	CPU_Geometry retGeom;
 
	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.

    retGeom.verts.push_back(glm::vec3(-1.f , 1.f , 1.f));
    retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 1.f));
    retGeom.verts.push_back(glm::vec3(1.f, -1.f, 1.f));
    retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 1.f));
    retGeom.verts.push_back(glm::vec3(1.f, -1.f, 1.f));
    retGeom.verts.push_back(glm::vec3(1.f, 1.f, 1.f));

    // texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

/*

This function generates a random float value to be used for the positional coordinates of our player. The floats it produces are between -1 and 1.

*/
float makeRandom()
{
    int random = rand() % 600;
    if (random % 2 == 0) random *= -1;
    float randomAdjustor = (float)(random) / 600;

    return randomAdjustor;
}

/*

Works in concert with makeRandom to produce a random float. All this function does is make sure the output from makeRandom is within a certain range. The range will be -x to x.

*/
float keepWithin(float x)
{
    float randomFloat = makeRandom();
    while(randomFloat > x || randomFloat < -x)
    {
        randomFloat = makeRandom();
    }

    return randomFloat;
}

/*

This function will return a unit vector of the same direction as the vector vector_in it takes as input. Does so by finding the magnitude of the input vector and dividing the x and y values by the reciprocal of this. We ignore the z because as a 2d vector we already know it to be 0.

*/
glm::vec3 makeUnitVector(glm::vec3 vector_in)
{
    float magnitude = sqrt(vector_in.x * vector_in.x + vector_in.y * vector_in.y);
    return glm::vec3((vector_in.x / magnitude),(vector_in.y / magnitude), 0.0f);
}

/*

This function finds the appropriate rotation to point the player (ship) towards the mouse click. It first makes a unit vector for where it should be pointing (ie. the new direction vector for the player) and the standard north pointing vector. We then check if the new direction is different from the current one, if it isn't we continue, otherwise we end the function. The dot product of the two vectors (current direction and new direction) is then found and we use this to find angle theta between them. We then account for the previous direction and add this to the angle between the vectors and we have our new theta. Depending on whether the click is to the right or left of the current direction (whether we're rotating clockwise or anticlockwise) we need to do different things and that's why we have an if of or_1. This is the cross product which we use to tell which vector is on the right of the other in the clockwise direction. 

*/
float findRotationTheta(State state, GameObject& ship)
{
    glm::vec3 new_direction = glm::vec3(state.mouse_coordinates.x - ship.v1, state.mouse_coordinates.y - ship.v2, 0.0f);
    new_direction = makeUnitVector(new_direction);

    glm::vec3 standard = glm::vec3(0.0f, 1.0f, 0.0f);

    if (new_direction == ship.direction)
    {
        return ship.target_theta;
    }
    else
    {
        double dot1 = ship.direction.x * new_direction.x + ship.direction.y * new_direction.y;
        double theta1 = acos(dot1);
        double dot2 = standard.x * ship.direction.x + standard.y * ship.direction.y;
        double theta2 = acos(dot2);
        double or_1 = ship.direction.x * -new_direction.y + ship.direction.y * new_direction.x;
        double or_2 = standard.x * -ship.direction.y + standard.y * ship.direction.x;

        if (or_1 > 0)
        {
            if (standard.x > ship.direction.x)
            {
                theta2 = 2 * M_PI - theta2;
            }

            ship.direction = new_direction;
            return theta1 + theta2;
        }
        else 
        {
            double theta;

            if (or_2 < 0)
            {
                theta = theta2 + theta1;
                theta = 2 * M_PI - theta;
            }
            else
            {
                theta = theta2 - theta1;
            }

            ship.direction = new_direction;
            return theta;
        }
    }
}

/*

This is used to detect a close enough threshold to where two angles can be treated as equal. Its used to check if our target angle and current angle are close enough yet. This is an essential function for creating the animation type effect of the rotation.

*/
bool notCloseEnoughAngle(float a, float b)
{
    if (a < b + 0.005f && a > b - 0.005f) return false;
    else return true;
}

/*

This is used to detect a close enough threshold to where two coordinates can be treated as equal. Its used to check if our target position and current position are close enough yet. This is an essential function for creating the animation type effect of the translation.

*/
bool notCloseEnoughPosition(float a, float b)
{
    if (a < b + 0.00005f && a > b - 0.00005f) return false;
    else return true;
}

/*

Is used to move our ship along a particular direction vector by a magnitude decided by us. By multiplying the magnitude by the unit direction vector we have moved along the vector by a certain amount of units. This is then added or subtracted to the positions of the ship to produce forward and backward movement respectively.

*/
std::tuple<float, float> moveShip(GameObject& ship)
{
    float magnitude = 0.005f;
    return std::tuple<float, float>{ship.direction.x * magnitude, ship.direction.y * magnitude};
}

/*

Finds the distance between two points by forming a vector between them and working out the magnitude of said vector.

*/
float distanceBetween(GameObject& a, GameObject& b)
{
    float x = b.v1 - a.v1;
    float y = b.v2 - a.v2;
    float magnitude = sqrt(x * x + y * y);
    return magnitude;
}

/*

Checks whether two objects in our game are close enough to one another. The enough being decided to be 0.25 units.

*/
bool withinOrbit(GameObject& ship, GameObject& diamond)
{
    if (distanceBetween(ship, diamond) < 0.25) return true;
    return false;
}

int main() {
	Log::debug("Starting main");

    int screen_width = 800;
    int screen_height = 800;

	// WINDOW
	glfwInit();
	Window window(screen_width, screen_height, "CPSC 453 Assignment 2"); // can set callbacks at construction if desired

    // SEEDING RAND
    srand(time(NULL));

    GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
    auto callback_controller = std::make_shared<MyCallbacks>(shader, screen_width, screen_height);
	window.setCallbacks(callback_controller); // can also update callbacks to new ones

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
    GameObject diamond_1("textures/diamond.png", GL_NEAREST);
    GameObject diamond_2("textures/diamond.png", GL_NEAREST);
    GameObject diamond_3("textures/diamond.png", GL_NEAREST);
    GameObject diamond_4("textures/diamond.png", GL_NEAREST);

    ship.cgeom = objectGeom();
    diamond_1.cgeom = objectGeom();
    diamond_2.cgeom = objectGeom();
    diamond_3.cgeom = objectGeom();
    diamond_4.cgeom = objectGeom();

    ship.ggeom.setVerts(ship.cgeom.verts);
    ship.ggeom.setTexCoords(ship.cgeom.texCoords);

    diamond_1.ggeom.setVerts(diamond_1.cgeom.verts);
    diamond_1.ggeom.setTexCoords(diamond_1.cgeom.texCoords);

    diamond_2.ggeom.setVerts(diamond_2.cgeom.verts);
    diamond_2.ggeom.setTexCoords(diamond_2.cgeom.texCoords);

    diamond_3.ggeom.setVerts(diamond_3.cgeom.verts);
    diamond_3.ggeom.setTexCoords(diamond_3.cgeom.texCoords);

    diamond_4.ggeom.setVerts(diamond_4.cgeom.verts);
    diamond_4.ggeom.setTexCoords(diamond_4.cgeom.texCoords);

    // Default Locations setting 

    diamond_1.v1 = -0.7 + (makeRandom() / 6);
    diamond_1.v2 = 0.7 - (makeRandom() / 6);
    diamond_1.theta = 0.0f;
    diamond_1.scaling_factor = 0.125f;

    diamond_2.v1 = 0.7 - (makeRandom() / 6);
    diamond_2.v2 = 0.7 - (makeRandom() / 6);
    diamond_2.theta = 0.0f;
    diamond_2.scaling_factor = 0.125f;

    diamond_3.v1 = -0.7 + (makeRandom() / 6);
    diamond_3.v2 = -0.7 + (makeRandom() / 6);
    diamond_3.theta = 0.0f;
    diamond_3.scaling_factor = 0.125f;

    diamond_4.v1 = 0.7 - (makeRandom() / 6);
    diamond_4.v2 = -0.7 + (makeRandom() / 6);
    diamond_4.theta = 0.0f;
    diamond_4.scaling_factor = 0.125f;

    ship.v1 = keepWithin(0.5);
    ship.v2 = keepWithin(0.5);

    ship.target_v1 = ship.v1;
    ship.target_v2 = ship.v2;
    ship.theta = 0.0f;
    ship.target_theta = 0.0f;
    ship.direction = makeUnitVector(glm::vec3(ship.v1 - ship.v1, 1.0f - ship.v2, 0.0f));
    ship.scaling_factor = 0.125f;
    int score_value = 0;

    State state = callback_controller->getState();

    // RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

        state = callback_controller->getState();

        if (state.reset == true)
        {
            // Default Locations setting

            diamond_1.v1 = -0.7 + (makeRandom() / 6);
            diamond_1.v2 = 0.7 - (makeRandom() / 6);
            diamond_1.theta = 0.0f;
            diamond_1.scaling_factor = 0.125f;

            diamond_2.v1 = 0.7 - (makeRandom() / 6);
            diamond_2.v2 = 0.7 - (makeRandom() / 6);
            diamond_2.theta = 0.0f;
            diamond_2.scaling_factor = 0.125f;

            diamond_3.v1 = -0.7 + (makeRandom() / 6);
            diamond_3.v2 = -0.7 + (makeRandom() / 6);
            diamond_3.theta = 0.0f;
            diamond_3.scaling_factor = 0.125f;

            diamond_4.v1 = 0.7 - (makeRandom() / 6);
            diamond_4.v2 = -0.7 + (makeRandom() / 6);
            diamond_4.theta = 0.0f;
            diamond_4.scaling_factor = 0.125f;

            ship.v1 = keepWithin(0.5);
            ship.v2 = keepWithin(0.5);

            ship.target_v1 = ship.v1;
            ship.target_v2 = ship.v2;
            ship.theta = 0.0f;
            ship.target_theta = 0.0f;
            ship.direction = makeUnitVector(glm::vec3(ship.v1 - ship.v1, 1.0f - ship.v2, 0.0f));
            ship.scaling_factor = 0.125f;
            score_value = 0;

            state.stateReset();
        }

        if (state.stateChanged())
        {
            if(state.mouse_clicked)
            {
                ship.target_theta = findRotationTheta(state, ship);
            }

            if(state.up_pressed)
            {
                std::tuple new_positions = moveShip(ship);
                ship.target_v1 += std::get<0>(new_positions);
                ship.target_v2 += std::get<1>(new_positions);
            }

            if(state.down_pressed)
            {
                std::tuple new_positions = moveShip(ship);
                ship.target_v1 -= std::get<0>(new_positions);
                ship.target_v2 -= std::get<1>(new_positions);
            }

            callback_controller->stateHandled();
        }

		shader.use();

        glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        diamond_1.ggeom.bind();

        GLint location_1 = glGetUniformLocation(shader.getID(), "scaling_factor");
        glUniform1f(location_1, diamond_1.scaling_factor);
        GLint location_2 = glGetUniformLocation(shader.getID(), "theta");
        glUniform1f(location_2, diamond_1.theta);
        GLint location_3 = glGetUniformLocation(shader.getID(), "v1");
        glUniform1f(location_3, diamond_1.v1);
        GLint location_4 = glGetUniformLocation(shader.getID(), "v2");
        glUniform1f(location_4, diamond_1.v2);

        diamond_1.texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        diamond_1.texture.unbind();

        diamond_2.ggeom.bind();

        location_1 = glGetUniformLocation(shader.getID(), "scaling_factor");
        glUniform1f(location_1, diamond_2.scaling_factor);
        location_2 = glGetUniformLocation(shader.getID(), "theta");
        glUniform1f(location_2, diamond_2.theta);
        location_3 = glGetUniformLocation(shader.getID(), "v1");
        glUniform1f(location_3, diamond_2.v1);
        location_4 = glGetUniformLocation(shader.getID(), "v2");
        glUniform1f(location_4, diamond_2.v2);

        diamond_2.texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        diamond_2.texture.unbind();

        diamond_3.ggeom.bind();

        location_1 = glGetUniformLocation(shader.getID(), "scaling_factor");
        glUniform1f(location_1, diamond_3.scaling_factor);
        location_2 = glGetUniformLocation(shader.getID(), "theta");
        glUniform1f(location_2, diamond_3.theta);
        location_3 = glGetUniformLocation(shader.getID(), "v1");
        glUniform1f(location_3, diamond_3.v1);
        location_4 = glGetUniformLocation(shader.getID(), "v2");
        glUniform1f(location_4, diamond_3.v2);

        diamond_3.texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        diamond_3.texture.unbind();

        diamond_4.ggeom.bind();

        location_1 = glGetUniformLocation(shader.getID(), "scaling_factor");
        glUniform1f(location_1, diamond_4.scaling_factor);
        location_2 = glGetUniformLocation(shader.getID(), "theta");
        glUniform1f(location_2, diamond_4.theta);
        location_3 = glGetUniformLocation(shader.getID(), "v1");
        glUniform1f(location_3, diamond_4.v1);
        location_4 = glGetUniformLocation(shader.getID(), "v2");
        glUniform1f(location_4, diamond_4.v2);

        diamond_4.texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        diamond_4.texture.unbind();

        ship.ggeom.bind();

        location_1 = glGetUniformLocation(shader.getID(), "scaling_factor");
        glUniform1f(location_1, ship.scaling_factor);
        location_2 = glGetUniformLocation(shader.getID(), "theta");
        if (notCloseEnoughAngle(ship.theta, ship.target_theta))
        {
            if (ship.theta < ship.target_theta) ship.theta += 0.05f;
            if (ship.theta > ship.target_theta) ship.theta -= 0.05f;
        } 
        else ship.theta = ship.target_theta;
        glUniform1f(location_2, ship.theta);
        location_3 = glGetUniformLocation(shader.getID(), "v1");
        if (notCloseEnoughPosition(ship.v1, ship.target_v1))
        {
            if (ship.v1 < ship.target_v1) ship.v1 += (abs(ship.v1 - ship.target_v1) * 1.0005f);
            if (ship.v1 > ship.target_v1) ship.v1 -= (abs(ship.v1 - ship.target_v1) * 1.0005f);
        }
        else ship.v1 = ship.target_v1;
        glUniform1f(location_3, ship.v1);
        location_4 = glGetUniformLocation(shader.getID(), "v2");
        if (notCloseEnoughPosition(ship.v2, ship.target_v2))
        {
            if (ship.v2 < ship.target_v2) ship.v2 += (abs(ship.v2 - ship.target_v2) * 1.0005f);
            if (ship.v2 > ship.target_v2) ship.v2 -= (abs(ship.v2 - ship.target_v2) * 1.0005f);
        }
        else ship.v2 = ship.target_v2;
        glUniform1f(location_4, ship.v2);

        ship.texture.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ship.texture.unbind();

        glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

        if (withinOrbit(ship, diamond_1))
        {
            score_value += 1;
            diamond_1.v1 = -5.0f;
            ship.scaling_factor *= 1.05;
        } 

        if (withinOrbit(ship, diamond_2))
        {
            score_value += 1;
            diamond_2.v1 = -5.0f;
            ship.scaling_factor *= 1.05;
        }

        if (withinOrbit(ship, diamond_3))
        {
            score_value += 1;
            diamond_3.v1 = -5.0f;
            ship.scaling_factor *= 1.05;
        }

        if (withinOrbit(ship, diamond_4))
        {
            score_value += 1;
            diamond_4.v1 = -5.0f;
            ship.scaling_factor *= 1.05;
        }

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
        if (score_value < 4)
        {
            ImGui::Text("Score: %d", score_value); // Second parameter gets passed into "%d"
        }
		else 
        {
            ImGui::Text("Congratulations !! You've won the game"); // Second parameter gets passed into "%d"
        }

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
