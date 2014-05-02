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

inline bool RayTracer::searchNode (const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> triangleNormals, Vec3Df &c, unsigned &k, float smallestIntersectionDistance){
    if (node->isLeaf())
    {
        return searchLeaf(node, ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
    }
    else
    {
        return searchSplit(node, ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
    }

}

inline bool RayTracer::searchSplit(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> triangleNormals, Vec3Df &c, unsigned &k, float smallestIntersectionDistance){
    Vec3Df intersectionPoint;
    bool isInFirst= ray.intersect(node->getLeftChild()->bbox, intersectionPoint);
    bool isInSecond = ray.intersect(node->getRightChild()->bbox, intersectionPoint);
    if (isInFirst && isInSecond) {
        std::vector<KDNode *> ordered = order( ray.getDirection()[node->getAxis()], node->getLeftChild(), node->getRightChild());
        bool inFirstNode = searchNode( ordered[0], ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
        if (inFirstNode)
            return true;
        else
            return searchNode( ordered[1], ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
    }
    else if( isInFirst ) {
        return searchNode( node->getLeftChild(), ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
    }
    else if( isInSecond ) {
        return searchNode( node->getRightChild(), ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
    }
    return false;
}

inline bool RayTracer::searchLeaf(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> triangleNormals, Vec3Df &c, unsigned &k, float smallestIntersectionDistance){
    std::vector<unsigned> triangleList = node->getTriangles();

    for (std::vector<unsigned>::iterator idTri = triangleList.begin() ; idTri != triangleList.end(); ++idTri){
        Triangle triangle = meshTriangles[*idTri];
        Vec3Df vertex1 = vertices [triangle.getVertex(0)].getPos();
        Vec3Df vertex2 = vertices [triangle.getVertex(1)].getPos();
        Vec3Df vertex3 = vertices [triangle.getVertex(2)].getPos();
        Vec3Df normal = triangleNormals[*idTri];
        float intersectionDistance;
        bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, normal, intersectionDistance);

        if (hasIntersection) {
            if (intersectionDistance<smallestIntersectionDistance)
            {
                for(int l=0 ; l<3 ; l++){
                    if (k==0){
                        c[l] = 100;
                    }
                    else{
                        c[l] = 200;
                    }
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
            float smallestIntersectionDistance = 10e8;
            dir.normalize ();
            Vec3Df c (backgroundColor);
            for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
                const Object & o = scene->getObjects()[k];
                Ray ray (camPos-o.getTrans (), dir);
                Vec3Df intersectionPoint;
                bool hasIntersection = ray.intersect(o.getBoundingBox(), intersectionPoint);
                if (hasIntersection){
                    Mesh mesh = o.getMesh();
                    std::vector <Triangle> meshTriangles = mesh.getTriangles();
                    std::vector <Vertex> vertices = mesh.getVertices();
                    std::vector<Vec3Df> triangleNormals;
                    mesh.computeTriangleNormals(triangleNormals);

                    searchNode(o.getTree(), ray, meshTriangles, vertices, triangleNormals, c, k, smallestIntersectionDistance);
                }
            }
            image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
        }
    }
    progressDialog.setValue (100);
    return image;
}
