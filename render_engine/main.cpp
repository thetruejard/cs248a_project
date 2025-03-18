#if true

#include "assets/assets.h"
#include "components/motion.h"
#include "components/keyboardcontroller.h"
#include "components/mouserotation.h"
#include "core/renderengine.h"
#include "geometry/sphere.h"
#include "objects/go_camera.h"
#include "objects/go_mesh.h"
#include "utils/printutils.h"

#include "graphics/pipeline/rp_deferred_opengl.h"
#include "graphics/pipeline/rp_forward_opengl.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#define PI 3.141592653589f


RenderEngine engine;


class Moving : public Component {
public:
    glm::vec3 v;
    float phase;
    GameObject* go;
    
    Moving(GameObject* go) : Component(go) {
        this->go = go;
        auto random = []() { return float(rand()) / float(RAND_MAX); };
        this->phase = PI * random();
        constexpr float mag = 0.02f;
        this->v = mag * glm::normalize(2.0f * glm::vec3(random(), random(), random()) - 1.0f);
    }

    void evaluate(float deltaTime) override {
        this->go->deltaPosition(cos(this->phase) * this->v);
        constexpr float speed = 0.1f;
        this->phase += speed;
    }
};


class Pivoting : public Component {
public:
    glm::mat4 orig;
    float rot = 0.0;
    GameObject* go;

    Pivoting(GameObject* go) : Component(go) {
        this->go = go;
        orig = go->getLocalMatrix();
    }

    void evaluate(float deltaTime) override {
        this->go->setLocalMatrix(glm::rotate(this->orig, this->rot, glm::vec3(0.0f, 1.0f, 0.0f))); 
        this->rot += 2.0f*PI/300.0f;
    }
};


class ChangeRadius : public Component {
public:
    float phase = 0.0f;
    float init_rad;
    GO_Light* go;

    ChangeRadius(GameObject* go) : Component(go) {
        auto random = []() { return float(rand()) / float(RAND_MAX); };
        this->go = (GO_Light*)go;
        this->init_rad = this->go->radius;
    }

    void evaluate(float deltaTime) override {
        this->go->radius = this->init_rad * exp(sin(this->phase));
        this->phase += 4.0f * PI / 300.0f;
    }
};





void spawnLights(Scene* scene, size_t num_lights) {

    std::vector<GameObject*> lightSpawns;

    // Traverse for 2 purposes:
    // 1. Count the number of existing lights (i.e. remove from num_lights)
    // 2. Find the "LIGHT_SPAWN" objects
    std::function<void(Ref<GameObject>)> recurse = [&recurse, &num_lights, &lightSpawns](Ref<GameObject> root) {
        if (root->getTypeName() == "Light") {
            if (num_lights > 0)
                num_lights--;
        }
        if (root->getName().find("LIGHT_SPAWN") != std::string::npos) {
            lightSpawns.push_back(root.get());
        }
        for (auto child : root->getChildren()) {
            recurse(child);
        }
    };
    recurse(scene->getRoot());

    if (num_lights == 0 || lightSpawns.size() == 0) {
        return;
    }

    auto random = []() { return float(rand()) / float(RAND_MAX); };
    auto chooseLightPos = [&random, &lightSpawns]() {
        int spawnIdx = rand() % lightSpawns.size();
        float angle = 2.0f * 3.141592653589f * random();
        float radius = random();
        float height = 2.0f * random() - 1.0f;
        glm::vec4 normalizedCoords = glm::vec4(radius * cos(angle), radius * sin(angle), height, 1.0f);
        glm::vec4 outCoords = lightSpawns[spawnIdx]->getModelMatrix() * normalizedCoords;
        return glm::vec3(outCoords);
    };


    bool make_atten_sphere = false;
    Ref<Material> s_mat = engine.createMaterial();
    s_mat->assignDiffuseColor(glm::vec4(1.0f, 0.8f, 0.4f, 1.0f));
    Ref<Mesh> sphere = engine.createMesh();
    sphere->assignMaterial(s_mat);
    Sphere(0.02f).toMesh(sphere, 12, 8);
    sphere->uploadMesh();
    for (size_t i = 0; i < num_lights; i++) {
        Ref<GO_Light> light = engine.createObject<GO_Light>();
        Ref<GO_Mesh> mesh = engine.createObject<GO_Mesh>();
        mesh->assignMesh(sphere);
        mesh->setParent(light, false);
        glm::vec3 pos = chooseLightPos();
        light->setPosition(pos);
        glm::vec3 color = glm::normalize(glm::vec3(random(), random(), random()));
        light->color = 6.0f * color;
        light->addComponent<Moving>();
        scene->addObject(light);
        if (make_atten_sphere) {
            Sphere bs = light->getBoundingSphere();
            Ref<Mesh> bs_mesh = engine.createMesh();
            bs.toMesh(bs_mesh, 12, 8);
            bs_mesh->uploadMesh();
            Ref<Material> bs_mat = engine.createMaterial();
            bs_mat->assignDiffuseColor(glm::vec4(0.1f * color, 1.0f));
            bs_mat->wireframe = true;
            bs_mesh->assignMaterial(bs_mat);
            Ref<GO_Mesh> bs_obj = engine.createObject<GO_Mesh>();
            bs_obj->assignMesh(bs_mesh);
            bs_obj->setParent(light, true);     // Sphere is already at correct position, to adjust to fit
        }
    }

}


