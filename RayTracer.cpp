// *********************************************************
// Ray Tracer Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2012 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "RayTracer.h"
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
                       const Vec3Df & intersectionPoint,
                       float occlusion,
                       int PTRays){

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
        float radius=0.3f;

        //Miroir
        if(scene->getObjects()[idObj].getRefl()>0 && activeMirror)
        {
            Vec3Df vDir= intersectionPoint-camPos;
            vDir.normalize();
            Vec3Df planA = Vec3Df::crossProduct(n, Vec3Df::crossProduct(vDir, n));
            Vec3Df newDir = Vec3Df::dotProduct(planA, vDir)*planA - Vec3Df::dotProduct(vDir, n)*n;
            float occ;
            int obj = getIntersectionPoint(intersectionPoint, newDir, intersectionPoint2, IntersPointNormal2, occ);
            if(obj>-1)
            {

                if(activeShadow)
                {
                    if(getIntersectionPoint(intersectionPoint,-intersectionPoint+light.getPos(),intersectionPoint2,IntersPointNormal2)==-1)
                    {
                        ci += Brdf(intersectionPoint,IntersPointNormal2,obj,intersectionPoint2,occ,0)*scene->getObjects()[idObj].getRefl();
                    }
                }
                else
                    ci += Brdf(intersectionPoint,IntersPointNormal2,obj,intersectionPoint2,occ,0)*scene->getObjects()[idObj].getRefl();
            }
        }

        //PathTracing
        if(activePT)
        {
            if(PTRays < depthPT)
            {
               for(int h=0; h< nbRayPT; h++)
                {
                    Vec3Df n1;
                    Vec3Df n2;

                    normal.getTwoOrthogonals(n1,n2);


                    float a = ((float)std::rand())/((float)RAND_MAX);
                    float b = ((float)std::rand())/((float)RAND_MAX)*2.-1.;
                    float c = ((float)std::rand())/((float)RAND_MAX)*2.-1.;

                    Vec3Df dir = normal*a+n1*b+n2*c;
                    dir.normalize();

                    int objPT = getIntersectionPoint(intersectionPoint,dir,intersectionPoint2,IntersPointNormal2);

                    ci+=Brdf(intersectionPoint,IntersPointNormal2,objPT,intersectionPoint2,0.,PTRays+1)/nbRayPT;
                }


            }



            //si pt<pt_max

            //lancer plein de rayons

            //contribution+=brdf(pt+1)
        }

        if(scene->getObjects()[idObj].getRefl()<1.0 || true)
        {
            if(nbRayShadow>0 && activeShadow)
                for(int p = 0;p<nbRayShadow;p++)
                {
                    float a = ((float)std::rand())/((float)RAND_MAX)*2.-1.;
                    float b = ((float)std::rand())/((float)RAND_MAX)*2.-1.;
                    float c = ((float)std::rand())/((float)RAND_MAX)*2.-1.;

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
            else if(activeShadow)
            {
                if(getIntersectionPoint(intersectionPoint,-intersectionPoint+light.getPos(),intersectionPoint2,IntersPointNormal2)==-1)
                {
                    ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255;
                }
            }
            else //sans ombre
            {
                ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255;
            }
            //ci += (((matDiffuse * diffuse * matDiffuseColor) +( matSpecular * spec * matSpecularColor*0.5))*lightColor)*255/nbrayshadow;
        }



    }

    if(activeAO) return ci*(1.f-occlusion);
    else return ci;
}

//Function giving the good intersection point (if intersection) or return 0 if not
//The function links the adress intersectionPoint, IntersectionPointNormal, obj to the actual intersection point, its normal
//and the object it belongs to.
int RayTracer::getIntersectionPoint(const Vec3Df & camPos,
                                    const Vec3Df & dir,
                                    Vec3Df & intersectionPoint,
                                    Vec3Df & IntersPointNormal)
{
    float adress;

    return getIntersectionPoint(camPos,
                                dir,
                                intersectionPoint,
                                IntersPointNormal,
                                adress);

}

int RayTracer::getIntersectionPoint(const Vec3Df & camPos,
                                    const Vec3Df & dir,
                                    Vec3Df & intersectionPoint,
                                    Vec3Df & IntersPointNormal,
                                    float & occlusion)
{
    Scene * scene = Scene::getInstance ();
    float smallestIntersectionDistance = 1000000.f;

    int idObj = -1;
    bool hasIntersection=false;
    for (unsigned int k = 0; k < scene->getObjects().size (); k++) {
        Object & o = scene->getObjects()[k];
        Ray ray (camPos-o.getTrans (), dir);
        hasIntersection = ray.intersect (o.getBoundingBox ());
        if (hasIntersection){
            Mesh mesh = o.getMesh();
            std::vector<Triangle> tabTriangle = mesh.getTriangles();
            std::vector<Vertex> vertices = mesh.getVertices();
            std::vector<Vec3Df> normals = o.getTrianglesNormals(); //Precomputed
            Vec3Df normal = normals[k];
            float intersectionDistance;
            float coefB[3]; //The three barycentric coefs of the intersection point
            unsigned idTriangle;
            bool hasIntersection = searchNode (o.getTree(), ray, tabTriangle, vertices, normals, intersectionDistance, idTriangle, coefB);

            if (hasIntersection) {

                if (intersectionDistance < smallestIntersectionDistance) {

                    Triangle triangle = tabTriangle[idTriangle];

                        if(o.getSmooth()) IntersPointNormal=normal;
                        else
                        {
                            IntersPointNormal =
                                    vertices[triangle.getVertex(0)].getNormal()*coefB[2]
                                    +vertices[triangle.getVertex(1)].getNormal()*coefB[0]
                                    +vertices[triangle.getVertex(2)].getNormal()*coefB[1];
                            IntersPointNormal = IntersPointNormal/(coefB[0]+coefB[1]+coefB[2]);
                        }


                        occlusion =
                                o.getMesh().getVertices()[triangle.getVertex(0)].getOcc()*coefB[2]
                                +o.getMesh().getVertices()[triangle.getVertex(1)].getOcc()*coefB[0]
                                +o.getMesh().getVertices()[triangle.getVertex(2)].getOcc()*coefB[1];
                        occlusion = occlusion/(coefB[0]+coefB[1]+coefB[2]);

                        intersectionPoint =
                                vertices[triangle.getVertex(0)].getPos()*coefB[2]
                                +vertices[triangle.getVertex(1)].getPos()*coefB[0]
                                +vertices[triangle.getVertex(2)].getPos()*coefB[1];
                        intersectionPoint = (intersectionPoint+o.getTrans())/(coefB[0]+coefB[1]+coefB[2]);

                    smallestIntersectionDistance = intersectionDistance;

                    idObj=k;


                }
            }
        }
    }

    return idObj;
}

