//
//  Block.h
//  SmartVideoChat
//
//  Created by Artem Lenskiy on 7/15/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#ifndef ICU_Block_h
#define ICU_Block_h

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <memory>

#define BLOCK_HEADER_LENGTH 4
#define SIDE_SIZE(BLOCK_SIZE)  ((BLOCK_SIZE) % 32 ? ((BLOCK_SIZE) % 16 ? ((BLOCK_SIZE) % 8 ? ((BLOCK_SIZE) % 4 ? ((BLOCK_SIZE) % 2 ? 1 : 2) : 4) : 8) : 16) : 32) 





class Block{
protected:
    //cv::Ptr<uchar> data;
    std::shared_ptr<uchar> data;
    //uchar *data;
    //unsigned short *p_sq_number;
    //unsigned short *p_compr_factor;
    unsigned *p_blk_header; // right to left: 22 bits sq_id, 3b block size, 7b compression params
    
    
    
public:
    Block(unsigned blkSize, unsigned matType = CV_8UC3);
    Block(const Block& other);
    Block(cv::Mat& mat);
    Block & operator=(const Block &rhs);

    ~Block();
    
    
    size_t getSize() const;
    
    unsigned  getSqId() const;
    void setSqId(unsigned sq_number);
    
    void setCompParams(unsigned params);
    unsigned getCompParams() const;
    
    void setBlockSize(unsigned params);
    unsigned getBlockSize() const;
    
    void setToZero();
    void print() const;
    //unsigned getBlockSide();
    
    cv::Mat mat;
    long total_block_length;
    
    friend class VideoCommunication;
    friend class Resample;
    friend class JPEGCompression;
};

#endif
