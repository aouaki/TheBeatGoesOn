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
#include "Object.h"

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
    inline bool searchNode (const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]);
    inline bool searchSplit(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]);
    inline bool searchLeaf(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]);

    QImage render (const Vec3Df & camPos,
                   const Vec3Df & viewDirection,
                   const Vec3Df & upVector,
                   const Vec3Df & rightVector,
                   float fieldOfView,
                   float aspectRatio,
                   unsigned int screenWidth,
                   unsigned int screenHeight);

    int getIntersectionPoint(const Vec3Df & camPos, const Vec3Df & dir, Vec3Df & intersectionPoint, Vec3Df & IntersPointNormal, float &occlusion);
     int getIntersectionPoint(const Vec3Df & camPos, const Vec3Df & dir, Vec3Df & intersectionPoint, Vec3Df & IntersPointNormal);

    Vec3Df Brdf(const Vec3Df & camPos,const Vec3Df & normal,int idObj,const Vec3Df & intersectionPoint, float occlusion);
    inline void setNbRayShadow(int nbrs){nbRayShadow =nbrs;}
    inline void setNbRayAO(int nbrs){nbRayAO =nbrs;}
    inline bool getActMir(){return activeMirror;}
    inline void changeActMir(bool b){activeMirror=b;}
    inline void setActShadow(bool b){activeShadow =b;}
    inline bool getActShadow(){return activeShadow;}
    inline void setActAO(bool b){activeAO =b;}
    inline bool getActAO(){return activeAO;}
    inline void setActPreAO(bool b){activePreAO =b;}
    inline bool getActPreAO(){return activePreAO;}
    inline void setActAA(bool b){activeAA =b;}
    inline bool getActAA(){return activeAA;}

protected:
    inline RayTracer () {}
    inline virtual ~RayTracer () {}
    
private:
    Vec3Df backgroundColor;
    int nbRayShadow;
    bool activeMirror;
    bool activeShadow;
    bool activePreAO;
    bool activeAO;
    bool activeAA;
    int nbRayAO;
};


#endif // RAYTRACER_H

// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
