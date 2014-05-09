// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <vector>

#include "Mesh.h"
#include "Material.h"
#include "BoundingBox.h"

class Object {
public:
    inline Object () {smooth = false;}
    inline Object (const Mesh & mesh, const Material & mat) : mesh (mesh), mat (mat) {
        updateBoundingBox ();
        //We precompute the calcul of triangles normals

        mesh.computeTriangleNormals(trianglesNormals);
        reflectance=0;
        smooth = false;

    }
    virtual ~Object () {}

    inline const Vec3Df & getTrans () const { return trans;}
    inline void setTrans (const Vec3Df & t) { trans = t; }

    inline const Mesh & getMesh () const { return mesh; }
    inline Mesh & getMesh () { return mesh; }
    
    inline const Material & getMaterial () const { return mat; }
    inline Material & getMaterial () { return mat; }

    inline const BoundingBox & getBoundingBox () const { return bbox; }
    void updateBoundingBox ();

    inline const std::vector<Vec3Df> & getTrianglesNormals () const { return trianglesNormals; }

    inline float getRefl(){return reflectance;}
    inline void setRefl(float ref){reflectance=ref;}

    inline bool getSmooth(){return smooth;}
    inline void setSmooth(bool b){smooth = b;}

    void resize(float coef);
    
private:
    Mesh mesh;
    Material mat;
    BoundingBox bbox;
    Vec3Df trans;
    std::vector<Vec3Df> trianglesNormals;
    float reflectance;
    bool smooth; //define if brdf should interpolate the normals
};


#endif // Scene_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
