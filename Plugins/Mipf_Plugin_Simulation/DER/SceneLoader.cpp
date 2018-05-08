#include "SceneLoader.h"
#include <map>
#include "RodGenerator.h"
#include "Rod.h"


struct mesh_object
{
    mesh_object():m_id(-1) { }
    long int m_id;
    std::string m_filename;
};

struct object_3D
{
    object_3D():m_id(-1), m_meshId(-1) { m_transform.identity(); }
    long int m_id;
    long int m_meshId;
    mg::Matrix4D m_transform;
    std::vector<mg::Matrix4D> m_collisionShapes;
};

struct Rod_object
{
    Rod_object():m_id(-1), m_objId(-1), m_faceCnt(0) { }
    long int m_id;
    long int m_objId;
    unsigned m_faceCnt;
    std::vector<unsigned> m_faceList;

    std::string m_type;
    double m_length;
    double m_lengthVariance;
    double m_helicalRadius;
    double m_helicalPitch;
    double m_density;
    double m_thickness;
    long int m_nParticles;

    mg::Vec3D m_netForce;
    double m_drag;

    long int m_resolveCollisions;
    long int m_resolveSelfInterations;
    double m_selfInterationDist;
    double m_selfStiction;
    double m_selfRepulsion;

    long int m_pbdIter;
    double m_bendStiffness;
    double m_twistStiffness;
    double m_maxElasticForce;

    ElasticRodParams::MINIMIZATION_STRATEGY m_minimizationStrategy;
    double m_minimizationTolerance;
    long int m_minimizationMaxIter;
};


struct scene_object
{
    std::map<unsigned, mesh_object> m_meshMap;
    std::map<unsigned, object_3D> m_object3DMap;
    std::map<unsigned, Rod_object> m_RodMap;
};




struct SceneLoader::PImpl
{
public:
    inline std::string &stripComment(std::string& s);
    bool parseScene(std::ifstream& ifs, scene_object& o_scene);
    bool parseMesh(std::ifstream& ifs, scene_object& o_scene);
    bool parseObject3D(std::ifstream& ifs, scene_object& o_scene);
    bool parseRodObject(std::ifstream& ifs, scene_object& o_scene);

    unsigned m_lineNumber;
};




SceneLoader::SceneLoader()
{
    m_impl = new PImpl();
}

SceneLoader::~SceneLoader()
{
    delete m_impl;
}

Scene* SceneLoader::loadTestScene()
{
    Scene* scene = new Scene();

    mg::Real size = 30;
    scene->m_boundingVolume.reshape(mg::Vec3D(-size, -size, -size), mg::Vec3D(size, size, size));

//    create mesh
    unsigned meshId = scene->m_meshes.size();
    Mesh* mesh = Mesh::createSphere(meshId, 20, 10);
    scene->m_meshes.push_back(mesh);
//    create render object
    mg::Matrix4D transform;
    mg::matrix_uniform_scale(transform, (mg::Real)1);
    mg::matrix_set_translation(transform, (mg::Real)0, (mg::Real)0, (mg::Real)0);

    RenderObject* object = new RenderObject(mesh, transform);
    scene->m_renderObjects.push_back(object);


    //    create mesh2
    unsigned meshId2 = scene->m_meshes.size();
    Mesh* mesh2 = Mesh::createSphere(meshId2, 20, 10);
    scene->m_meshes.push_back(mesh2);
    //    create render object2
    mg::Matrix4D transform2;
    mg::matrix_uniform_scale(transform2, (mg::Real)1);
    mg::matrix_set_translation(transform2, (mg::Real)0, (mg::Real)2.0, (mg::Real)0);
    RenderObject* object2 = new RenderObject(mesh2, transform2);
    scene->m_renderObjects.push_back(object2);


    std::vector<unsigned> fidx(10);
    for (unsigned i = 0; i < fidx.size(); ++i)
    {
        fidx[i] = 2 * i;
    }
//    create Rod object
    Rod* rod = new Rod();
    scene->m_Rods.push_back(rod);
//    generate Rod
  //  rod->m_params->m_length = 200;
   // rod->m_params->m_rodParams->setBendStiffness(0.5);
    RodGenerator::generateStraightRod(object, fidx, *rod);

//    add collision shape for the object
    mg::Real radius = object2->getMeshBoundingRadius() + rod->m_params->m_thickness;
    mg::Matrix4D shape;
    mg::matrix_scale(shape, radius, radius, radius);
    mg::matrix_set_translation(shape, object2->getMeshAABB().getCenter());
    object2->addCollisionShape(shape);

    scene->m_spiral = new Spiral();
    scene->m_spiral->init(object);

    return scene;
}



