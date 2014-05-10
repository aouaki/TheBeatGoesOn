#include <math.h>
#include "Kdnode.h"
#include "Object.h"
#include "BoundingBox.h"







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
    for (std::vector<unsigned>::iterator idTri = triangles.begin() ; idTri != triangles.end(); idTri++) {

        for(unsigned i = 0 ; i<3 ; i++) {
            unsigned v = mesh.getTriangles()[*idTri].getVertex(i);
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
    for(std::vector<unsigned>::iterator idTri = triangles.begin() ; idTri != triangles.end(); idTri++) {
        bool isInLeft = true;
        bool isInRight = true;

        unsigned v = mesh.getTriangles()[*idTri].getVertex(0);
        const Vec3Df & vertex0 = mesh.getVertices()[v].getPos();
        v = mesh.getTriangles()[*idTri].getVertex(1);
        const Vec3Df & vertex1 = mesh.getVertices()[v].getPos();
        v = mesh.getTriangles()[*idTri].getVertex(2);
        const Vec3Df & vertex2 = mesh.getVertices()[v].getPos();
        isInLeft = boxTriangleIntersectionTest(vertex0, vertex1, vertex2, leftBox);
        isInRight = boxTriangleIntersectionTest(vertex0, vertex1, vertex2, rightBox);
        if(isInLeft)
            leftTri.push_back(*idTri);
        if(isInRight)
            rightTri.push_back(*idTri);
        if (!isInLeft && !isInRight)
        {
            leftTri.push_back(*idTri);
            rightTri.push_back(*idTri);
        }
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

bool KDNode::boxTriangleIntersectionTest(const Vec3Df & A, const  Vec3Df & B, const  Vec3Df & C, BoundingBox box) {

    //on teste si les points du tri sont dans la box
    if(box.contains(A) || box.contains(B) || box.contains(C)){return true;}

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

    //projection du triangle sur les axes x y z
    //selon x :
    float min=std::min(v1[0],std::min(v2[0],v3[0]));
    float max=std::max(v1[0],std::max(v2[0],v3[0]));
    if(min>width/2 || max<-width/2) return false;
    //selon y :
    min=std::min(v1[1],std::min(v2[1],v3[1]));
    max=std::max(v1[1],std::max(v2[1],v3[1]));
    if(min>height/2 || max<-height/2) return false;
    //selon z :
    min=std::min(v1[2],std::min(v2[2],v3[2]));
    max=std::max(v1[2],std::max(v2[2],v3[2]));
    if(min>length/2 || max<-length/2) return false;
    Vec3Df e0 = v2 - v1;
    Vec3Df e1 = v3 - v2;
    Vec3Df normal = Vec3Df::crossProduct(e0,e1);
    normal.normalize();
    std::vector<Vec3Df> points = box.getPoints();
    float k=0;
    float l=8;
    for(std::vector<Vec3Df>::iterator point = points.begin() ; point != points.end(); point++) {
        float product = Vec3Df::dotProduct(*point, normal);
        if (product > 1)
            k+=1;
        if (product <1)
            k=+1;
        if (product==1)
            l-=1;
    }

    if (k==l || k==-l)
        return false;

    //si tous tests n'ont rien retourné
    return true;
}
