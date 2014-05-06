#include <math.h>
#include "Kdnode.h"
#include "Object.h"

KDNode::KDNode(Object &o):o(o),
                                 axis(0),
                                 bbox(o.getBoundingBox()) {
    const Mesh & mesh = o.getMesh();
    triangles.resize(mesh.getTriangles().size());
    for(unsigned int i = 0 ; i < mesh.getTriangles().size() ; i++)
    {
        triangles[i] = i;
    }
    buildKDTree(0);
}

KDNode::KDNode(Object &o, std::vector<unsigned> partition, int &axis, float &q, BoundingBox &box): o(o), triangles(partition), axis(axis), median(q), bbox(box) {
    buildKDTree(median);
}

inline float KDNode::findMedianSample(std::vector<unsigned> & triangles, int dim){
    std::vector<float> positions;
//    positions.resize(triangles.size());
    const Mesh & mesh = o.getMesh();
    for (unsigned idT=0; idT<triangles.size(); idT++) {
        unsigned t = triangles[idT];

        for(unsigned i = 0 ; i<3 ; i++) {
            unsigned v = mesh.getTriangles()[t].getVertex(i);
            Vec3Df p = mesh.getVertices()[v].getPos();
            positions.push_back( p[dim] );
            }
        }

    std::vector<float>::iterator first = positions.begin();
    std::vector<float>::iterator last = positions.end();
    std::vector<float>::iterator middle = first + positions.size() / 2;
    std::nth_element(first, middle, last);
    unsigned mil = floor(positions.size()/2);
    return positions[mil];
}

void KDNode::splitTriangles(std::vector <unsigned> &leftTri, std::vector <unsigned> &rightTri, BoundingBox &leftBox, BoundingBox &rightBox){
    const Mesh & mesh = o.getMesh();
    for(unsigned idT=0; idT<triangles.size(); idT++) {
        unsigned t = triangles[idT];
        bool isInLeft = false;
        bool isInRight = false;

        unsigned v = mesh.getTriangles()[t].getVertex(0);
        const Vec3Df & vertex0 = mesh.getVertices()[v].getPos();
        v = mesh.getTriangles()[t].getVertex(1);
        const Vec3Df & vertex1 = mesh.getVertices()[v].getPos();
        v = mesh.getTriangles()[t].getVertex(2);
        const Vec3Df & vertex2 = mesh.getVertices()[v].getPos();
        isInLeft = boxTriangleIntersectionTest(vertex0, vertex1, vertex2, leftBox);
        isInRight = boxTriangleIntersectionTest(vertex0, vertex1, vertex2, rightBox);
        if(isInLeft)
            leftTri.push_back(t);
        if(isInRight)
            rightTri.push_back(t);
    }
}

void KDNode::buildKDTree (float oldMed){
    if(triangles.size() <= MIN_TRIANGLES) {return;}
    axis = bbox.getMaxAxis();
    median = findMedianSample(triangles, axis);
    if(median==oldMed) {return;}
    BoundingBox leftBox;
    BoundingBox rightBox;
    bbox.split(median, axis, leftBox, rightBox);
    std::vector <unsigned> leftTri;
    std::vector <unsigned> rightTri;
    splitTriangles(leftTri, rightTri, leftBox, rightBox);
    leftChild = new KDNode(o, leftTri, axis, median, leftBox);
    rightChild = new KDNode(o, rightTri, axis, median, rightBox);
    triangles.clear();
}

