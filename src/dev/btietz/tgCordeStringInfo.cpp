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
 * @file tgLinearStringInfo.cpp
 * @brief Implementation of class tgLinearStringInfo
 * @author Ryan Adams
 * @date March 2014
 * $Id$
 */

#include "tgCordeStringInfo.h"

#include "core/tgWorld.h"
#include "core/tgBulletUtil.h"

#include "dev/Corde/CordeModel.h"
#include "dev/Corde/cordeCollisionObject.h"
#include "dev/Corde/cordeDynamicsWorld.h"

#include "tgCordeModel.h"

tgCordeStringInfo::tgCordeStringInfo(const CordeModel::Config& config) : 
m_config(config),
tgConnectorInfo()
{}

tgCordeStringInfo::tgCordeStringInfo(const CordeModel::Config& config, tgTags tags) : 
m_config(config),
tgConnectorInfo(tags)
{}

tgCordeStringInfo::tgCordeStringInfo(const CordeModel::Config& config, const tgPair& pair) :
m_config(config),
tgConnectorInfo(pair)
{}
    

tgConnectorInfo* tgCordeStringInfo::createConnectorInfo(const tgPair& pair)
{
    return new tgCordeStringInfo(m_config, pair);
}

void tgCordeStringInfo::initConnector(tgWorld& world)
{
    // Note: Muscle2P holds pointers to things in the world, but it doesn't actually have any in-world representation.
    m_cordeString = createCordeString(world);
    
    cordeDynamicsWorld& dynamicsWorld =
                tgBulletUtil::worldToCordeDynamicsWorld(world);
	
	dynamicsWorld.addSoftBody(m_cordeString);
}

tgModel* tgCordeStringInfo::createModel(tgWorld& world)
{
    // Don't have to do anything in the world for a Muscle2P...
    // @todo: set the name based on joined tags, or remove name from the model...
    //std::cout << "tgCordeStringInfo::createModel" << std::endl;
    
    // ensure connector has been initialized
    assert(m_cordeString);
    return new tgCordeModel(m_cordeString, getTags());
}

double tgCordeStringInfo::getMass() 
{
    // @todo: calculate a mass? Muscle2P doesn't have physics...
    return 0;
}


cordeCollisionObject* tgCordeStringInfo::createCordeString(tgWorld& world)
{
    //std::cout << "tgLinearStringInfo::createMuscle2P()" << std::endl;
    
    //std::cout << "  getFromRigidInfo(): " << getFromRigidInfo() << std::endl;
    //std::cout << "  getFromRigidInfo(): " << getFromRigidInfo()->getRigidInfoGroup() << std::endl;
    
    // @todo: need to check somewhere that the rigid bodies have been set...
    btRigidBody* fromBody = getFromRigidBody();
    btRigidBody* toBody = getToRigidBody();
	
	/// @todo restore rotation parameters
    btVector3 from = getFromRigidInfo()->getConnectionPoint(getFrom(), getTo(), 0.0);
    btVector3 to = getToRigidInfo()->getConnectionPoint(getTo(), getFrom(), 0.0);
	
	std::vector<btVector3> startPositions = generatePoints(from, to, m_config.resolution);
	
    return new cordeCollisionObject(startPositions, world, m_config);
}
    
std::vector<btVector3> tgCordeStringInfo::generatePoints(btVector3& point1, 
													btVector3& point2,
													std::size_t resolution)
{
	std::vector<btVector3> points;
	
	points.push_back(point1);
	
    btVector3 rodLength(point2 - point1);
    btVector3 unitLength( rodLength / ((double) resolution - 1) );
    
    btVector3 massPos(point1);
    
    for (std::size_t i = 1; i < resolution; i++)
    {
		massPos += unitLength;
        points.push_back(massPos);
	}
    
    assert(points.size() == resolution);

    return points;
}