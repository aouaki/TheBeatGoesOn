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
#include <cstdlib>

#include <omp.h>

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

Vec3Df RayTracer::Brdf(const Vec3Df & camPos,
                       const Vec3Df & normal,
                       int idObj,
                       const Vec3Df & intersectionPoint){

    Scene * scene = Scene::getInstance ();
    std::vector<Light> lights = scene->getLights();
    Object & object = scene->getObjects()[idObj];
    Vec3Df ci;
    for(unsigned int i =0;i<lights.size();i++)
    {
        Light light = lights[i];
        Vec3Df n = normal;
        n.normalize();
        Vec3Df wi = light.getPos() - intersectionPoint;
        wi.normalize();
        Vec3Df w0 = (camPos-intersectionPoint);
        w0.normalize();
        Vec3Df r = 2*(Vec3Df::dotProduct(wi,n))*n-wi;
        r.normalize();
        float diffuse = Vec3Df::dotProduct(wi, n);
        float shininess = 11;
        float spec = pow(std::max(Vec3Df::dotProduct(r,w0),0.f),shininess);
        diffuse = std::max(diffuse,0.0f);
        Vec3Df lightColor = light.getColor();
        Material material = object.getMaterial();
        float matDiffuse = material.getDiffuse();
        float matSpecular = material.getSpecular();
        Vec3Df matDiffuseColor = material.getColor();
        Vec3Df matSpecularColor = material.getColor();

        Vec3Df intersectionPoint2;
        Vec3Df IntersPointNormal2;

        //Area Lighting
        float radius=0.1f;

        //Miroir
        if(scene->getObjects()[idObj].getRefl()>0 && activeMirror)
        {
            Vec3Df vDir= intersectionPoint-camPos;
            vDir.normalize();
            Vec3Df planA = Vec3Df::crossProduct(n, Vec3Df::crossProduct(vDir, n));
            Vec3Df newDir = Vec3Df::dotProduct(planA, vDir)*planA - Vec3Df::dotProduct(vDir, n)*n;
            int obj = getIntersectionPoint(intersectionPoint, newDir, intersectionPoint2, IntersPointNormal2);
            if(obj>-1)
            {
                if(getIntersectionPoint(intersectionPoint,-intersectionPoint+light.getPos(),intersectionPoint2,IntersPointNormal2)==-1)
                {
                    ci += Brdf(intersectionPoint,IntersPointNormal2,obj,intersectionPoint2)*scene->getObjects()[idObj].getRefl();
                }
            }
        }

        if(scene->getObjects()[idObj].getRefl()<1.0 || true)
        {
            if(nbRayShadow>0)
                for(int p = 0;p<nbRayShadow;p++)
                {
                    float a = ((float)std::rand())/((float)RAND_MAX);
                    float b = ((float)std::rand())/((float)RAND_MAX);
                    float c = ((float)std::rand())/((float)RAND_MAX);

                    float sum = a+b+c;
                    a=a/sum*radius;
                    b=b/sum*radius;
                    c=c/sum*radius;
                    Vec3Df lightposbis;

                    lightposbis[0]=light.getPos()[0]+a;
                    lightposbis[1]=light.getPos()[1]+b;
                    lightposbis[2]=light.getPos()[2]+c;

                    if(getIntersectionPoint(intersectionPoint,-intersectionPoint+lightposbis,intersectionPoint2,IntersPointNormal2)==-1)
                    {
                        ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255/nbRayShadow;
                    }
                }
            else
                if(getIntersectionPoint(intersectionPoint,-intersectionPoint+light.getPos(),intersectionPoint2,IntersPointNormal2)==-1 || true)
                {
                    ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255;
                }

            //ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255/nbrayshadow;
        }



    }
    return ci;
}

//Function giving the good intersection point (if intersection) or return 0 if not
//The function links the adress intersectionPoint, IntersectionPointNormal, obj to the actual intersection point, its normal
//and the object it belongs to.

