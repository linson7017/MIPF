#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "Rod.h"
#include "Spiral.h"
#include "GuideWire.h"

class SceneLoader;

class Scene
{
public:
    Scene();
    ~Scene();

    void initialize();
    void update(mg::Real dt);

    inline const AABB& getBoundingVolume() const { return m_boundingVolume; }
    inline const std::vector<RenderObject*>& getRenderObjects() const { return m_renderObjects; }
    inline const std::vector<Mesh*>& getMeshes() const { return m_meshes; }

    inline Rod* getRodById(unsigned id) const { return ((id < m_Rods.size())? m_Rods[id] : NULL); }
    inline const std::vector<ElasticRod*> getStrands() const { return m_spiral->m_strands; }
private:
    AABB m_boundingVolume;

    Spiral* m_spiral;
    std::vector<Rod*> m_Rods;
    std::vector<RenderObject*> m_renderObjects;
    std::vector<Mesh*> m_meshes;

private:

    friend class SceneLoader;
};

#endif // SCENE_H
