// *********************************************************
// Scene Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Scene.h"

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

// Changer ce code pour creer des scenes originales
void Scene::buildDefaultScene () {


    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Material groundMat;
    Object ground (groundMesh, groundMat);
    //ground.setRefl(0.8);
    objects.push_back (ground);

    Mesh wallMesh;
    wallMesh.loadOFF ("models/wall.off");
    Material wallMat (1.f, 1.f, Vec3Df (0.6f, 0.4f, 0.4f));
    Object wall (wallMesh, wallMat);
    wall.setTrans (Vec3Df (-1.9f, 0.0f, 1.5f));
    objects.push_back (wall);

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

    Mesh ballMesh;
    ballMesh.loadOFF ("models/facet_ball_2.off");
    Material ballMat (1.5f, 2.f, Vec3Df (0.5f, 0.5f, 0.5f));
    Object ball (ballMesh, ballMat);
    ball.setRefl(1.0);
    ball.setTrans (Vec3Df (0.0f, 0.0f, 2.0f));
    ball.setSmooth(true);
    ball.resize(3.0);
    objects.push_back (ball);

    Mesh ramMesh;
    ramMesh.loadOFF ("models/ram.off");
    //ramMesh.rotate({0,0,1}, M_PI/2);
    Material ramMat (1.f, 1.f, Vec3Df (1.f, .6f, .2f));
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (1.f, 0.5f, 0.f));
    //ram.resize(100.0);
    objects.push_back (ram);

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
    Light l (Vec3Df (3.0f, 3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.f);
    //Light l2 (Vec3Df (3.0f, -3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);
    //lights.push_back (l2);
}
