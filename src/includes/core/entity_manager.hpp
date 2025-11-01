#pragma once
#include <cstdint>
#include <vector>

// Minimal ECS EntityId and EntityManager with stable IDs (index + generation)
struct EntityId
{
    uint32_t index{0xFFFFFFFFu};
    uint32_t generation{0u};

    static constexpr uint32_t invalid_index = 0xFFFFFFFFu;

    bool operator==(const EntityId &other) const
    {
        return index == other.index && generation == other.generation;
    }
    bool operator!=(const EntityId &other) const { return !(*this == other); }
    explicit operator bool() const { return index != invalid_index; }
};

class EntityManager
{
public:
    EntityManager() = default;

    // Optionally pre-allocate slots
    void reserve(std::size_t count)
    {
        if (count > generations.size())
        {
            generations.resize(count, 0u);
        }
    }

    // Create a new entity and return a stable id
    EntityId create()
    {
        uint32_t index;
        if (!freeIndices.empty())
        {
            index = freeIndices.back();
            freeIndices.pop_back();
            // generation already set; reuse current value
        }
        else
        {
            index = static_cast<uint32_t>(generations.size());
            generations.push_back(1u); // start generations at 1
        }
        return EntityId{index, generations[index]};
    }

    // Destroy an entity, invalidating its current generation and freeing the slot
    void destroy(EntityId id)
    {
        if (!isAlive(id))
            return;
        uint32_t &gen = generations[id.index];
        gen = nextGeneration(gen);
        freeIndices.push_back(id.index);
    }

    // Check if an id is still alive (matches current generation)
    bool isAlive(EntityId id) const
    {
        return id.index < generations.size() && generations[id.index] == id.generation;
    }

    // Number of allocated indices (not necessarily alive ones)
    std::size_t capacity() const { return generations.size(); }

    // Count of free slots available for reuse
    std::size_t freeCount() const { return freeIndices.size(); }

private:
    static uint32_t nextGeneration(uint32_t g)
    {
        // Avoid 0 as a valid generation to keep default {invalid,0} clearly dead
        ++g;
        if (g == 0u)
            ++g;
        return g;
    }

    std::vector<uint32_t> generations; // current generation per index
    std::vector<uint32_t> freeIndices; // freelist of reusable indices
};
