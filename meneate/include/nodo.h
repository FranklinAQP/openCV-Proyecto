#ifndef NODO_H
#define NODO_H
#include <string>
#include <opencv\highgui.h>
#include <opencv\cv.h>

using namespace std;
using namespace cv;
class nodo;
class nodo
{
    private:
        int x,y;
        Scalar HSVmin, HSVmax;
    public:
        nodo();
        virtual ~nodo();
        int getx(){return x;}
        int gety(){return y;}
        void setx(int _x){x = _x;}
        void sety(int _y){y = _y;}
        Scalar getHSVmin(){return HSVmin;}
        Scalar getHSVmax(){return HSVmax;}
        Scalar setHSVmin(Scalar _min){HSVmin = _min;}
        Scalar setHSVmax(Scalar _max){HSVmax = _max;}
    protected:

};

#endif // NODO_H
