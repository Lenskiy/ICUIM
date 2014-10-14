//
//  ROI.cpp
//  SmartVideoChat
//
//  Created by Artem Lenskiy on 7/15/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#include "ROI.h"
#include "utils.h"

ROI::ROI(VideoCommunication *vc):vc(vc){}

int ROI::addCascade(cv::string xml_url){
    cv::CascadeClassifier cc;
    cascades.push_back(cc);
    if( !cascades.back().load( xml_url ) ){ printf("--(!)Error loading\n"); cascades.pop_back(); return -1; };
    return 0;
}

void ROI::analyze(const cv::Mat &frame){
    roi_rects.clear();
    cv::cvtColor( frame, frame_gray, CV_BGR2GRAY );
    cv::equalizeHist( frame_gray, frame_gray );
    
    std::vector<cv::Rect> temp_rects;
    
    
    for (int i = 0; i < cascades.size(); i++){
        cascades.at(i).detectMultiScale(frame_gray, temp_rects, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(50, 50) );
        roi_rects.insert(roi_rects.end(), temp_rects.begin(), temp_rects.end());
    }
    
    //imshow( "Capture - Face detection", frame_gray );
    //printf("\nNumber of face is %ld", faces.size());
    temp_rects.clear();
    frame_gray.release();
}

unsigned char ROI::isInRoi(Block &bl){
    
    unsigned block_x_center = ((bl.getSqId() * bl.getBlockSize()) %  vc->params.width) + bl.getBlockSize()/2;
    unsigned block_y_center = (int ((bl.getSqId() * bl.getBlockSize()) /  vc->params.width)) * bl.getBlockSize() + bl.getBlockSize()/2;
    
    std::vector<cv::Rect>::iterator it;
    
    //printf("\nx = %u, y = %u", block_x_center, block_y_center);
    for (it = roi_rects.begin();it != roi_rects.end(); ++it){
        if((block_x_center > it->x) && (block_x_center < (it->x + it->width)) &&
           (block_y_center > it->y) && (block_y_center < (it->y + it->height)))
            return 1;
    }
    
    return 0;
}