void generateStraightRod(const GuideWireParams& params,
    const mg::Vec3D& p, const mg::Vec3D& n, const mg::Vec3D& u,
    ElasticRod& o_rod)
{
    o_rod.m_ppos.resize(params.m_nParticles);
    o_rod.m_pvel.resize(params.m_nParticles);
    o_rod.m_pmass.resize(params.m_nParticles);
    o_rod.m_theta.set_size(params.m_nParticles - 1);
    o_rod.m_isClamped.clear();

    mg::Real length = params.m_length;
    mg::Real volume = length * params.m_thickness * params.m_thickness * mg::Constants::pi();
    mg::Real pmass = params.m_density * volume / (params.m_nParticles - 1);

    //    generate straight line
    mg::Vec3D end = p + length * mg::normalize(n);
    mg::Real t = 0.0;
    for (unsigned i = 0; i < o_rod.m_ppos.size(); ++i)
    {
        t = (mg::Real)(i) / (params.m_nParticles - 1);
        o_rod.m_ppos[i] = (1 - t) * p + t * end;
        //        init velocity and mass
        o_rod.m_pvel[i].zero();
        o_rod.m_pmass[i] = pmass;
        if (i < (unsigned)o_rod.m_theta.size())
        {
            o_rod.m_theta(i) = 0;
        }
    }
    o_rod.m_isClamped.insert(0);
    //    o_rod.m_isClamped.insert(params->m_nParticles - 1);

    mg::Vec3D e0 = o_rod.m_ppos[1] - o_rod.m_ppos[0];
    o_rod.m_u0 = mg::cross(e0, u);
    o_rod.m_u0 = mg::normalize(mg::cross(o_rod.m_u0, e0));

    o_rod.init(o_rod.m_ppos, o_rod.m_u0, o_rod.m_ppos, o_rod.m_pvel, o_rod.m_pmass, o_rod.m_theta, o_rod.m_isClamped);
}

void CreateGuideWire(GuideWire& wire,const mg::Vec3D& entrancePoint,const mg::Vec3D& headingDirection)
{
    ElasticRod* rod = new ElasticRod(wire.m_params->m_rodParams);
    wire.m_strand = rod;
    generateStraightRod(*wire.m_params, entrancePoint, -headingDirection, -mg::EY, *rod);
    wire.initialize();
}



GuideWire* SceneLoader::createGuideWireScene(const mg::Vec3D& entrancePoint, const mg::Vec3D& headingDirection,const std::vector<vtkSmartPointer<vtkPolyData>>& objects, const std::vector<vtkSmartPointer<vtkImageData>>& imgs)
{
    GuideWire* wireScene = new GuideWire();

    wireScene->m_params->m_length = 70;
    wireScene->m_params->m_nParticles = 70;


    wireScene->m_objects = objects;
    wireScene->m_imgs = imgs;

    mg::Vec3D hd = headingDirection;
  //  wireScene->m_params->m_movingVel = hd.normalize()*0.5;
    CreateGuideWire(*wireScene, entrancePoint, headingDirection);

    /* mg::Real radius = object->getMeshBoundingRadius() + wireScene->m_params->m_thickness;
     mg::Matrix4D shape;
     mg::matrix_scale(shape, radius, radius, radius);
     mg::matrix_set_translation(shape, object->getMeshAABB().getCenter());
     object->addCollisionShape(shape);*/

    return wireScene;

}


inline std::string &SceneLoader::PImpl::stripComment(std::string& s)
{
    std::size_t found = s.find('#');
    if (found == std::string::npos)
        return s;
    return s.erase(found, s.size());
}

