#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include "opencv2/opencv.hpp" 

using namespace cv;
using namespace std;
 
int main() {
    IplImage *frame = 0;
    CvVideoWriter *writer = 0;
 
    CvFont font;
    //cvInitFont(폰트, 폰트이름, 글자 수평스케일, 글자 수직스케일, 기울임, 굵기) : 폰트 초기화
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 5);


    //cvCaptureFromFile(파일명) : 동영상 파일 불러오기
    CvCapture *capture = cvCreateCameraCapture(0);
     cvNamedWindow("video", CV_WINDOW_AUTOSIZE);
    
    int fps = 30;    //30프레임
 
    //첫번째 프레임으로부터 크기 구하기
    cvGrabFrame(capture);
    frame = cvRetrieveFrame(capture);
    //cvCreateVideoWriter(경로, 코덱방식, fps, 프레임크기) : 동영상파일 생성
    //CV_FOURCC('D', 'I', 'V', 'X') : MPEG-4 코덱
    writer = cvCreateVideoWriter("output.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, cvGetSize(frame));
    while (1) {
        //cvGrabFrame(동영상) : 하나의 프레임을 잡음
        cvGrabFrame(capture);
        //cvRetrieveFrame(동영상) : 잡은 프레임으로부터 이미지를 구함
        frame = cvRetrieveFrame(capture);
 
        if (!frame || cvWaitKey(10) >= 0) { break; }
 
        //cvPutText(이미지, 텍스트, 출력위치, 폰트, 텍스트색상) : 텍스트를 이미지에 추가
        cvPutText(frame, "TEST TEXT", cvPoint(100, 100), &font, CV_RGB(255,0,0));

        //cvWriteFrame(동영상, 프레임) : 지정한 동영상에 프레임을 쓴다
        cvWriteFrame(writer, frame);
 
        cvShowImage("video", frame);
    }
 
    //cvReleaseVideoWriter(동영상) : 메모리에서 해제한다
    cvReleaseVideoWriter(&writer);
 
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
    return 0;
}

