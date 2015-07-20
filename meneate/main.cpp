#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include "cuerpo.h"


int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int MAX_NUM_OBJECTS=50;

const int MIN_DETECT_AREA = 10*10;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

const string windowName = "Video Original";
const string windowName1 = "Imagen HSV";
const string windowName2 = "Image con umbral";
//const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
const string meneate = "Meneate";
void on_trackbar( int, void* )
{

}
string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars(){
	namedWindow(trackbarWindowName,0);
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", H_MIN);
	sprintf( TrackbarName, "H_MAX", H_MAX);
	sprintf( TrackbarName, "S_MIN", S_MIN);
	sprintf( TrackbarName, "S_MAX", S_MAX);
	sprintf( TrackbarName, "V_MIN", V_MIN);
	sprintf( TrackbarName, "V_MAX", V_MAX);
	createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
	createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
	createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
	createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
	createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
	createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );
}

void drawObject(vector <nodo> marcas,Mat &frame){
    for(size_t i=0; i<marcas.size(); ++i){
        cv::circle(frame,cv::Point(marcas.at(i).getx(),marcas.at(i).gety()),10,cv::Scalar(0,0,255));
        cv::putText(frame,intToString(marcas.at(i).getx())+ " , " + intToString(marcas.at(i).gety()),cv::Point(marcas.at(i).getx(),marcas.at(i).gety()+20),1,1,Scalar(0,255,0));
    }
}

void drawNewObject(vector <nodo> marcas,Mat &frame, cuerpo &_cuerpo){
    _cuerpo.drawcuerpo(marcas, frame);
}

void morphOps(Mat &thresh){
	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(4,4));
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(10,10));
	erode(thresh,thresh,erodeElement);
	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
}

void trackFilteredObject(Mat threshold,Mat HSV, Mat &cameraFeed, Mat &figura, cuerpo &humano){
	vector <nodo> marcas;
	Mat temp;
	threshold.copyTo(temp);
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		if(numObjects<MAX_NUM_OBJECTS){
			for (int i=0; i >= 0; i = hierarchy[i][0]) {
				Moments moment = moments((cv::Mat)contours[i]);
				double area = moment.m00;
				if(area>MIN_DETECT_AREA){
                    nodo marca;
                    marca.setx(moment.m10/area);
                    marca.sety(moment.m01/area);
					marcas.push_back(marca);
					objectFound = true;
				}else objectFound = false;
			}
			if(objectFound ==true){
				drawNewObject(marcas,figura,humano);
            }
		}else putText(cameraFeed,"RUIDO! AJUSTANDO FILTRO",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}

int main(int argc, char* argv[])
{
	cuerpo humano;
	bool grabarmeneada = false;
	bool calibrationMode = false;
	Mat cameraFeed;
	Mat threshold;
	Mat HSV;
	cv::Size frameSize(FRAME_WIDTH, FRAME_HEIGHT);
	Mat figura= Mat::zeros(frameSize, CV_8UC1);
    vector<Mat> spl;

	if(calibrationMode){
		createTrackbars();
	}
	VideoCapture captura;
	captura.open(1);
    //
    cv::VideoWriter grabar;

    //figura = Mat::zeros(frameSize, CV_8UC1);
    if(grabarmeneada){
        string filename = "meneate.avi";
        //fcc
        int fcc = CV_FOURCC('D','I','V','3');
        int fps = 10;

        grabar = VideoWriter(filename, fcc, fps, frameSize);
        if(!grabar.isOpened()){
            cout<<"ERROR al grabar archivo"<<endl;
            getchar();
            return -1;
        }

    }
	captura.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	captura.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);

	while(1){

		captura.read(cameraFeed);

		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);

		if(calibrationMode==true){
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
            morphOps(threshold);
            imshow(windowName2,threshold);
            trackFilteredObject(threshold,HSV,cameraFeed,figura, humano);
		}else{
		    nodo marca;
		    marca.setHSVmin(Scalar(24,2,240));
		    marca.setHSVmax(Scalar(85,115,256));
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,marca.getHSVmin(),marca.getHSVmax(),threshold);
            morphOps(threshold);
            trackFilteredObject(threshold,HSV,cameraFeed,figura, humano);
            if(grabarmeneada){
                split(cameraFeed, spl);
                for (int i =0; i < 3; ++i)
                    spl[i] = figura;
                merge(spl, figura);
                grabar.write(figura);
            }
		}
		imshow(windowName2,threshold);
		imshow(windowName,cameraFeed);
		grabar.write(figura);
		imshow(meneate,figura);
		//imshow(windowName1,HSV);
        switch(waitKey(1)){
            case 27://ESC
                return 0;
        }
		waitKey(10);
	}
	return 0;
}
