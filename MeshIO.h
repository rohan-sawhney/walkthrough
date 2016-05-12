#ifndef MESH_IO_H
#define MESH_IO_H

#include "Types.h"

class MeshIO {
public:
    // read geometry
    static bool readGeometry(const std::string& path,
                             std::vector<Instance>& instances);
    
    // read materials
    static bool readMaterials(const std::string& path,
                              std::vector<Material>& materials,
                              std::vector<Texture>& textures);
};

#endif
