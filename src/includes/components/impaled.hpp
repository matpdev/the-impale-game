#pragma once
#include "box2d/box2d.h"

// Component marking entities that have been impaled/attached to spikes
struct Impaled
{
    bool frozen{false};
    b2JointId jointId{b2_nullJointId}; // Joint connecting box to spike (for chain/pendulum physics)
    bool hasJoint() const { return B2_IS_NON_NULL(jointId); }
};