bool KDNode::boxTriangleIntersectionTest(const Vec3Df &A, const Vec3Df &B, const Vec3Df &C, BoundingBox box){

    float min,max,p0,p1,p2,rad,fex,fey,fez;
    //on récupère infos
    Vec3Df center = box.getCenter();
    Vec3Df maxBb = box.getMax();
    Vec3Df minBb = box.getMin();
    float width = box.getWidth();
    float height = box.getHeight();
    float length = box.getLength();
    //translation pour calculs facilités
    maxBb = maxBb - center;
    minBb = minBb - center;
    Vec3Df v1 = A - center;
    Vec3Df v2 = B - center;
    Vec3Df v3 = C - center;

    //1ERE SERIE TESTS
    //selon x :
    min=std::min(v1[0],std::min(v2[0],v3[0]));
    max=std::max(v1[0],std::min(v2[0],v3[0]));
    if(min>width/2 || max<-width/2) return false;
    //selon y :
    min=std::min(v1[1],std::min(v2[1],v3[1]));
    max=std::max(v1[1],std::min(v2[1],v3[1]));
//    if(min>height/2 || max<-height/2) return false;
    //selon z :
    min=std::min(v1[2],std::min(v2[2],v3[2]));
    max=std::max(v1[2],std::min(v2[2],v3[2]));
//    if(min>length/2 || max<-length/2) return false;

    //2EME SERIE DE TESTS
    Vec3Df e0 = v2 - v1;
    Vec3Df e1 = v3 - v2;
    Vec3Df e2 = v1 - v3;
    Vec3Df normal = Vec3Df::crossProduct(e0,e1);
    float v;
    Vec3Df vmin,vmax;
    Vec3Df maxbox = Vec3Df(width/2,height/2,length/2);

    for(int i = 0 ; i<3 ; i++){

        v=v1[i];

            if(normal[i]>0.0f){

                vmin[i]=-maxbox[i] - v;

                vmax[i]= maxbox[i] - v;
            }
            else{

                vmin[i]= maxbox[i] - v;

                vmax[i]=-maxbox[i] - v;

                }


    }
    if(Vec3Df::dotProduct(normal,vmin)>0.0f){
        return false;}

    if(Vec3Df::dotProduct(normal,vmax)>=0.0f){
        return true;}

    //3EME SERIE DE TESTS
    Vec3Df boxhalfsize=maxbox;
    //selon x :
    fex = std::fabs(e0[0]);  //noté fabsf(...) mais fabsf n'existe pas ???
    fey = std::fabs(e0[1]);
    fez = std::fabs(e0[2]);

   // AXISTEST_X01(e0[Z], e0[Y], fez, fey);

    p0 = e0[2]*v1[1] - e0[1]*v1[2];

    p2 = e0[2]*v3[1] - e0[1]*v3[2];

    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}

    rad = fez * boxhalfsize[1] + fey * boxhalfsize[2];

    if(min>rad || max<-rad) return false;

   // AXISTEST_Y02(e0[Z], e0[X], fez, fex);

    p0 = -e0[2]*v1[1] + e0[0]*v1[2];

    p2 = -e0[2]*v3[1] + e0[0]*v3[2];

    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}

    rad = fez * boxhalfsize[0] + fex * boxhalfsize[2];

    if(min>rad || max<-rad) return false;

  // AXISTEST_Z12(e0[Y], e0[X], fey, fex);
    p1 = e0[1]*v2[1] - e0[0]*v2[2];

    p2 = e0[1]*v3[1] - e0[0]*v3[2];

    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}

    rad = fey * boxhalfsize[0] + fex * boxhalfsize[1];

    if(min>rad || max<-rad) return false;

    //selon y :
    fex = std::fabs(e1[0]);
    fey = std::fabs(e1[1]);
    fez = std::fabs(e1[2]);

    //AXISTEST_X01(e1[Z], e1[Y], fez, fey);

    p0 = e1[2]*v1[1] - e1[1]*v1[2];

    p2 = e1[2]*v3[1] - e1[1]*v3[2];

    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}

    rad = fez * boxhalfsize[1] + fey * boxhalfsize[2];

    if(min>rad || max<-rad) return true;

    //AXISTEST_Y02(e1[Z], e1[X], fez, fex);

    p0 = -e1[2]*v1[1] + e1[0]*v1[2];

    p2 = -e1[2]*v3[1] + e1[0]*v3[2];

    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}

    rad = fez * boxhalfsize[0] + fex * boxhalfsize[2];

    if(min>rad || max<-rad) return false;

    //AXISTEST_Z0(e1[Y], e1[X], fey, fex);

    p0 = e1[1]*v1[0] - e1[0]*v1[1];

    p1 = e1[1]*v2[0] - e1[0]*v2[1];

    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}

    rad = fey * boxhalfsize[0] + fex * boxhalfsize[1];

    if(min>rad || max<-rad) return false;

    //selon z :
    fex = std::fabs(e2[0]);
    fey = std::fabs(e2[1]);
    fez = std::fabs(e2[2]);

    //AXISTEST_X2(e2[Z], e2[Y], fez, fey);
    p0 = e2[2]*v1[1] - e2[1]*v1[2];

    p1 = e2[2]*v2[1] - e2[1]*v2[2];

    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}

    rad = fez * boxhalfsize[1] + fey * boxhalfsize[2];

    if(min>rad || max<-rad) return false;

    //AXISTEST_Y1(e2[Z], e2[X], fez, fex);
    p0 = -e2[2]*v1[0] + e2[0]*v1[2];

    p1 = -e2[2]*v2[0] + e2[0]*v2[2];

    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}

    rad = fez * boxhalfsize[0] + fex * boxhalfsize[2];

    if(min>rad || max<-rad) return false;

    //AXISTEST_Z12(e2[Y], e2[X], fey, fex);
    p1 = e2[1]*v2[1] - e2[0]*v2[2];

    p2 = e2[1]*v3[1] - e2[0]*v3[2];

    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}

    rad = fey * boxhalfsize[0] + fex * boxhalfsize[1];

    if(min>rad || max<-rad) return false;


    //si tous tests n'ont rien retourné
    return true;
}
