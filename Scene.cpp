// *********************************************************
// Scene Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Scene.h"
#include "RayTracer.h"

using namespace std;

static Scene * instance = NULL;

Scene * Scene::getInstance () {
    if (instance == NULL)
        instance = new Scene ();
    return instance;
}

void Scene::destroyInstance () {
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

Scene::Scene () {
    buildDefaultScene ();
    updateBoundingBox ();
}

Scene::~Scene () {
}

void Scene::updateBoundingBox () {
    if (objects.empty ())
        bbox = BoundingBox ();
    else {
        bbox = objects[0].getBoundingBox ();
        for (unsigned int i = 1; i < objects.size (); i++)
            bbox.extendTo (objects[i].getBoundingBox ());
    }
}

//Calcul l'ambient occlusion
void Scene::computeAO(int nbRay, float maxDist)
{
    for(unsigned int i =0;i<objects.size();i++)
    {
        Object & o = objects[i];
        Mesh mesh = o.getMesh();
        std::vector<Vertex> vertices = mesh.getVertices();
        int vertSize =vertices.size();


        for(int j=0;j<vertSize;j++)
        {
            int touched=0;
            for(int r=0;r<nbRay;r++)
            {
                Vec3Df normal = vertices[j].getNormal();
                Vec3Df pos = vertices[j].getPos();

                Vec3Df n1;
                Vec3Df n2;

                normal.getTwoOrthogonals(n1,n2);

                float a = -((float)std::rand())/((float)RAND_MAX);
                float b = ((float)std::rand())/((float)RAND_MAX)*2.-1.;
                float c = ((float)std::rand())/((float)RAND_MAX)*2.-1.;

                Vec3Df dir = normal*a+n1*b+n2*c;
                dir.normalize();
                Vec3Df intersectionPoint2;
                Vec3Df IntersPointNormal2;

                RayTracer * rt = RayTracer::getInstance();
                int hasIn = rt->getIntersectionPoint(pos,dir,intersectionPoint2,IntersPointNormal2);

                if(hasIn)
                    if(Vec3Df::distance(pos,intersectionPoint2) < maxDist)
                        touched++;



            }

            o.getMesh().getVertices()[j].setOcc((float)touched/(float)nbRay);

        }
    }
}

// Changer ce code pour creer des scenes originales
void Scene::buildDefaultScene () {


    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Material groundMat(1.f, 1.f, Vec3Df (0.15f, 0.68f, 0.37f));
    Object ground (groundMesh, groundMat);
    //ground.setRefl(0.5f);
    objects.push_back (ground);


    Mesh wallMesh;
    wallMesh.loadOFF ("models/ground.off");
    wallMesh.rotate({0,1,0}, -M_PI/2);
    wallMesh.rotate({0,0,1}, M_PI);
    Material wallMat (1.f, 1.f, Vec3Df (0.75f, .22f, 0.17f));
    Object wall (wallMesh, wallMat);
    wall.setTrans(Vec3Df(-1.95f, 0.f, 1.95f));
    //wall.setRefl(1.f);
    objects.push_back (wall);

    Mesh wall2Mesh;
    wall2Mesh.loadOFF ("models/ground.off");
    wall2Mesh.rotate({0,1,0}, -M_PI/2);
    wall2Mesh.rotate({0,0,1}, 3*M_PI/2);
    Material wall2Mat (1.f, 1.f, Vec3Df (0.75f, .22f, 0.17f));
    Object wall2 (wall2Mesh, wall2Mat);
    wall2.setTrans(Vec3Df(0.f, -1.95f, 1.95f));
    //wall2.setRefl(1.f);
    objects.push_back (wall2);

    Mesh mirrorMesh;
    mirrorMesh.loadOFF ("models/little_ground.off");
    mirrorMesh.rotate({0,1,0}, -M_PI/2);
    mirrorMesh.rotate({0,0,1}, M_PI);
    Material mirrorMat (1.f, 1.f, Vec3Df (0.f, 0.f, 0.f));
    Object mirror (mirrorMesh, mirrorMat);
    mirror.setTrans (Vec3Df (-1.9f, 0.0f, 1.4f));
    mirror.setRefl(1.);
    objects.push_back (mirror);

    /*Mesh wallMesh2;
    wallMesh2.loadOFF ("models/wall.off");
    wallMesh2.rotate({0,0,1}, -M_PI/2);
    Material wallMat2;
    Object wall2 (wallMesh2, wallMat2);
    wall2.setTrans (Vec3Df (0.f, 1.9f, 1.5f));
    objects.push_back (wall2);


    Mesh wallMesh3;
    wallMesh3.loadOFF ("models/wall.off");
    wallMesh3.rotate({0,0,1}, -M_PI/2);
    Material wallMat3;
    Object wall3 (wallMesh3, wallMat3);
    wall3.setTrans (Vec3Df (0.f, -1.9f, 1.5f));
    objects.push_back (wall3);*/

    /*Mesh ballMesh;
    ballMesh.loadOFF ("models/facet_ball_2.off");
    Material ballMat (1.5f, 2.f, Vec3Df (0.5f, 0.5f, 0.5f));
    Object ball (ballMesh, ballMat);
    ball.setRefl(1.0);
    ball.setTrans (Vec3Df (0.0f, 0.0f, 2.0f));
    ball.setSmooth(true);
    ball.resize(3.0);
    objects.push_back (ball);*/

    Mesh ramMesh;
    ramMesh.loadOFF ("models/ram.off");
    //ramMesh.rotate({0,0,1}, M_PI);
    ramMesh.rotate({0,1,0}, M_PI);
    ramMesh.rotate({1,0,0}, -M_PI/8);
    Material ramMat (1.f, 1.f, Vec3Df (1.f, .6f, .2f));
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (1.f, -.7f, 1.2f));
    //ram.resize(100.0);
    objects.push_back (ram);

    Mesh bedMesh;
    bedMesh.loadOFF ("models/bed.off");
    //bedMesh.rotate({0,0,1}, M_PI/2);
    Material bedMat (1.f, 1.f, Vec3Df (1.f, 1.f, 1.f));
    Object bed (bedMesh, bedMat);
    bed.setTrans (Vec3Df (1.f, -1.0f, 0.f));
    objects.push_back (bed);

    Mesh monitorMesh;
    monitorMesh.loadOFF ("models/tv.off");
    //monitorMesh.rotate({0,0,1}, M_PI);
    Material monitorMat (1.f, 1.f, Vec3Df (0.4f, .4f, .4f));
    Object monitor (monitorMesh, monitorMat);
    monitor.setTrans (Vec3Df (1.f, 1.2f, 0.f));
    objects.push_back (monitor);

    /*Mesh rhinoMesh;
    rhinoMesh.loadOFF ("models/rhino.off");
    Material rhinoMat (1.0f, 0.2f, Vec3Df (0.6f, 0.6f, 0.7f));
    Object rhino (rhinoMesh, rhinoMat);
    rhino.setTrans (Vec3Df (-1.f, -1.0f, 0.4f));
    objects.push_back (rhino);
    Mesh gargMesh;
    gargMesh.loadOFF ("models/gargoyle.off");
    Material gargMat (0.7f, 0.4f, Vec3Df (0.5f, 0.8f, 0.5f));
    Object garg (gargMesh, gargMat);
    garg.setTrans (Vec3Df (-1.f, 1.0f, 0.1f));
    objects.push_back (garg);
    */
    Light l (Vec3Df (3.0f, 1.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.f);
    //Light l2 (Vec3Df (3.0f, -3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);
    //lights.push_back (l2);
    //computeAO(2,2.);
}