void setupDemoScene(std::string path, Scene* scene, size_t num_lights, std::string force_shadows, bool pivoting, bool changerad) {

    scene->backgroundColor = 0.1f * glm::vec3(0.5f, 0.6f, 1.0f); //1.3f * glm::vec3(0.5f, 0.6f, 1.0f);

    std::cout << "Loading scene\n"; 
    Ref<GameObject> object = Assets::importObject(engine, path);
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
    camera->setPosition(0.0f, 0.0f, 1.0f);
    float fov = 70.0f; // 70 standard, 22 for dolly vid
    camera->setPerspective(glm::radians(fov), 1920.0f / 1080.0f, 0.1f, 100.0f);
    camera->setParent(controller, false);
    scene->addObject(controller);
    scene->setActiveCamera(camera);


    std::cout << "Dimming the lights\n";
    // Dim the lights, since blender exports them super bright.
    // Also give random colors, just for fun.
    auto random = []() { return float(rand()) / float(RAND_MAX); };
    std::function<void(Ref<GameObject>)> dim_the_lights = [&dim_the_lights, &random,
        &force_shadows, &pivoting, &changerad](Ref<GameObject> root) {

        if (root->getName() == "PIVOT" && pivoting)
            root->addComponent<Pivoting>();
        if (root->getTypeName() == "Light") {
            constexpr float brightness = 0.002f; // 0.0001f
            auto L = root.cast<GO_Light>();
            L->color *= brightness;
            L->radius = 0.08f;
            if (L->type == GO_Light::Type::Directional) {
                std::string n = (force_shadows != "") ? force_shadows : L->getName();
                if (n.length() >= 5 && n.substr(n.length() - 5) == "_none")
                    L->shadowType = GO_Light::ShadowType::DisabledClip;
                else if (n.length() >= 6 && n.substr(n.length() - 6) == "_basic")
                    L->shadowType = GO_Light::ShadowType::Basic;
                else if (n.length() >= 4 && n.substr(n.length() - 4) == "_pcf")
                    L->shadowType = GO_Light::ShadowType::PCF;
                else if (n.length() >= 12 && n.substr(n.length() - 12) == "_filteredpcf")
                    L->shadowType = GO_Light::ShadowType::FilteredPCF;
                else if (n.length() >= 5 && n.substr(n.length() - 5) == "_pcss")
                    L->shadowType = GO_Light::ShadowType::PCSS;
                else if (n.length() >= 9 && n.substr(n.length() - 9) == "_raymarch") {
                    L->shadowType = GO_Light::ShadowType::RayMarching;
                    //L->radius *= 0.25f; // not ideal :/
                }
                else
                    L->shadowType = GO_Light::ShadowType::PCSS;

                if (changerad)
                    L->addComponent<ChangeRadius>();
            }
            std::cout << "LIGHT: " << root->getName() << " | ";
            Utils::Print::vec3(root.cast<GO_Light>()->color);
        }
        for (auto child : root->getChildren()) {
            dim_the_lights(child);
        }
    };
    dim_the_lights(object);

    //spawnLights(scene, num_lights);


    std::cout << "Scene graph:\n";
    Utils::Print::objectTree(scene->getRoot().get());

}