int RayTracer::getIntersectionPoint(const Vec3Df & camPos,
                                    const Vec3Df & dir,
                                    Vec3Df & intersectionPoint,
                                    Vec3Df & IntersPointNormal)
{
    Scene * scene = Scene::getInstance ();
    float smallestIntersectionDistance = 1000000.f;

    int idObj = -1;
    bool hasIntersection=false;
    for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
        Object & o = scene->getObjects()[k];
        //std::cout << o.getTrans()
        Ray ray (camPos-o.getTrans (), dir);
        hasIntersection = ray.intersect (o.getBoundingBox ());
        if (hasIntersection){
            Mesh mesh = o.getMesh();
            std::vector<Triangle> tabTriangle = mesh.getTriangles();
            std::vector<Vertex> vertices = mesh.getVertices();
            for(unsigned int m =0 ; m<tabTriangle.size() ; m++){
                Triangle triangle = tabTriangle[m];
                Vec3Df normal = o.getTrianglesNormals()[m]; //Pecomputed
                Vec3Df vertex1 (vertices[triangle.getVertex(0)].getPos());
                Vec3Df vertex2 (vertices[triangle.getVertex(1)].getPos());
                Vec3Df vertex3 (vertices[triangle.getVertex(2)].getPos());
                float intersectionDistance;
                float coefB[3]; //The three barycentric coefs of the intersection point
                bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, normal, coefB, intersectionDistance);

                if (hasIntersection) {

                    if (intersectionDistance < smallestIntersectionDistance) {

                        IntersPointNormal =
                                vertices[triangle.getVertex(0)].getNormal()*coefB[2]
                                +vertices[triangle.getVertex(1)].getNormal()*coefB[0]
                                +vertices[triangle.getVertex(2)].getNormal()*coefB[1];
                        IntersPointNormal = IntersPointNormal/(coefB[0]+coefB[1]+coefB[2]);

                        intersectionPoint =
                                vertices[triangle.getVertex(0)].getPos()*coefB[2]
                                +vertices[triangle.getVertex(1)].getPos()*coefB[0]
                                +vertices[triangle.getVertex(2)].getPos()*coefB[1];
                        intersectionPoint = intersectionPoint/(coefB[0]+coefB[1]+coefB[2]);

                        smallestIntersectionDistance = intersectionDistance;

                        idObj=k;


                    }
                }
            }
        }
    }

    return idObj;
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
    std::vector<Light> lights = scene->getLights();
    Light light = lights[0];

    std::cout << "Couleur de la lumiere : " << light.getColor() << std::endl;

    QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
    progressDialog.show ();

    //#pragma omp parallel for
    for (unsigned int i = 0; i < screenWidth; i++) {
        progressDialog.setValue ((100*i)/screenWidth);
        for (unsigned int j = 0; j < screenHeight; j++) {


            float tanX = tan (fieldOfView)*aspectRatio;
            float tanY = tan (fieldOfView);

            //Nombre de découpe par dimension du pixel (Antialiasing)
            int aliaNb = 1;
            aliaNb++;

            Vec3Df c (backgroundColor);
            Vec3Df tempc(backgroundColor);
            for(int pixi=1; pixi<aliaNb; pixi++){
                for(int pixj=1; pixj<aliaNb; pixj++){
                    Vec3Df stepX = (float (i)-0.5+float(pixi)/float(aliaNb) - screenWidth/2.f)/screenWidth * tanX * rightVector;
                    Vec3Df stepY = (float (j)-0.5+float(pixj)/float(aliaNb) - screenHeight/2.f)/screenHeight * tanY * upVector;
                    Vec3Df step = stepX + stepY;

                    Vec3Df dir = direction + step;
                    dir.normalize ();
                    Vec3Df intersectionPoint;
                    Vec3Df IntersPointNormal;

                    int idObj = getIntersectionPoint(camPos,dir,intersectionPoint,IntersPointNormal);

                    if(idObj>=0)
                    {

                        tempc += Brdf(camPos, IntersPointNormal, idObj,intersectionPoint)/std::pow(aliaNb-1,2);
                        c=tempc;

                    }
                }
            }

            image.setPixel (i, j, qRgb (clamp (c[0], 0, 255), clamp (c[1], 0, 255), clamp (c[2], 0, 255)));
        }
    }
    progressDialog.setValue (100);
    return image;
}

