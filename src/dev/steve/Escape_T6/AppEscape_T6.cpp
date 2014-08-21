/*
 * Copyright © 2012, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 * 
 * The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
 * under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 */

/**
 * @file AppEscape_T6.cpp
 * @brief Contains the definition function main() for the Escape T6
 * application.
 * $Id$
 */

// This application
#include "Escape_T6Model.h"
#include "Escape_T6Controller.h"

// This library
#include "core/terrain/tgHillyGround.h"
#include "core/tgModel.h"
#include "core/tgSimViewGraphics.h"
#include "core/tgSimulation.h"
#include "core/tgWorld.h"

// Bullet Physics
#include "LinearMath/btVector3.h"

// The C++ Standard Library
#include <iostream>

tgHillyGround *createGround();
tgWorld *createWorld();
tgSimViewGraphics *createGraphicsView(tgWorld *world);
tgSimView *createView(tgWorld *world);
void simulate(tgSimulation *simulation);

/**
 * Runs a series of episodes. 
 * Each episode tests a given control pattern for a given number of steps.
 * The fitness function (reward metric) for this experiment is 
 *     the maximum distance from the tensegrity's starting point 
 *     at any point during the episode
 * NB: Running episodes and using graphics are mutually exclusive features
 */
int main(int argc, char** argv)
{
    std::cout << "AppEscape_T6" << std::endl;

    // First create the world
    tgWorld *world = createWorld();

    // Second create the view
    //tgSimViewGraphics *view = createGraphicsView(world); // For visual experimenting on one tensegrity
    tgSimView       *view = createView(world);         // For running multiple episodes

    // Third create the simulation
    tgSimulation *simulation = new tgSimulation(*view);

    // Fourth create the models with their controllers and add the models to the simulation
    Escape_T6Model* const model = new Escape_T6Model();

    // Fifth create controller and attach it to the model
    double initialLength = 9; // decimeters
    Escape_T6Controller* const controller = new Escape_T6Controller(initialLength);
    model->attach(controller);

    //Sixth add model (with controller) to simulation
    simulation->addModel(model);

    simulate(simulation);

    //Teardown is handled by delete, so that should be automatic
    return 0;
}

tgHillyGround *createGround() {
    // Determine the angle of the ground in radians. All 0 is flat
    const double yaw = 0.0;
    const double pitch = 0.0;
    const double roll = 0.0;
    const tgHillyGround::Config groundConfig(btVector3(yaw, pitch, roll));
    // the world will delete this
    return new tgHillyGround(groundConfig);
}

tgWorld *createWorld() {
    const tgWorld::Config config(98.1); // gravity, cm/sec^2  Use this to adjust length scale of world.
    // NB: by changing the setting below from 981 to 98.1, we've
    // scaled the world length scale to decimeters not cm.

    tgHillyGround* ground = createGround();
    return new tgWorld(config, ground);
}

/** Use for displaying tensegrities in simulation */
tgSimViewGraphics *createGraphicsView(tgWorld *world) {
    const double timestep_physics = 1.0 / 60.0 / 10.0; // Seconds
    const double timestep_graphics = 1.f /60.f; // Seconds, AKA render rate
    return new tgSimViewGraphics(*world, timestep_physics, timestep_graphics); 
}

/** Use for trial episodes of many tensegrities in an experiment */
tgSimView *createView(tgWorld *world) {
    const double timestep_physics = 1.0 / 60.0 / 10.0; // Seconds
    const double timestep_graphics = 1.f /60.f; // Seconds, AKA render rate
    return new tgSimView(*world, timestep_physics, timestep_graphics); 
}

/** Run a series of episodes for nSteps each */
void simulate(tgSimulation *simulation) {
    int nEpisodes = 10; // Number of episodes ("trial runs")
    int nSteps = 60000; // Number of steps in each episode
    for (int i=0; i<nEpisodes; i++)
    {   
        simulation->run(nSteps);
        simulation->reset();
    }
}

