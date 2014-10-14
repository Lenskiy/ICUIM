//
//  util.cpp
//  Video_compression
//
//  Created by Artem Lenskiy on 5/24/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#include "utils.h"
#include "Block.h"

Resample::Resample(VideoCommunication *vc):vc(vc){
}

/*
 //bl.print();
 //printf("\n\n");
 unsigned compressionFactor = 1 << bl.getCompParams();
 cv::Rect region_with_data(0, 0, (bl.getBlockSize() * bl.getBlockSize())/(compressionFactor * compressionFactor), 1);
 cv::Mat temp(bl.mat);
 //bl.mat.copyTo(temp);
 cv::resize(temp, temp, cv::Size(bl.getBlockSize()/compressionFactor, bl.getBlockSize()/compressionFactor)); //, 0, 0, cv::INTER_LANCZOS4
 temp = temp.reshape(0, 1);
 bl.mat = bl.mat.reshape(0,1);
 cv::Mat compressed = bl.mat(region_with_data);
 //bl.mat.cols = bl.mat.rows = bl.mat.cols/(1 << bl.getCompParams());
 temp.copyTo(compressed);
 if (bl.getSqId() == 5) {
 printf("\n\n");
 Block tempbl(compressed);
 tempbl.mat = tempbl.mat.reshape(0, bl.getBlockSize()/compressionFactor);
 tempbl.print();
 printf("\n%d",tempbl.mat.isContinuous());
 }
 bl.mat = bl.mat.reshape(0,bl.getBlockSize());
 //temp.copySize(bl.mat);
 bl.total_block_length = (bl.getBlockSize() * bl.getBlockSize())/(compressionFactor * compressionFactor) * bl.mat.elemSize() + BLOCK_HEADER_LENGTH;
 if (bl.getSqId() == 0) {
 //bl.print();
 //printf("\n\n");
 }
 temp.release();
 return bl;
 */
Block &Resample::compress(Block &bl){
    unsigned compressedSized = bl.getBlockSize() / (1 << bl.getCompParams());
    //Block compressed_block
    //cv::Mat temp = bl.mat(cv::Rect(0,0,compressedSized,compressedSized));
    cv::Mat temp;
    cv::resize(bl.mat, temp, cv::Size(compressedSized, compressedSized)); //, 0, 0, cv::INTER_LANCZOS4  //Upsample
    Block compressed_block(temp);
    compressed_block.setSqId(bl.getSqId());
    compressed_block.setCompParams(bl.getCompParams());
    bl = compressed_block;
    bl.total_block_length = compressedSized * compressedSized * bl.mat.elemSize() + BLOCK_HEADER_LENGTH;
    return bl;
}

/*
 //bl.print();
 unsigned compressionFactor = 1 << bl.getCompParams();
 cv::Rect region_with_data(0, 0, (bl.getBlockSize() * bl.getBlockSize())/(compressionFactor * compressionFactor), 1);
 //bl.mat.cols = bl.mat.rows = bl.getBlockSize()/(1 << bl.getCompParams());
 cv::Mat temp = bl.mat.reshape(bl.mat.channels(), 1);
 //bl.print();
 temp = temp(region_with_data);
 //bl.mat.copyTo(temp);
 temp = temp.reshape(0,bl.getBlockSize()/compressionFactor);
 //(bl.getBlockSize()/(1 << bl.getCompParams()), bl.getBlockSize()/(1 << bl.getCompParams()), bl.mat.type())
 //If size of both temp and bl.mat is the same no new memory will be allocated.
 if (bl.getSqId() == 5) {
 printf("\n\n");
 Block tempbl(temp);
 tempbl.print();
 printf("\n%d",tempbl.mat.isContinuous());
 }
 cv::resize(temp, bl.mat, cv::Size(bl.getBlockSize(), bl.getBlockSize())); //, 0, 0, cv::INTER_LANCZOS4  //Upsample
 //bl.mat = bl.mat.reshape(0, bl.getBlockSize());
 //temp.copyTo(bl.mat);
 bl.setCompParams(0);
 bl.total_block_length = bl.mat.cols * bl.mat.rows * bl.mat.elemSize() + BLOCK_HEADER_LENGTH;
 temp.release();
 return bl;
 */
