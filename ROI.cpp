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

int ROI::addCascade(cv::string xml_url, unsigned char imp){
    cv::CascadeClassifier cc;
    cascades.push_back(cc);
    if( !cascades.back().load( xml_url ) ){ printf("--(!)Error loading\n"); cascades.pop_back(); return -1; };
    user_importance.push_back(imp);
    return 0;
}

void ROI::analyze(const cv::Mat &frame){
    roi_rects.clear();
    block_importance.clear();
    cv::cvtColor( frame, frame_gray, CV_BGR2GRAY );
    cv::equalizeHist( frame_gray, frame_gray );
    
    std::vector<cv::Rect> temp_rects;
    
    
    for (int i = 0; i < cascades.size(); i++){
        cascades.at(i).detectMultiScale(frame_gray, temp_rects, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(10, 10) );
        roi_rects.insert(roi_rects.end(), temp_rects.begin(), temp_rects.end());
        for (int j = 0; j < temp_rects.size(); j++)
            block_importance.insert(block_importance.end(), user_importance.at(i));
    }
    
    //imshow( "Capture - Face detection", frame_gray );
    //printf("\nNumber of face is %ld", faces.size());
    temp_rects.clear();
    frame_gray.release();
}

unsigned char ROI::isInRoi(Block &bl){
    
    unsigned block_x_center = ((bl.getSqId() * bl.getBlockSize()) %  vc->params.width) + bl.getBlockSize()/2;
    unsigned block_y_center = (int ((bl.getSqId() * bl.getBlockSize()) /  vc->params.width)) * bl.getBlockSize() + bl.getBlockSize()/2;
    int delta_x, delta_y;
    unsigned char imp_max = 0;
    std::vector<cv::Rect>::iterator it;
    std::vector<unsigned char>::iterator it_imp;
    
    //printf("\nx = %u, y = %u", block_x_center, block_y_center);
    for (it = roi_rects.begin(), it_imp = block_importance.begin();it != roi_rects.end(); ++it, ++it_imp){
//        if((block_x_center > it->x) && (block_x_center < (it->x + it->width)) &&
//           (block_y_center > it->y) && (block_y_center < (it->y + it->height)))
//            imp_max = MAX(imp_max, *it_imp);
        delta_x = (block_x_center - (it->x + it->width/2));
        delta_y = (block_y_center - (it->y + it->height/2));
        if( delta_x * delta_x + delta_y * delta_y < (it->width * it->height)/4)
            imp_max = MAX(imp_max, *it_imp);
    }
    return imp_max;
}
