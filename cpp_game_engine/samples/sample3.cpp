#if false

#include "core/cppgameengine.h"
#include "assets/assets.h"
#include "objects/go_camera.h"
#include "objects/go_controller.h"

#include <iostream>


// PROTOTYPE.

CppGameEngine engine;


void setupDemoScene(Scene* scene) {

    GameObject* pyramid = Assets::importObject(engine, "samples/assets/pyramid.gltf");
    scene->addObject(pyramid);

    GameObject* monkey = Assets::importObject(engine, "samples/assets/monkey.gltf");
    scene->addObject(monkey);

    pyramid->setPosition(-2.0f, 0.0f, -6.0f);
    monkey->setPosition(2.0f, 0.0f, -6.0f);


    // Prepare the camera & controls.
    GO_Controller* controller = engine.createObject<GO_Controller>();
    GO_Camera* camera = engine.createObject<GO_Camera>();
    camera->setPerspective(glm::radians(50.0f), 1024.0f / 512.0f, 0.1f, 100.0f);
    camera->setParent(controller, false);
    scene->addObject(controller);
    scene->setActiveCamera(camera);


    std::cout << "Scene graph:\n";
    Utils::printObjectTree(scene->getRoot());
}


int main(int argc, char* argv[]) {

    Scene* scene = engine.createScene();
    setupDemoScene(scene);
    engine.setActiveScene(scene);

    engine.launch("Sample 3", 1024, 512, false);

    return 0;
}


#endif
