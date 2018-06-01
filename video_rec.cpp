#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include "opencv2/opencv.hpp" 
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h> // getimeofday( ) 함수에서 사용

using namespace cv;
using namespace std;
 
#define VIDEO_FPS	30
#define TEXT_POS	230, 30
#define DURATION	10
//#define TEXT_ON		


int flag;
struct timeval UTCtime_r;

void *timer_func(void *data)
{
    int time;
    int flagEnd;
    struct timeval UTCtime_s, UTCtime_e;
    gettimeofday(&UTCtime_s, NULL);  // UTC 현재 시간 구하기(마이크로초까지)
    sleep(DURATION-1);
    usleep(999500);
    flag = 0;
    int count=0;

    while(!flag)
    {
	usleep(100);
        gettimeofday(&UTCtime_e, NULL);  // UTC 현재 시간 구하기

        if((UTCtime_e.tv_usec- UTCtime_s.tv_usec)<0)
        {
            UTCtime_r.tv_sec  = UTCtime_e.tv_sec - UTCtime_s.tv_sec - 1;
            UTCtime_r.tv_usec = (1000000 + UTCtime_e.tv_usec) - UTCtime_s.tv_usec;
        }
        else
        {
        UTCtime_r.tv_sec = UTCtime_e.tv_sec - UTCtime_s.tv_sec;
        UTCtime_r.tv_usec = UTCtime_e.tv_usec - UTCtime_s.tv_usec;
        }
        if(UTCtime_r.tv_usec > 999000)
            flag=1;
    }
    //printf("%ld.%d\n",UTCtime_r.tv_sec,UTCtime_r.tv_usec);
}

 
int main(int argc, char *argv[]) 
{
    time_t UTCtime;
    struct tm *tm;
    char buf[BUFSIZ];
    int count=0;
    int run=1;
    int err;
    pthread_t p_thread;

    if((err = pthread_create (&p_thread, NULL, timer_func, NULL) < 0))
    {
        perror("thread create error : ");
        exit(2);
    }
    pthread_detach(p_thread);

    IplImage *frame = 0;
    CvVideoWriter *writer = 0;
    CvFont font;

    //cvInitFont(폰트, 폰트이름, 글자 수평스케일, 글자 수직스케일, 기울임, 굵기) : 폰트 초기화
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 1);

    //cvCaptureFromFile(파일명) : 동영상 파일 불러오기
    CvCapture *capture = cvCreateCameraCapture(0);
    cvNamedWindow("video", CV_WINDOW_AUTOSIZE);
    
    int fps = VIDEO_FPS;   
 
    //첫번째 프레임으로부터 크기 구하기
    cvGrabFrame(capture);
    frame = cvRetrieveFrame(capture);
	
    //time(&UTCtime);
    //tm = localtime(&UTCtime);
    //strftime(buf, sizeof(buf), "%Y%m%j_%H%M%S.avi",tm);

    strcpy(buf, argv[1]);

    //========================================================================
    // cvCreateVideoWriter(경로, 코덱방식, fps, 프레임크기) : 동영상파일 생성
    // 기 능: 처리한 영상을 비디오 파일로 만드는 함수
    // 
    // 코덱 방식
    //========================================================================
    // CV_FOURCC('P','I','M','1') : MPEG-1 코덱
    // CV_FOURCC('M','J','P','G') : motion jpeg codec
    // CV_FOURCC('M','P','4','2') : MPEG-4.2 codec
    // CV_FOURCC('D','I','V','3') : MPEG-4.3 codec
    // CV_FOURCC('D','I','V','X') : MPEG-4 codec
    // CV_FOURCC('U','2','6','3') : H.263 codec
    // CV_FOURCC('I','2','6','3') : H.263I codec
    // CV_FOURCC('F','L','V','1') : FLV1 codec
    //========================================================================

    //CV_FOURCC('D', 'I', 'V', 'X') : MPEG-4 코덱
    writer = cvCreateVideoWriter(buf, CV_FOURCC('D', 'I', 'V', 'X'), fps, cvGetSize(frame));
    while (run) {
        //cvGrabFrame(동영상) : 하나의 프레임을 잡음
        cvGrabFrame(capture);
        //cvRetrieveFrame(동영상) : 잡은 프레임으로부터 이미지를 구함
        frame = cvRetrieveFrame(capture);
 
        if (!frame || cvWaitKey(10) >= 0) { break; }
 
 #ifdef TEXT_ON	
	if((count%5)==0)
	{
		// 현재 시간 얻어와서 문자열로 저장
		time(&UTCtime); 
		tm = localtime(&UTCtime); 
		strftime(buf, sizeof(buf), "%Y-%m-%j %H:%M:%S", tm); 
	}
        //cvPutText(이미지, 텍스트, 출력위치, 폰트, 텍스트색상) : 텍스트를 이미지에 추가
        cvPutText(frame,  buf, cvPoint(TEXT_POS), &font, CV_RGB(255,255,255));
#endif

        //cvWriteFrame(동영상, 프레임) : 지정한 동영상에 프레임을 쓴다
        cvWriteFrame(writer, frame);
 
        cvShowImage("video", frame);
   
	if(flag==1)
	    run=0;
			
        count++;
    }
 
    //cvReleaseVideoWriter(동영상) : 메모리에서 해제한다
    cvReleaseVideoWriter(&writer);
 
    cvReleaseCapture(&capture);
    cvDestroyWindow("video");
													
    printf("%ld.%d\n",UTCtime_r.tv_sec,UTCtime_r.tv_usec);
    return 0;
}

