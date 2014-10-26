//
//  ROI.h
//  SmartVideoChat
//
//  Created by Artem Lenskiy on 7/15/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#ifndef ICU_ROI_h
#define ICU_ROI_h

#include "Block.h"
#ifdef ICU_VideoCommunication_h
#include "utils.h"
#endif
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class VideoCommunication;

class ROI{
    std::vector<cv::CascadeClassifier> cascades;
    
    std::vector<cv::Rect> roi_rects;
    std::vector<unsigned char> block_importance;
    std::vector<unsigned char> user_importance;
    cv::Mat frame_gray;
    VideoCommunication *vc;
    
    //unsigned block_size;
    //unsigned width;
    unsigned char isInRoi(Block &bl);
    
public:
    ROI(VideoCommunication *vc);
    void  analyze(const cv::Mat &frame);
    //void splitBlocks(std::vector<Block> &input, std::vector<Block> &roiBlocks, std::vector<Block> &nonRoiBlocks);
    int addCascade(cv::string xml_url, unsigned char importance);
    
    friend class VideoCommunication;
};

#endif /* defined(__SmartVideoChat__ROI__) */
