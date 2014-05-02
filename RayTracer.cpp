// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "RayTracer.h"
#include <QProgressDialog>
#include <math.h>

static RayTracer * instance = NULL;

RayTracer * RayTracer::getInstance () {
    if (instance == NULL)
        instance = new RayTracer ();
    return instance;
}

void RayTracer::destroyInstance () {
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

inline int clamp (float f, int inf, int sup) {
    int v = static_cast<int> (f);
    return (v < inf ? inf : (v > sup ? sup : v));
}

inline std::vector<KDNode *> RayTracer::order(float & rayDir, KDNode *leftChild, KDNode *rightChild){
    std::vector<KDNode *> orderedVector;
    if (rayDir > 0.f) {
        orderedVector.push_back(leftChild);
        orderedVector.push_back(rightChild);
    }
    else {
        orderedVector.push_back(rightChild);
        orderedVector.push_back(leftChild);
    }
    return orderedVector;
}

inline bool RayTracer::searchNode (const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k){
    if (node->isLeaf())
    {
        return searchLeaf(node, ray, mesh, c, k);
    }
    else
    {
        return searchSplit(node, ray, mesh, c, k);
    }

}

inline bool RayTracer::searchSplit(const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k){
    Vec3Df intersectionPoint;
    bool isInFirst= ray.intersect(node->getLeftChild()->bbox, intersectionPoint);
    bool isInSecond = ray.intersect(node->getRightChild()->bbox, intersectionPoint);
    if (isInFirst && isInSecond) {
        std::vector<KDNode *> ordered = order( ray.getDirection()[node->getAxis()], node->getLeftChild(), node->getRightChild());
        bool inFirstNode = searchNode( ordered[0], ray, mesh, c, k);
        if (inFirstNode)
            return true;
        else
            return searchNode( ordered[1], ray, mesh, c, k);
    }
    else if( isInFirst ) {
        return searchNode( node->getLeftChild(), ray, mesh, c, k);
    }
    else if( isInSecond ) {
        return searchNode( node->getRightChild(), ray, mesh, c, k);
    }
    return false;
}

inline bool RayTracer::searchLeaf(const KDNode *node, Ray &ray, Mesh &mesh, Vec3Df &c, unsigned &k){
    const std::vector <Triangle> & meshTriangles = mesh.getTriangles();
    const std::vector <Vertex> vertices = mesh.getVertices();
    for (unsigned nbTri = 0; nbTri < node->getTriangles().size(); nbTri++) {
        unsigned idTri = node->getTriangles()[nbTri];
        const Triangle & triangle = meshTriangles[idTri];
        const Vec3Df & vertex1 = vertices [triangle.getVertex(0)].getPos();
        const Vec3Df & vertex2 = vertices [triangle.getVertex(1)].getPos();
        const Vec3Df & vertex3 = vertices [triangle.getVertex(2)].getPos();
        std::vector<Vec3Df> triangleNormals;
        mesh.computeTriangleNormals(triangleNormals);
        Vec3Df normal = triangleNormals[idTri];
        float intersectionDistance;
        bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, normal, intersectionDistance);

        if (hasIntersection) {
            for(int l=0 ; l<3 ; l++){
                if (k==0){
                    c[l] = 100;
                }
                else{
                    c[l] = 200;
                }
            }
            return true;
        }
    }
    return false;
}

QImage RayTracer::render (const Vec3Df & camPos,
                          const Vec3Df & direction,
                          const Vec3Df & upVector,
                          const Vec3Df & rightVector,
                          float fieldOfView,
                          float aspectRatio,
                          unsigned int screenWidth,
                          unsigned int screenHeight) {
    QImage image (QSize (screenWidth, screenHeight), QImage::Format_RGB888);
    Scene * scene = Scene::getInstance ();

    QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
    progressDialog.show ();
    for (unsigned int i = 0; i < screenWidth; i++) {
        progressDialog.setValue ((100*i)/screenWidth);
        for (unsigned int j = 0; j < screenHeight; j++) {
            float tanX = tan (fieldOfView)*aspectRatio;
            float tanY = tan (fieldOfView);
            Vec3Df stepX = (float (i) - screenWidth/2.f)/screenWidth * tanX * rightVector;
            Vec3Df stepY = (float (j) - screenHeight/2.f)/screenHeight * tanY * upVector;
            Vec3Df step = stepX + stepY;
            Vec3Df dir = direction + step;
            dir.normalize ();
            Vec3Df c (backgroundColor);
            for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
                const Object & o = scene->getObjects()[k];
                Ray ray (camPos-o.getTrans (), dir);
                Vec3Df intersectionPoint;
                bool hasIntersection = ray.intersect(o.getBoundingBox(), intersectionPoint);
                if (hasIntersection){
                    Mesh mesh = o.getMesh();
                    searchNode(o.getTree(), ray, mesh, c, k);
                }
            }
            image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
        }
    }
    progressDialog.setValue (100);
    return image;
}
