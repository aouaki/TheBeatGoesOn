// *********************************************************
// Object Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Object.h"

using namespace std;

void Object::updateBoundingBox () {
    const vector<Vertex> & V = mesh.getVertices ();
    if (V.empty ())
        bbox = BoundingBox ();
    else {
        bbox = BoundingBox (V[0].getPos ());
        for (unsigned int i = 1; i < V.size (); i++)
            bbox.extendTo (V[i].getPos ());
    }
}

void Object::resize(float coef)
{
    vector<Vertex> & V = mesh.getVertices ();

    int size = V.size();

    for(int i =0;i<size;i++)
    {
        Vec3Df newV = V[i].getPos()/coef;
        V[i].setPos(newV);
    }
}
