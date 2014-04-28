// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "RayTracer.h"
#include "Ray.h"
#include "Scene.h"
#include <QProgressDialog>
#include "math.h"

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

// POINT D'ENTREE DU PROJET.
// Le code suivant ray trace uniquement la boite englobante de la scene.
// Il faut remplacer ce code par une veritable raytracer
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

    /*const BoundingBox & bbox = scene->getBoundingBox ();
    const Vec3Df & minBb = bbox.getMin ();
    const Vec3Df & maxBb = bbox.getMax ();
    const Vec3Df rangeBb = maxBb - minBb;*/

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
            Vec3Df intersectionPoint;
            float smallestIntersectionDistance = 1000000.f;
            Vec3Df c (backgroundColor);
            for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
                const Object & o = scene->getObjects()[k];
                Ray ray (camPos-o.getTrans (), dir);
                Mesh mesh = o.getMesh();

                bool hasIntersection = ray.intersect (o.getBoundingBox (), intersectionPoint);
                if (hasIntersection == true){
                    std::vector<Triangle> tabTriangle = mesh.getTriangles();
                    std::vector<Vertex> vertices = mesh.getVertices();
                    std::vector<Vec3Df> triangleNormals;
                    mesh.computeTriangleNormals(triangleNormals);

                    for(unsigned int m =0 ; m<tabTriangle.size() ; m++){
                        Triangle triangle = tabTriangle[m];
                        Vec3Df normal = triangleNormals[m];
                        Vec3Df vertex1 (vertices[triangle.getVertex(0)].getPos());
                        Vec3Df vertex2 (vertices[triangle.getVertex(1)].getPos());
                        Vec3Df vertex3 (vertices[triangle.getVertex(2)].getPos());
                        bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, intersectionPoint, normal);

                        if (hasIntersection) {
                            float intersectionDistance = Vec3Df::squaredDistance (intersectionPoint /*+ o.getTrans()*/, camPos-o.getTrans());
                            if (intersectionDistance < smallestIntersectionDistance) {
                                //c = 255.f * ((intersectionPoint - minBb) / rangeBb);
                                for(int l=0 ; l<3 ; l++){
                                    if (k==0){
                                        c[l] = 100;
                                    }
                                    else if (k==1){
                                        c[l] = 200;
                                    }
                                }
                                smallestIntersectionDistance = intersectionDistance;
                            }
                        }
                    }
                }
            }
            image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
        }
    }
    progressDialog.setValue (100);
    return image;
}
