#include "Scene.h"

Scene::Scene():m_spiral(NULL) { }

Scene::~Scene()
{
    if (m_spiral != NULL)
    {
        delete m_spiral;
    }

//    delete Rod objects
    typedef std::vector<Rod*>::const_iterator Iter;
    for (Iter it = m_Rods.begin(); it != m_Rods.end(); ++it)
    {
        delete (*it);
    }

//    delete render objects
    typedef std::vector<RenderObject*>::const_iterator RIter;
    for (RIter it = m_renderObjects.begin(); it != m_renderObjects.end(); ++it)
    {
        delete (*it);
    }

//    delete meshes
    typedef std::vector<Mesh*>::const_iterator MIter;
    for (MIter it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        delete (*it);
    }
}

void Scene::initialize()
{
    for (unsigned i = 0; i < m_meshes.size(); ++i)
    {
        m_meshes[i]->setId(i);
    }

    for (unsigned i = 0; i < m_renderObjects.size(); ++i)
    {
        m_renderObjects[i]->setId(i);
    }

    for (unsigned i = 0; i < m_Rods.size(); ++i)
    {
        m_Rods[i]->setId(i);
    }
}


void Scene::update(mg::Real dt)
{
    typedef std::vector<Rod*>::iterator Iter;
    for (Iter it = m_Rods.begin(); it != m_Rods.end(); ++it)
    {
        (*it)->update(dt);
    }

//    m_Rods[0]->update(dt);
//    m_spiral->update(dt);
}