// POINT D'ENTREE DU PROJET.
// Le code suivant ray trace uniquement la boite englobante de la scene.
// Il faut remplacer ce code par une veritable raytracer

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

inline bool RayTracer::searchNode (const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]){
    if (node->isLeaf())
    {
        return searchLeaf(node, ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
    }
    else
    {
        return searchSplit(node, ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
    }

}

inline bool RayTracer::searchSplit(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]){
    bool isInFirst= ray.intersect(node->getLeftChild()->bbox);
    bool isInSecond = ray.intersect(node->getRightChild()->bbox);
    if (isInFirst && isInSecond) {
        std::vector<KDNode *> ordered = order( ray.getDirection()[node->getAxis()], node->getLeftChild(), node->getRightChild());
        bool inFirstNode = searchNode( ordered[0], ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
        if (inFirstNode)
            return true;
        else
            return searchNode( ordered[1], ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
    }
    else if( isInFirst ) {
        return searchNode( node->getLeftChild(), ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
    }
    else if( isInSecond ) {
        return searchNode( node->getRightChild(), ray, meshTriangles, vertices, triangleNormals, intersectionDistance, idTriangle, coefB);
    }
    return false;
}

inline bool RayTracer::searchLeaf(const KDNode *node, Ray &ray, std::vector <Triangle> &meshTriangles, std::vector <Vertex> &vertices, std::vector<Vec3Df> &triangleNormals, float &intersectionDistance, unsigned &idTriangle, float coefB[]){
    std::vector<unsigned> triangleList = node->getTriangles();

    float smallest = 10e8;
    bool found = false;
    for (std::vector<unsigned>::iterator idTri = triangleList.begin() ; idTri != triangleList.end(); ++idTri){
        Triangle triangle = meshTriangles[*idTri];
        Vec3Df vertex1 = vertices [triangle.getVertex(0)].getPos();
        Vec3Df vertex2 = vertices [triangle.getVertex(1)].getPos();
        Vec3Df vertex3 = vertices [triangle.getVertex(2)].getPos();
        Vec3Df normal = triangleNormals[*idTri];
        float coefBTemp[3];
        bool hasIntersection = ray.intersectTriangle(vertex1, vertex2, vertex3, normal, coefBTemp, intersectionDistance);
        Vec3Df intersectionPoint =vertex1*coefBTemp[2]+vertex2*coefBTemp[0]+vertex3*coefBTemp[1];
        intersectionPoint = (intersectionPoint)/(coefBTemp[0]+coefBTemp[1]+coefBTemp[2]);
        if (hasIntersection && intersectionDistance < smallest && node->bbox.contains(intersectionPoint)) {
            coefB[0]=coefBTemp[0];
            coefB[1]=coefBTemp[1];
            coefB[2]=coefBTemp[2];
            found = true;
            idTriangle = *idTri;
            smallest = intersectionDistance;
        }
    }
    if (found)
        return true;
    else
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
    std::vector<Light> lights = scene->getLights();
    Light light = lights[0];

    QProgressDialog progressDialog ("Raytracing...", "Cancel", 0, 100);
    progressDialog.show ();

    //#pragma omp parallel for
    for (unsigned int i = 0; i < screenWidth; i++) {
        progressDialog.setValue ((100*i)/screenWidth);
        for (unsigned int j = 0; j < screenHeight; j++) {


            float tanX = tan (fieldOfView)*aspectRatio;
            float tanY = tan (fieldOfView);

            //Nombre de découpe par dimension du pixel (Antialiasing)
            int aliaNb = 2;
            if(!activeAA) aliaNb=1;
            aliaNb++;

            Vec3Df c (backgroundColor);
            Vec3Df tempc(0.,0.,0.);
            for(int pixi=1; pixi<aliaNb; pixi++){
                for(int pixj=1; pixj<aliaNb; pixj++){
                    Vec3Df stepX = (float (i)-0.5+float(pixi)/float(aliaNb) - screenWidth/2.f)/screenWidth * tanX * rightVector;
                    Vec3Df stepY = (float (j)-0.5+float(pixj)/float(aliaNb) - screenHeight/2.f)/screenHeight * tanY * upVector;
                    Vec3Df step = stepX + stepY;

                    Vec3Df dir = direction + step;
                    dir.normalize ();
                    Vec3Df intersectionPoint;
                    Vec3Df IntersPointNormal;
                    float occlusion;

                    int idObj = getIntersectionPoint(camPos,dir,intersectionPoint,IntersPointNormal,occlusion);

                    if(idObj>=0)
                    {

                        tempc += Brdf(camPos, IntersPointNormal, idObj,intersectionPoint,occlusion,0)/std::pow(aliaNb-1,2);
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