void argsError() {
    std::cout << "Args error\n";
    exit(1);
}

int main(int argc, char* argv[]) {

    RenderPipelineType pipeline = RenderPipelineType::Forward;
    std::string pipeline_name = "forward-none";
    glm::ivec3 numTiles = glm::ivec3(48, 27, 24);
    GLint maxLightsPerTile = 128;
    size_t num_lights = 50;
    bool pivoting = false;
    bool changerad = false;
    std::string force_shadows = "";
    std::string scene_path = "./samples/shadows/demo1/demo1.gltf";
    std::filesystem::path log_file;
    std::filesystem::path render_dir;
    std::filesystem::path campose_file;
    bool interactive = true;

    srand(1);

    // Parse arguments
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--lights") {
            if (i == args.size()-1)
                argsError();
            num_lights = std::stoi(args[++i]);
        }
        else if (args[i] == "--pipeline") {
            if (++i == args.size())
                argsError();
            if (args[i] == "none") {
                pipeline = RenderPipelineType::None;
            }
            else if (args[i] == "clay") {
                pipeline = RenderPipelineType::Clay;
            }
            else if (args[i] == "deferred-none" ||
                args[i] == "deferred-boundingsphere" ||
                args[i] == "deferred-rastersphere" ||
                args[i] == "deferred-tiled-cpu" ||
                args[i] == "deferred-clustered-cpu" ||
                args[i] == "deferred-tiled-gpu" ||
                args[i] == "deferred-clustered-gpu") {
                pipeline = RenderPipelineType::Deferred;
            }
            else if (args[i] == "forward-none" ||
                args[i] == "forward-boundingsphere" ||
                args[i] == "forward-tiled-cpu" ||
                args[i] == "forward-clustered-cpu" ||
                args[i] == "forward-tiled-gpu" ||
                args[i] == "forward-clustered-gpu") {
                pipeline = RenderPipelineType::Forward;
            }
            else argsError();
            pipeline_name = args[i];
        }
        else if (args[i] == "--numTiles") {
            if (i + 2 >= args.size())
                argsError();
            numTiles.x = (GLint)std::stoi(args[++i]);
            numTiles.y = (GLint)std::stoi(args[++i]);
        }
        else if (args[i] == "--numClustersZ") {
            if (++i == args.size())
                argsError();
            numTiles.z = (GLint)std::stoi(args[i]);
        }
        else if (args[i] == "--maxLightsPerTile") {
            if (++i == args.size())
                argsError();
            maxLightsPerTile = (GLint)std::stoi(args[i]);
        }
        else if (args[i] == "--shadows") {
            if (++i == args.size())
                argsError();
            force_shadows = "_" + args[i];
        }
        else if (args[i] == "--scene") {
            if (++i == args.size())
                argsError();
            scene_path = args[i];
        }
        else if (args[i] == "--log-file") {
            if (++i == args.size())
                argsError();
            log_file = args[i];
        }
        else if (args[i] == "--render-dir") {
            if (++i == args.size())
                argsError();
            render_dir = args[i];
        }
        else if (args[i] == "--campose-file") {
            if (++i == args.size())
                argsError();
            campose_file = args[i];
        }
        else if (args[i] == "--eval") {
            interactive = false;
        }
        else if (args[i] == "--interactive" || args[i] == "-I") {
            interactive = true;
        }
        else if (args[i] == "--pivoting") {
            pivoting = true;
        }
        else if (args[i] == "--changerad") {
            changerad = true;
        }
        else {
            std::cout << "Unknown argument: " << args[i] << "\n";
            argsError();
        }
    }

    engine.getGraphics()->setRenderPipeline(pipeline);
    RenderPipeline* gpipeline = engine.getGraphics()->getRenderPipeline();
    if (pipeline_name == "deferred-none")
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::None;
    else if (pipeline_name == "deferred-boundingsphere")
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::BoundingSphere;
    else if (pipeline_name == "deferred-rastersphere")
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::RasterSphere;
    else if (pipeline_name == "deferred-tiled-cpu") {
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::TiledCPU;
        numTiles.z = 1;     // IMPORTANT.
        ((RP_Deferred_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Deferred_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "deferred-clustered-cpu") {
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::ClusteredCPU;
        ((RP_Deferred_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Deferred_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "deferred-tiled-gpu") {
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::TiledGPU;
        numTiles.z = 1;     // IMPORTANT.
        ((RP_Deferred_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Deferred_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "deferred-clustered-gpu") {
        ((RP_Deferred_OpenGL*)gpipeline)->culling = RP_Deferred_OpenGL::LightCulling::ClusteredGPU;
        ((RP_Deferred_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Deferred_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "forward-none")
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::None;
    else if (pipeline_name == "forward-boundingsphere")
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::BoundingSphere;
    else if (pipeline_name == "forward-tiled-cpu") {
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::TiledCPU;
        numTiles.z = 1;     // IMPORTANT.
        ((RP_Forward_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Forward_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "forward-clustered-cpu") {
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::ClusteredCPU;
        ((RP_Forward_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Forward_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "forward-tiled-gpu") {
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::TiledGPU;
        numTiles.z = 1;     // IMPORTANT.
        ((RP_Forward_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Forward_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }
    else if (pipeline_name == "forward-clustered-gpu") {
        ((RP_Forward_OpenGL*)gpipeline)->culling = RP_Forward_OpenGL::LightCulling::ClusteredGPU;
        ((RP_Forward_OpenGL*)gpipeline)->numTiles = numTiles;
        ((RP_Forward_OpenGL*)gpipeline)->maxLightsPerTile = maxLightsPerTile;
    }

    std::cout << "lights: " << num_lights << "\n";
    std::cout << "pipeline: " << pipeline_name << "\n";

     
    Ref<Scene> scene = engine.createScene();
    setupDemoScene(scene_path, scene.get(), num_lights, force_shadows, pivoting, changerad);
    engine.setActiveScene(scene);

    if (interactive) {
        engine.launch("Interactive", 1920, 1080, false);
    }
    else {
        // Load camera path
        std::ifstream campath_file(campose_file);
        json campath = json::parse(campath_file);
        std::vector<float> cam_mats;
        size_t num_cam_mats = campath.size();
        cam_mats.reserve(16 * campath.size());
        for (int i = 0; i < campath.size(); i++) {
            auto& m = campath[i];
            if (m.size() != 16) {
                std::cout << "TRAJECTORY MATRIX SIZE NOT 16\n";
                exit(1);
            }
            cam_mats.insert(cam_mats.end(), m.begin(), m.end());
        }

        json result = engine.launch_eval("Eval", 1920, 1080, false, (glm::mat4*)cam_mats.data(), num_cam_mats, !log_file.empty(), render_dir);

        if (!log_file.empty()) {
            std::ofstream log_out(log_file);
            log_out << result.dump();
        }
    }

    
    return 0;
}


#endif
