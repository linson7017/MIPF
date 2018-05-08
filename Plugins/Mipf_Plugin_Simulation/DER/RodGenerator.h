#ifndef RodGENERATOR_H
#define RodGENERATOR_H

#include "Rod.h"

class RodGenerator
{
public:
    RodGenerator();
    ~RodGenerator();

    static void generateCurlyRod(const RenderObject* object, const std::vector<unsigned>& findices, Rod& o_Rod);
    static void generateStraightRod(const RenderObject* object, const std::vector<unsigned>& findices, Rod& o_Rod);

    static void generateHelicalRod(const RodParams& params,
                            const mg::Vec3D& p, const mg::Vec3D& n, const mg::Vec3D& u,
                            ElasticRod& o_rod);
    static void generateStraightRod(const RodParams& params,
                             const mg::Vec3D& p, const mg::Vec3D& n, const mg::Vec3D& u,
                             ElasticRod& o_rod);
};

#endif // RodGENERATOR_H