Block &Resample::decompress(Block &bl){
    unsigned unCompressedSized = bl.getBlockSize() * (1 << bl.getCompParams());
    cv::Mat temp;
    cv::resize(bl.mat, temp, cv::Size(unCompressedSized, unCompressedSized)); //, 0, 0, cv::INTER_LANCZOS4  //Upsample
    Block compressed_block(temp);
    compressed_block.setSqId(bl.getSqId());
    compressed_block.setCompParams(0);
    bl = compressed_block;
    bl.total_block_length = unCompressedSized * unCompressedSized * bl.mat.elemSize() + BLOCK_HEADER_LENGTH;
    return bl;
}
/*
int Resample::decode(Block &bl){
    switch ((bl.total_block_length - BLOCK_HEADER_LENGTH) / 3) {
        case 1024:  bl.mat.rows = bl.mat.cols = 32;  break;
        case 256:   bl.mat.rows = bl.mat.cols = 16;  break;
        case 64:    bl.mat.rows = bl.mat.cols = 8;   break;
        case 16:    bl.mat.rows = bl.mat.cols = 4;   break;
        case 4:     bl.mat.rows = bl.mat.cols = 2;   break;
        case 1:     bl.mat.rows = bl.mat.cols = 1;   break;
        default: return -1; break;
    }
    return 1;
}
*/

VideoCommunication::VideoCommunication():roi(this),compress(this){
    socket_to_send = 0;
}

VideoCommunication::VideoCommunication(unsigned short port_send, unsigned short port_recv, const std::string ip_address):roi(this),compress(this){
    socket_to_send = 0;
    socket_to_recv = 0;
    curBeginning = 0;
    numOfPackRecv = 0;
    numOfBlocksPerFrame = 0;
    nextFrameBlock = 0;
    
    if(ip_address.length() > 0)
        networkInit(port_recv, port_send, ip_address);
    else
        networkInit(port_recv, port_send, "127.0.0.1");
}

VideoCommunication::~VideoCommunication(){
    close(socket_to_recv);
    close(socket_to_send);
       
    imageBlocks.clear();
    blocksReceieved.clear();    
}

Frame_Resolution VideoCommunication::determineResolution(){
    if (sendFrame.empty())
        return Frame_Resolution::UKNOWN;
    
    if (sendFrame.cols == 160 && sendFrame.rows == 140) {
        return Frame_Resolution::QQVGA;
    }else if(sendFrame.cols == 320 && sendFrame.rows == 240){
        return Frame_Resolution::QVGA;
    }else if(sendFrame.cols == 640 && sendFrame.rows == 480){
        return Frame_Resolution::VGA;
    }else if(sendFrame.cols == 800 && sendFrame.rows == 600){
        return Frame_Resolution::SVGA;
    }else if(sendFrame.cols == 1024 && sendFrame.rows == 768){
        return Frame_Resolution::XGA;
    }else if(sendFrame.cols == 1280 && sendFrame.rows == 800){
        return Frame_Resolution::WXGA;
    }else if(sendFrame.cols == 1280 && sendFrame.rows == 1024){
        return Frame_Resolution::SXGA;
    }else if(sendFrame.cols == 1600 && sendFrame.rows == 1200){
        return Frame_Resolution::WXGA;
    }else if(sendFrame.cols == 1920 && sendFrame.rows == 1080){
        return Frame_Resolution::FullHD;
    }else if(sendFrame.cols == 1920 && sendFrame.rows == 1200){
        return Frame_Resolution::WUXGA;
    }else
        return Frame_Resolution::UKNOWN;
}

void VideoCommunication::networkInit(unsigned short port_send, unsigned short port_recv, const std::string ip_address){
    int err;
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    char port_str[6];

    serv_addr.ai_family = AF_INET;
    serv_addr.ai_socktype = SOCK_DGRAM;
    serv_addr.ai_protocol = IPPROTO_UDP;
    serv_addr.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sprintf(port_str, "%d", port_recv);
    err = getaddrinfo(NULL, port_str, &serv_addr, &res_serv_addr);
    if (err!=0) {
        printf("%s", gai_strerror(err));
        abort();
    }
    
    if((socket_to_recv = socket(res_serv_addr->ai_family, res_serv_addr->ai_socktype, res_serv_addr->ai_protocol)) < 0){
        printf("\n Error : Could not create socket \n");
        abort();
    }
    
    int tos=5;
    socklen_t toslen=0;
    
    if (setsockopt(socket_to_recv, IPPROTO_IP, IP_TOS,  &tos, sizeof(toslen)) < 0) {
        perror("error to get option");
    }else {
        printf ("changing tos opt = %d\n",tos);
    }

    
    tos = 0;
    if (getsockopt(socket_to_recv, IPPROTO_IP, IP_TOS,  &tos, &toslen) < 0) {
        perror("error to get option");
    }else {
        printf ("current tos opt = %d\n",tos);
    }
    
    
    int buf_size = 1500000;
    socklen_t buf_size_len=0;
    if (setsockopt(socket_to_recv, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(int)) == -1) {
        fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
    }
    
    if (getsockopt(socket_to_recv, SOL_SOCKET, SO_RCVBUF,  &buf_size, &buf_size_len) < 0) {
        perror("error to get option");
    }else {
        printf ("current buffer length = %d\n",buf_size);
    }
    
    bind(socket_to_recv, res_serv_addr->ai_addr, res_serv_addr->ai_addrlen);

    memset(&clie_addr, '0', sizeof(clie_addr));
    clie_addr.ai_family = AF_INET;
    clie_addr.ai_socktype = SOCK_DGRAM;
    clie_addr.ai_protocol = IPPROTO_UDP;
    clie_addr.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    sprintf(port_str, "%d", port_send);
    err = getaddrinfo(NULL, port_str, &clie_addr, &res_clie_addr);
    if (err!=0) {
        printf("%s", gai_strerror(err));
        abort();
    }
    if((socket_to_send = socket(res_clie_addr->ai_family, res_clie_addr->ai_socktype, res_clie_addr->ai_protocol)) < 0){
        printf("\n Error : Could not create socket \n");
        abort();
    }
    
    if (setsockopt(socket_to_send, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(int)) == -1) {
        fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
    }
    
    tos=5;
    if (setsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, sizeof(toslen)) < 0) {
        perror("error to get option");
    }else {
        printf ("changing tos opt = %d\n",tos);
    }
    
    
    //tos = 0;
    if (getsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, &toslen) < 0) {
        perror("error to get option");
    }else {
        printf ("current tos opt = %d\n",tos);
    }
    
}

