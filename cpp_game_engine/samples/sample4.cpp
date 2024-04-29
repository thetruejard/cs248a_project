#if false

#include "core/cppgameengine.h"
#include "assets/assets.h"
#include "objects/go_camera.h"
#include "objects/go_controller.h"

#include <iostream>


// PROTOTYPE.

CppGameEngine engine;


void setupDemoScene(Scene* scene) {

    std::cout << "Loading object\n";
    GameObject* object = Assets::importObject(engine, "./samples/assets/house/house.gltf");
    if (!object) {
        std::cout << "Failed to load object\n";
        exit(0);
    }
    std::cout << "Done loading object\n";
    scene->addObject(object);

    object->setPosition(0.0f, 0.0f, -6.0f);


    // Prepare the camera & controls.
    GO_Controller* controller = engine.createObject<GO_Controller>();
    GO_Camera* camera = engine.createObject<GO_Camera>();
    camera->setPerspective(glm::radians(70.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    camera->setParent(controller, false);
    scene->addObject(controller);
    scene->setActiveCamera(camera);


    // Dim the lights, since blender exports them super bright.
    std::function<void(GameObject*)> dim_the_lights = [&dim_the_lights](GameObject* root) {
        if (root->getTypeName() == "Light") {
            ((GO_Light*)root)->color *= 0.0005f;
        }
        for (auto child : root->getChildren()) {
            dim_the_lights(child);
        }
    };
    dim_the_lights(object);
    

    std::cout << "Scene graph:\n";
    Utils::printObjectTree(scene->getRoot());

}


int main(int argc, char* argv[]) {

    Scene* scene = engine.createScene();
    setupDemoScene(scene);
    engine.setActiveScene(scene);

    engine.launch("Sample 4", 1280, 720, false);

    return 0;
}


#endif
