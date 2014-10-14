#include "utils.h"


int main(int argc, char *argv[]){
    //transmission simulation
    //VideoCommunication vc;
    //vc.initializeVideoParams(videoSource, 8);
    //    vc.simulate();
    if(argc == 4){
        cv::VideoCapture videoSource(0);
        videoSource.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        videoSource.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        
        //cv::VideoCapture videoSource("udp://127.0.0.1:10000?pkt_size=1316");
        //cv::VideoCapture videoSource("rtmp://cdn-sov-2.musicradio.com:80/LiveVideo/Heart");
        if (!videoSource.isOpened()) {
            printf("Cannot open video stream\n"); //Error
            return 0;
        }
        //broadcast video
        VideoCommunication vc(atoi(argv[1]), atoi(argv[2]), argv[3]);
        vc.setVideoSource(videoSource);
        vc.initializeVideoParams(16);
        vc.roi.addCascade("/usr/local//Cellar/opencv/2.4.9/share/OpenCV/haarcascades/haarcascade_profileface.xml");
        vc.roi.addCascade("/usr/local//Cellar/opencv/2.4.9/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml");
        //vc.initializeVideoParams(videoSource, 8);
        //vc.sendVideoTransmissionParameters(videoSource, 8);
        vc.transmit();
    }else if(argc == 3){
        //recieve video
        VideoCommunication vc(atoi(argv[1]), atoi(argv[2]));
        vc.initializeVideoParams(16);
        //vc.receivedVideoTransmissionParameters();
        vc.receive();
    }else
        std::cout << "Wrong number of parameters" << std::endl;
    
    return 0;
}