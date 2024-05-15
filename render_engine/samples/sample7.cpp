#if false

#include "assets/assets.h"
#include "components/motion.h"
#include "components/keyboardcontroller.h"
#include "components/mouserotation.h"
#include "core/renderengine.h"
#include "geometry/sphere.h"
#include "objects/go_camera.h"
#include "objects/go_mesh.h"
#include "utils/printutils.h"


#include <iostream>


// PROTOTYPE.

RenderEngine engine;


void setupDemoScene(Scene* scene) {

    scene->backgroundColor = 0.1f * glm::vec3(0.5f, 0.6f, 1.0f);

    std::cout << "Loading scene\n";
    Ref<GameObject> object = Assets::importObject(engine, "./samples/assets/house/house.gltf");
    if (!object) {
        std::cout << "Failed to load scene\n";
        exit(0);
    }
    std::cout << "Done loading scene\n";
    scene->addObject(object);


    // Prepare the camera & controls.
    Ref<GameObject> controller = engine.createObject<GameObject>();
    controller->addComponent<Motion>();
    controller->addComponent<KeyboardController>();
    controller->addComponent<MouseRotation>();
    controller->addComponent<ApplyMotion>();

    // Make the camera appear third-person-ish by moving it backwards relative to the controller.
    Ref<GO_Camera> camera = engine.createObject<GO_Camera>();
    camera->setPosition(0.0f, 0.0f, 3.0f);
    camera->setPerspective(glm::radians(70.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    camera->setParent(controller, false);
    scene->addObject(controller);
    scene->setActiveCamera(camera);


    // Dim the lights, since blender exports them super bright.
    // Also give random colors, just for fun.
    auto random = []() { return float(rand()) / float(RAND_MAX); };
    std::function<void(Ref<GameObject>)> dim_the_lights = [&dim_the_lights, &random](Ref<GameObject> root) {
        if (root->getTypeName() == "Light") {
            constexpr float brightness = 0.0002f;
            glm::vec3 color = glm::normalize(glm::vec3(random(), random(), random()));
            root.cast<GO_Light>()->color *= brightness * color;
        }
        for (auto child : root->getChildren()) {
            dim_the_lights(child);
        }
    };
    dim_the_lights(object);


    std::cout << "Scene graph:\n";
    Utils::Print::objectTree(scene->getRoot().get());

}


int main(int argc, char* argv[]) {

    Ref<Scene> scene = engine.createScene();
    setupDemoScene(scene.get());
    engine.setActiveScene(scene);

    engine.launch("Sample 6", 1280, 720, false);

    return 0;
}


#endif
