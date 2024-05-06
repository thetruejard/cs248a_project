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

#define PI 3.14159265358979f


#include <iostream>


// PROTOTYPE.

RenderEngine engine;


void setupDemoScene(Scene* scene) {

    scene->backgroundColor = glm::vec3(0.5f, 0.6f, 1.0f);

    std::cout << "Loading scene\n";
    Ref<GameObject> object = Assets::importObject(engine, "./samples/assets/collision_test/collision_test.glb");
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
    std::function<void(Ref<GameObject>)> dim_the_lights = [&dim_the_lights](Ref<GameObject> root) {
        if (root->getTypeName() == "Light") {
            root.cast<GO_Light>()->color *= 0.001f;
        }
        for (auto child : root->getChildren()) {
            dim_the_lights(child);
        }
    };
    dim_the_lights(object);


    // Add a procedurally generated sphere.
    Ref<GO_Mesh> sphere = engine.createObject<GO_Mesh>();
    Ref<Mesh> sphereMesh = engine.createMesh();
    sphere->assignMesh(sphereMesh);
    sphere->setName("TEST_SPHERE");
    scene->addObject(sphere);
    Sphere s(3.0f, glm::vec3(0.0f, 5.0f, 0.0f));
    if (!s.toMesh(sphereMesh, 16, 8)) {
        std::cout << "Failed to build sphere\n";
    }
    sphereMesh->uploadMesh();

    Ref<Material> sphereMat = engine.createMaterial();
    sphereMat->assignDiffuseColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
    sphereMat->wireframe = true;
    sphereMesh->assignMaterial(sphereMat);



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
