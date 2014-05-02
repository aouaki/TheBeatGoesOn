// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <iostream>
#include <vector>
#include <QImage>

#include "Vec3D.h"
#include "Ray.h"
#include "Scene.h"

class RayTracer {
public:
    static RayTracer * getInstance ();
    static void destroyInstance ();

    inline const Vec3Df & getBackgroundColor () const { return backgroundColor;}
    inline void setBackgroundColor (const Vec3Df & c) { backgroundColor = c; }

    inline std::vector<KDNode *> order(float & rayDir, KDNode *leftChild, KDNode *rightChild);
    inline bool searchNode (const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k);
    inline bool searchSplit(const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k);
    inline bool searchLeaf(const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k);
    inline void continueSearch(Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k);

    QImage render (const Vec3Df & camPos,
                   const Vec3Df & viewDirection,
                   const Vec3Df & upVector,
                   const Vec3Df & rightVector,
                   float fieldOfView,
                   float aspectRatio,
                   unsigned int screenWidth,
                   unsigned int screenHeight);


protected:
    inline RayTracer () {}
    inline virtual ~RayTracer () {}
    
private:
    Vec3Df backgroundColor;
};


#endif // RAYTRACER_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
