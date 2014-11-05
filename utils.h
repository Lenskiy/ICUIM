//
//  utils.h
//  SmartVideoChat
//
//  Created by Artem Lenskiy on 6/27/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#ifndef ICU_VideoCommunication_h
#define ICU_VideoCommunication_h
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv/highgui.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "Block.h"
#include "ROI.h"

#define MAX_BLOCK_SIDE 32
#define MAX_PACKET_SIZE (MAX_BLOCK_SIDE * MAX_BLOCK_SIDE + BLOCK_HEADER_LENGTH)

struct VideoParams{
    unsigned blockSize;
    unsigned width;
    unsigned height;
    unsigned image_type;
};


class ROI;

enum Frame_Resolution{UKNOWN,QQVGA, QVGA, VGA, SVGA, XGA, WXGA, SXGA, UXGA, FullHD, WUXGA};

class Compression{
public:
    virtual Block &compress(Block &blk) = 0;
    virtual Block &decompress(Block &blk) = 0;
    
    friend class Block;
};

class Resample : Compression{
protected:
    VideoCommunication *vc;
   // int decode(Block &bl);
public:
    Resample(VideoCommunication *vc);
    virtual Block &compress(Block &blk);
    virtual Block &decompress(Block &blk);
    
};

class VideoCommunication{
private:
    int socket_to_send, socket_to_recv;
    long num_byte_recv, num_byte_sent;
    

    
    struct addrinfo serv_addr, *res_serv_addr;
    struct addrinfo clie_addr, *res_clie_addr;
    struct sockaddr temp_addr;

    unsigned long block_length;
    char *contBlock;
    
    std::vector<Block> imageBlocks;     //container for blocks to be sent
    std::vector<Block> blocksReceieved; //container for blocks to be received
    
    VideoParams params;
    Resample compress;
    cv::VideoCapture videoSource;

    
    cv::Mat sendFrame;
    cv::Mat recvFrame;
    
    void sendBlock(const Block &bl);
    Block &receiveBlock(Block &bl);
    
    unsigned curBeginning;
    unsigned numOfPackRecv;
    unsigned numOfBlocksPerFrame;
    unsigned nextFrameBlock; //
public:
    VideoCommunication();
    VideoCommunication(unsigned short port_send, unsigned short port_recv, const std::string ip_address = "");
    ~VideoCommunication();
    
    void setVideoSource(cv::VideoCapture vs);
    int setROI(ROI *roi);
    int setCompression(Compression *compression);

    Frame_Resolution determineResolution();
    void networkInit(unsigned short port_send, unsigned short port_recv, const std::string ip_address);
    
    void initializeVideoParams(unsigned bs, unsigned width, unsigned height);
    
    void transmit();
    void receive();
    
    unsigned receiveBlocks();
    unsigned long sendBlocks();
    
    void getBlocks(const cv::Mat& Frame);
    void updateFrame(cv::Mat &baseFrame, std::vector<Block>  &blocksReceieved);
    
    ROI roi;
    friend class ROI;
    friend class Resample;
    
};




#endif
