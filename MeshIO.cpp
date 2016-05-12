#include "MeshIO.h"
#include "Instance.h"
#include "Material.h"
#include "Texture.h"
#include <fstream>

bool MeshIO::readGeometry(const std::string& path,
                          std::vector<Instance>& instances)
{
    std::ifstream in(path.c_str());
    
    if (in.is_open()) {
        int i = - 1;
        int mIndex = -1;
        int lod = -1;
        
        std::string line;
        while (getline(in, line)) {
            std::stringstream ss(line);
            std::string token;
            
            ss >> token;
            if (token == "i") {
                i++;
                lod = 0;
                instances.push_back(Instance());
                
            } else if (token == "cl") {
                ss >> instances[i].mesh.closed;
                
            } else if (token == "lods") {
                int lods;
                ss >> lods;
                instances[i].mesh.lods.reserve(lods);
                
            } else if (token == "lod") {
                ss >> lod;
                if (lod > 0) instances[i].mesh.lods.push_back(Mesh());
                
            } else if (token == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                
                Mesh& mesh(lod == 0 ? instances[i].mesh : instances[i].mesh.lods[lod-1]);
                mesh.vertices.push_back(Vertex(Eigen::Vector3f(x, y, z), (int)mesh.vertices.size()));
                
            } else if (token == "vt") {
                float u, v;
                ss >> u >> v;
                
                Mesh& mesh(lod == 0 ? instances[i].mesh : instances[i].mesh.lods[lod-1]);
                mesh.uvs.push_back(Eigen::Vector2f(u, v));
                
            } else if (token == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                
                Mesh& mesh(lod == 0 ? instances[i].mesh : instances[i].mesh.lods[lod-1]);
                mesh.normals.push_back(Eigen::Vector3f(x, y, z));
                
            } else if (token == "m") {
                ss >> mIndex;
                
            } else if (token == "f") {
                Mesh& mesh(lod == 0 ? instances[i].mesh : instances[i].mesh.lods[lod-1]);
                int f = (int)mesh.faces.size();
                mesh.faces.push_back(Face());
                
                int x, y, z;
                ss >> x >> y >> z;
                
                Eigen::Vector3i indices(x, y, z);
                mesh.faces[f].vIndices = indices;
                mesh.faces[f].mIndex = mIndex;
                mesh.faces[f].index = f;
                
                if (std::find(mesh.mIndices.begin(), mesh.mIndices.end(), mIndex) == mesh.mIndices.end()) {
                    mesh.mIndices.push_back(mIndex);
                }
                
            } else if (token == "t") {
                Eigen::Matrix4f transform;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        ss >> transform(i, j);
                    }
                }
                
                transform.row(0) *= -1;
                transform.row(1).swap(transform.row(2));
                instances[i].transforms.push_back(transform);
            }
        }
        
        in.close();
    
    } else {
        std::cerr << "Error: Could not open geometry file for reading" << std::endl;
        return false;
    }
    
    return true;
}

bool MeshIO::readMaterials(const std::string& path,
                           std::vector<Material>& materials,
                           std::vector<Texture>& textures)
{
    std::ifstream in((path + "m").c_str());
    
    if (in.is_open()) {
        int i = - 1;
        std::string line;
        while (getline(in, line)) {
            std::stringstream ss(line);
            std::string token;
            std::string texturePath = path.substr(0, path.find_last_of("/")+1) + "textures/";
            
            ss >> token;
            if (token == "n") {
                materials.push_back(Material());
                i++;
                
            } else if (token == "d") {
                ss >> materials[i].color.x() >> materials[i].color.y() >> materials[i].color.z();
                materials[i].color /= 255.0;
                
                Texture texture;
                std::string str;
                while (ss >> str) texture.name.append(str + " ");
                texture.name = texture.name.substr(0, texture.name.length()-1);
                
                if (!texture.name.empty()) {
                    int t = (int)textures.size();
                    textures.push_back(texture);
                    materials[i].tIndex = t;
                    
                    Image& image(textures[t].image);
                    image.read(texturePath + texture.name);
                    if (image.matte()) image.magick("RGBA");
                    else image.magick("RGB");
                    image.flip();
                }
                
            } else if (token == "a") {
                ss >> materials[i].alpha;
                materials[i].alpha /= 255.0;
            }
        }
        
        in.close();
        
    } else {
        std::cerr << "Error: Could not open material file for reading" << std::endl;
        return false;
    }
    
    return true;
}
