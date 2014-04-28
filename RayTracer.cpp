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

Vec3Df Brdf(const Vec3Df & direction,
                           const Vec3Df & normal,
                           const Object object,
                           const Vec3Df & intersectionPoint){

    Scene * scene = Scene::getInstance ();
    std::vector<Light> lights = scene->getLights();
    Light light = lights[0];
    Vec3Df n = normal;
    n.normalize();
    Vec3Df wi = light.getPos() - intersectionPoint;
    wi.normalize();
    Vec3Df w0 = - direction;
    w0.normalize();
    Vec3Df r = 2*(Vec3Df::dotProduct(wi,n))*n-wi;
    float diffuse = Vec3Df::dotProduct(wi, n);
    float shininess = 2;
    float spec = pow(Vec3Df::dotProduct(r,w0),shininess);
    spec = std::max(spec,0.f);
    diffuse = std::max(diffuse,0.f);
    Vec3Df lightColor = light.getColor();
    Material material = object.getMaterial();
    float matDiffuse = material.getDiffuse();
    float matSpecular = material.getSpecular();
    Vec3Df matDiffuseColor = material.getColor();
    Vec3Df matSpecularColor = material.getColor();
    return ((matDiffuse * diffuse * matDiffuseColor + matSpecular * spec * matSpecularColor) * lightColor)*255;

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

    //We precompute the calcul of triangles normals
    std::vector<Vec3Df> triangleNormalsByObject[scene->getObjects().size ()];
    for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
        const Object & o = scene->getObjects()[k];
        Mesh mesh = o.getMesh();
        mesh.computeTriangleNormals(triangleNormalsByObject[k]);
    }
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
                //std::cout << o.getTrans()
                Ray ray (camPos-o.getTrans (), dir);
                bool hasIntersection = ray.intersect (o.getBoundingBox (), intersectionPoint);
                if (hasIntersection){
                    Mesh mesh = o.getMesh();
                    std::vector<Triangle> tabTriangle = mesh.getTriangles();
                    std::vector<Vertex> vertices = mesh.getVertices();
                    for(unsigned int m =0 ; m<tabTriangle.size() ; m++){
                        Triangle triangle = tabTriangle[m];
                        Vec3Df normal = triangleNormalsByObject[k][m]; //Pecomputed
                        Vec3Df vertex1 (vertices[triangle.getVertex(0)].getPos());
                        Vec3Df vertex2 (vertices[triangle.getVertex(1)].getPos());
                        Vec3Df vertex3 (vertices[triangle.getVertex(2)].getPos());
                        float intersectionDistance;
                        float coefB[3]; //The three barycentric coefs of the intersection point
                        bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, normal, coefB, intersectionDistance);

                        if (hasIntersection) {
                            if (intersectionDistance < smallestIntersectionDistance) {

                                Vec3Df IntersPointNormal =
                                        vertices[triangle.getVertex(0)].getNormal()*coefB[2]
                                        +vertices[triangle.getVertex(1)].getNormal()*coefB[0]
                                        +vertices[triangle.getVertex(2)].getNormal()*coefB[1];
                                IntersPointNormal = IntersPointNormal/(coefB[0]+coefB[1]+coefB[2]);

                                Vec3Df IntersPoint =
                                        vertices[triangle.getVertex(0)].getPos()*coefB[2]
                                        +vertices[triangle.getVertex(1)].getPos()*coefB[0]
                                        +vertices[triangle.getVertex(2)].getPos()*coefB[1];
                                IntersPoint = IntersPoint/(coefB[0]+coefB[1]+coefB[2]);


                                c = Brdf(dir, IntersPointNormal, o,intersectionPoint);
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

