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
};
