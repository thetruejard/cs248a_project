#if false

#include "assets/assets.h"
#include "components/motion.h"
#include "components/keyboardcontroller.h"
#include "components/mouserotation.h"
#include "core/renderengine.h"
#include "objects/go_camera.h"
#include "utils/printutils.h"

#define PI 3.14159265358979f


#include <iostream>


// PROTOTYPE.

RenderEngine engine;


void setupDemoScene(Scene* scene) {

    std::cout << "Loading object\n";
    GameObject* object = Assets::importObject(engine, "./samples/assets/house/house.gltf");
    if (!object) {
        std::cout << "Failed to load object\n";
        exit(0);
    }
    std::cout << "Done loading object\n";
    scene->addObject(object);

    std::cout << "Loading object2\n";
    GameObject* object2 = Assets::importObject(engine, "./samples/assets/train/Train Dr14.gltf");
    if (!object2) {
        std::cout << "Failed to load object2\n";
        exit(0);
    }
    std::cout << "Done loading object2\n";
    scene->addObject(object2);

    // Make object2 rotate (yaw) at 1 full rotation every 8 seconds.
    Motion* motion = object2->addComponent<Motion>();
    motion->setAngularVelocity(0.25f * PI, 0.0f, 0.0f);
    object2->addComponent<ApplyMotion>(motion);
    object2->setPosition(0.0f, 0.0f, 1.0f);


    // Prepare the camera & controls.
    GameObject* controller = engine.createObject<GameObject>();
    controller->addComponent<Motion>();
    controller->addComponent<KeyboardController>();
    controller->addComponent<MouseRotation>();
    controller->addComponent<ApplyMotion>();

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
    Utils::Print::objectTree(scene->getRoot());

}


int main(int argc, char* argv[]) {

    Scene* scene = engine.createScene();
    setupDemoScene(scene);
    engine.setActiveScene(scene);

    engine.launch("Sample 5", 1280, 720, false);

    return 0;
}


#endif
