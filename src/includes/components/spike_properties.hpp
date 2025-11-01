#pragma once

// Spike type variants
enum class SpikeType
{
    NORMAL, // standard spike hazard
    CHAIN,  // hanging chain/hook spike
    SAW     // rotating saw blade
};

// Spike-specific properties
struct SpikeProperties
{
    SpikeType type{SpikeType::NORMAL};
    float rotationSpeed{0.0f};   // for SAW type (degrees per second)
    float chainLength{0.0f};     // for CHAIN type (pixels)
    float currentRotation{0.0f}; // current rotation angle

    // Chain tuning (for CHAIN type)
    float linkLengthPx{20.0f};    // rectangular link length in pixels
    float linkThicknessPx{6.0f};  // rectangular link thickness in pixels
    float linkDensity{1.0f};      // density for link bodies
    float linkFriction{0.6f};     // friction for chain links and hook
    float linkRestitution{0.0f};  // restitution (bounciness) for chain links and hook
    float hookScaleW{1.2f};       // hook width scale relative to link thickness
    float hookScaleH{1.6f};       // hook height scale relative to link length
    float jointHertz{3.0f};       // spring freq for attach (distance joint)
    float jointDamping{0.5f};     // damping ratio for attach (distance joint)
    bool chainSelfCollide{false}; // whether links collide with each other and spike
};
