
/*
#include <iostream>
#include <thread>

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "glm/glm.hpp"


#include "core/CppGameEngine.h"
#include "core/Scene.h"
#include "assets/Assets.h"


void setupGLFW() {

    // Initialize GLFW. Most GLFW functions will not work before doing this.
    if (!glfwInit())
        throw -1;

    // Configure GLFW
    glfwDefaultWindowHints(); // optional, the current window hints are already the default
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // the window will stay hidden after creation
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // the window will be resizable

    // Create the window
    GLFWwindow* window = glfwCreateWindow(300, 300, "Hello World!", NULL, NULL);
    if (window == NULL)
        throw -2;

    // Setup a key callback. It will be called every time a key is pressed, repeated or released.
    //glfwSetKeyCallback(window, (window, key, scancode, action, mods) -> {
    //    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    //        glfwSetWindowShouldClose(window, true); // We will detect this in the rendering loop
    //});

    int width = 0;
    int height = 0;
    glfwGetWindowSize(window, &width, &height);

        // Get the resolution of the primary monitor
    const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        // Center the window
    glfwSetWindowPos(
        window,
        (vidmode->width - width) / 2,
        (vidmode->height - height) / 2
    );

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    // Enable v-sync
    glfwSwapInterval(1);

    // Make the window visible
    glfwShowWindow(window);
    //glfwSetWindowSizeCallback(window, (long window, int w, int h) -> {
    //    glViewport(0, 0, w, h);
    //    perspective = new Matrix4f().perspective(20.0f, (float)w / h, 0.01f, 100.0f);
    //});

    //glfwSetKeyCallback(window, (long window, int key, int scancode, int action, int mods) -> {
    //    if (key < 512) {
    //        if (action == GLFW_PRESS)
    //            keys[key] = true;
    //        else if (action == GLFW_RELEASE)
    //            keys[key] = false;
    //
    //        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    //            if (GLFW_CURSOR_DISABLED == glfwGetInputMode(window, GLFW_CURSOR))
    //                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //            else
    //                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //        }
    //    }
    //});
    //
    //glfwSetCursorPosCallback(window, (long window, double xpos, double ypos) -> {
    //    float rotspeed = 0.002f;
    //    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
    //        Vector2f delta = new Vector2f((float)(px - xpos), (float)(py - ypos));
    //        yawpitch.add(delta.mul(rotspeed));
    //        if (yawpitch.x < 0.0f)
    //            yawpitch.x += 2.0f * (float)Math.PI;
    //        else if (yawpitch.x >= 2.0f * (float)Math.PI)
    //            yawpitch.x -= 2.0f * (float)Math.PI;
    //        yawpitch.y = Math.max((float)Math.PI * -0.49f, Math.min((float)Math.PI * 0.49f, yawpitch.y));
    //        px = xpos;
    //        py = ypos;
    //    }
    //});
    //
    //glfwSetCharCallback(window, (long window, int codepoint) -> {
    //    char[] ch = Character.toChars(codepoint);
    //    String str = new String(ch);
    //    //System.out.println(str);
    //});

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


}

*/

/*
* main.cpp - A test environment for development purposes.
* A distribution of the game engine will *not* include this file; rather,
* it will be a statically linked library for use in other projects.
*/

/*
int _main(int argc, char* argv[]) {

    // If we don't use the "new" keyword in C++, the class by default has the same
    // scope as a primitive type like "int".
    // There's no such thing as a null class in C++, but there are null pointers
    // (pointers to memory address 0), which is what Java uses under the hood.
    CppGameEngine engine;

    // Make a scene from scratch. In the future, we should be able to load in scenes
    // from asset files, likely either COLLADA or glTF (or some custom meta format).
    Scene scene(engine);
    
    // TODO
    //scene.setSkybox(Assets::importSkybox(engine, "path/to/skybox.png"));

    GameObjectID object = Assets::importObject(engine, "path/to/object.obj");
    scene.addObject(object);
    scene.addObject(Assets::importObject(engine, "path/to/object2.obj"));

    engine.setActiveScene(scene.getID());
    engine.launch(1280, 720, false);

    

	return 0;
}

*/