void VideoCommunication::setVideoSource(cv::VideoCapture vs){
    videoSource = vs;
}

void VideoCommunication::initializeVideoParams(unsigned bs){
    params.blockSize = bs;
    
    
    params.width = 640;//sendFrame.cols;
    params.height = 480;//sendFrame.rows;
    //params.image_type = sendFrame.type();
    params.image_type = CV_8UC3;
    
    recvFrame = cv::Mat(params.height, params.width, params.image_type);
    
    numOfBlocksPerFrame = params.width * params.height / (params.blockSize * params.blockSize);
    for (int i = 0; i < numOfBlocksPerFrame; i++) {
        Block bl1(params.blockSize, params.image_type);
        imageBlocks.push_back(bl1);
        
        Block bl2(params.blockSize, params.image_type);
        blocksReceieved.push_back(bl2);
    }
}

Block &VideoCommunication::receiveBlock(Block &bl){
    socklen_t slen=sizeof(temp_addr);
    bl.mat =  bl.mat.reshape(0, 1); // Make sure that memory that is used to store a matrix is continous
    if((num_byte_recv = recvfrom(socket_to_recv, bl.data.get(), bl.getSize(), 0, (struct sockaddr *) &temp_addr, &slen)) <= 0){
        printf("Only %ld bytes have been sent", num_byte_sent); abort();
    }
//    if (bl.getSqId() == 5) {
//        printf("\n\n");
//        bl.print();
//        printf("\n%d",bl.mat.isContinuous());
//    }
    bl.total_block_length = num_byte_recv; //initialize total block length
    bl.mat = bl.mat.reshape(0, bl.getBlockSize());
    //printf("%d\t%d\t%d\n", bl.getSqId(), bl.getBlockSize(), bl.getCompParams());
    return bl;
    
}

unsigned VideoCommunication::receiveBlocks(){
    curBeginning = (curBeginning + numOfPackRecv - nextFrameBlock) % numOfBlocksPerFrame;
    unsigned curIndex = 0;
    int seqid_prev = -1;
    
    for (numOfPackRecv = nextFrameBlock; numOfPackRecv < numOfBlocksPerFrame; numOfPackRecv++) {
        curIndex = (curBeginning + numOfPackRecv) % numOfBlocksPerFrame;
        compress.decompress(receiveBlock(blocksReceieved.at(curIndex)));
        if ( (int)blocksReceieved.at(curIndex).getSqId() < seqid_prev && blocksReceieved.at(curIndex).getSqId() != (numOfBlocksPerFrame - 1)) {
            //printf("\n\t%d:\t\t%d:\t\t%d\t\t%d\t\t%d", curBeginning - nextFrameBlock, numOfPackRecv, blocksReceieved.at(curIndex-1).getSqId(), blocksReceieved.at(curIndex).getSqId(), blocksReceieved.at(curIndex+1).getSqId());
            nextFrameBlock = 1; // we got a packet from a next block
            break;
    }
       
        nextFrameBlock = 0;
        seqid_prev = blocksReceieved.at(curIndex).getSqId();
    }
    //printf("\n\t%d", numOfPackRecv);
    return numOfPackRecv;
}



void VideoCommunication::sendBlock(const Block &bl){
//send block as one paket
    int tos;
    if (bl.getCompParams() != 0)
        tos = 0;
    else
        tos = 7;
    
    if (setsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, sizeof(tos)) < 0)
        perror("error to get option");
    else
        ;//printf ("changing tos opt = %d\n",tos);

    if((num_byte_sent = sendto(socket_to_send, bl.data.operator->(), bl.getSize(), 0, res_clie_addr->ai_addr, res_clie_addr->ai_addrlen)) != bl.getSize()){
        printf("Only %ld bytes have been sent", num_byte_sent); abort();
    }
