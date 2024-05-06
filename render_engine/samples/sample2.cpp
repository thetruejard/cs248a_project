#if false

#include "core/renderengine.h"
#include "assets/Assets.h"
#include "objects/GO_Camera.h"
#include "objects/GO_Controller.h"

#include <iostream>


// PROTOTYPE.

RenderEngine engine;


void setupDemoScene(Scene scene) {

    GameObjectID pyramid_id = Assets::importObject(engine, "samples/assets/pyramid.gltf");
    scene.addObject(pyramid_id);

    GameObjectID monkey_id = Assets::importObject(engine, "samples/assets/monkey.gltf");
    scene.addObject(monkey_id);

    GameObject* pyramid = engine.getObject(pyramid_id);
    pyramid->setPosition(-2.0f, 0.0f, -6.0f);
    GameObject* monkey = engine.getObject(monkey_id);
    monkey->setPosition(2.0f, 0.0f, -6.0f);


    // Prepare the camera & controls.
    GameObjectID controller_id = engine.createObject<GO_Controller>();
    GameObjectID camera_id = engine.createObject<GO_Camera>();
    GO_Controller* controller = (GO_Controller*)engine.getObject(controller_id);
    GO_Camera* camera = (GO_Camera*)engine.getObject(camera_id);
    camera->setPerspective(glm::radians(50.0f), 1024.0f / 512.0f, 0.1f, 100.0f);
    scene.addObject(controller_id);
    scene.setActiveCamera(camera);
    camera->setParent(controller, false);

    // Hard-coded for this sample only: GameObjectID 1 is the root object of the scene.
    std::cout << "Scene graph:\n";
    Utils::printObjectTree(engine.getObject(1));
}


int main2(int argc, char* argv[]) {

    Scene scene = engine.createScene();
    setupDemoScene(scene);
    engine.setActiveScene(scene.getID());

    engine.launch("Sample 2", 1024, 512, false);

    return 0;
}


#endif
