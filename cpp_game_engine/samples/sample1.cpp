#if false

#include "core/CppGameEngine.h"


// PROTOTYPE; NOT FUNCTIONAL


class SampleScene : public Scene {
private:

    virtual void init() override {

        // Can either use Assets::SkyBox or Assets::Equireq (equirectangular map).
        this->setBackground(Assets::Equireq("assets/equireq.png"));

        // Add some objects to the scene.
        for (const char* f : {
            "landscape.obj",
            "building1.obj",
            "building2.obj",
            "car.obj",
        }) {
            this->addObject(Assets::Import("assets/" + std::string(f)));
        }

        // Add a whole collection of objects to the scene.
        this->addObject(Assets::Import("assets/subscene.dae"));

        // Add a player to the scene.
        this->addPlayer(PlayerMode::FirstPerson);

        // Set the player spawnpoint. List => choose from the list of spawnpoints.
        this->setSpawn(SpawnMode::List, {
            // Arguments are (position, probability).
            // If multiple spawnpoints are given, the probability is divided
            // by the sum of all probabilities. Must be nonnegative.
            SpawnPoint(Vec3(0.0f, 0.0f, 0.0f), 1.0f),
        });

    }

    virtual void onStart() override {}


}



int main1(int argc, char* argv[]) {


    CppGameEngine engine("Sample 1");

    Ref<Scene> scene = MakeRef<SampleScene>();
    engine.setScene(scene);

    engine.launch();

    return 0;
}


#endif