//    if (bl.getSqId() == 5) {
//        printf("\n\n");
//        bl.print();
//        printf("\n%d",bl.mat.isContinuous());
//    }
}


unsigned long VideoCommunication::sendBlocks(){
    unsigned long blocks_sent = 0;
    if (imageBlocks.size() == 0)
        return 0;
    
    //transmit blocks one by one
    for (unsigned i = 0; i < numOfBlocksPerFrame; i++){
            sendBlock(compress.compress(imageBlocks.at(i))); //////////////////////// check is we send sqe = 0
            blocks_sent++;
    }
    
    //printf("%lu\n",blocks_sent);
    return blocks_sent;
}

void VideoCommunication::getBlocks( const cv::Mat& Frame){
    //Block *bl;
    unsigned sq_number = 0;
    unsigned blockSize = this->params.blockSize;
    cv::Mat tempMat;
    if(Frame.cols == 0 || Frame.rows == 0)
        return;
    roi.analyze(Frame);
    for(int j = 0; j < (params.height/blockSize); ++j) {
        for(int i = 0; i < (params.width/blockSize); ++i) {
            //bl = &imageBlocks.at(sq_number);
            //Frame(cv::Rect(i*blockSize, j*blockSize, blockSize, blockSize)).copyTo(bl->mat);
            //Initialuze the header
            //bl->setBlockSize(blockSize);
           
            tempMat = Frame(cv::Rect(i*blockSize, j*blockSize, blockSize, blockSize));
            Block bl  = Block(tempMat);
            bl.setBlockSize(blockSize);
            bl.setSqId(sq_number);
            
            if(roi.isInRoi(bl))
                bl.setCompParams(0);
            else
                bl.setCompParams(3);
            imageBlocks.at(sq_number) = bl;
            sq_number++;
        }
    }
    
    for (unsigned i = 0; i < numOfBlocksPerFrame; i++)
        compress.decompress(compress.compress(imageBlocks.at(i))); //////////////////////// check is we send sqe = 0
    
    
//    numOfPackRecv = 1200;
//    updateFrame(recvFrame, imageBlocks);
//    std::string windowName = "Test Video Stream";
//    cv::namedWindow( windowName, CV_WINDOW_FREERATIO);
//    cv::imshow( windowName, recvFrame );
    
    
}

void VideoCommunication::updateFrame(cv::Mat &baseFrame, std::vector<Block>  &blocksReceieved){
    unsigned block_x, block_y;

    if(baseFrame.empty())
        return;
    
    cv::Mat tempROI;
    Block *bl;
    unsigned curIndex;
    //std::vector<Block>::iterator it = blocksReceieved.begin();
    printf("\n\n");
    //blocksReceieved.at(0).print();
    for (unsigned i = 0; i < numOfPackRecv; i++){
        curIndex = (curBeginning + i) % numOfBlocksPerFrame;
        bl = &blocksReceieved.at(curIndex);
        block_x = (bl->getSqId() * bl->getBlockSize()) %  params.width;
        block_y = (int ((bl->getSqId() * bl->getBlockSize()) /  params.width)) * bl->getBlockSize();
        tempROI = baseFrame(cv::Rect(block_x, block_y, bl->getBlockSize(), bl->getBlockSize()));
        bl->mat.copyTo(tempROI);
        //if(bl->getCompParams() == 0)
            //printf("(%d)\n", bl->getSqId());
            //printf("(%d,\t%d)\n", block_x, block_y);
            //bl->print();
    }
}

void VideoCommunication::transmit(){
    std::string windowName = "Broadcating Video Stream";
    cv::namedWindow( windowName, CV_WINDOW_FREERATIO);
    videoSource.grab();
    videoSource.retrieve(sendFrame);
    int blocks_sent = 0;
    do{
        getBlocks(sendFrame);
        blocks_sent = sendBlocks();
        //printf("Blocks sent: %d\n", blocks_sent);
        cv::imshow( windowName, sendFrame ); // visualize frame to be set
        if( cv::waitKey(33) >= 0 ) break;
        sendFrame.release();
        videoSource.grab();
    }while (videoSource.retrieve(sendFrame));
}

void VideoCommunication::receive(){
    long recv_blocks;
    std::string windowName = "Received Video Stream";
    cv::namedWindow( windowName, CV_WINDOW_FREERATIO);
    do{
        recv_blocks = receiveBlocks();
        //printf("Number of receieved of blocks: %ld\n", recv_blocks);
        updateFrame(recvFrame, blocksReceieved);
        cv::imshow(windowName, recvFrame ); // visualize frame to be set
        if( cv::waitKey(30) >= 0 ) break;
    }while (1);
}
