#pragma once

// Physics material properties for rigidbody behavior
struct PhysicsMaterial
{
    float density{1.0f};         // mass per unit area
    float friction{0.3f};        // surface friction (0-1+)
    float restitution{0.2f};     // bounciness (0 = no bounce, 1 = perfect bounce)
    float linearDamping{0.0f};   // resistance to linear motion
    float angularDamping{0.05f}; // resistance to rotation
    bool affectedByGravity{true};
};
