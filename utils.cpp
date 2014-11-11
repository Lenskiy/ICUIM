//
//  util.cpp
//  Video_compression
//
//  Created by Artem Lenskiy on 5/24/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#include "utils.h"
#include "Block.h"
#include <jpeglib.h>
#include <setjmp.h>


typedef struct _JFIFHeader
{
    uchar SOI[2];          /* 00h  Start of Image Marker     */
    uchar APP0[2];         /* 02h  Application Use Marker    */
    uchar Length[2];       /* 04h  Length of APP0 Field      */
    uchar Identifier[5];   /* 06h  "JFIF" (zero terminated) Id String */
    uchar Version[2];      /* 07h  JFIF Format Revision      */
    uchar Units;           /* 09h  Units used for Resolution */
    uchar Xdensity[2];     /* 0Ah  Horizontal Resolution     */
    uchar Ydensity[2];     /* 0Ch  Vertical Resolution       */
    uchar XThumbnail;      /* 0Eh  Horizontal Pixel Count    */
    uchar YThumbnail;      /* 0Fh  Vertical Pixel Count      */
} JFIFHEAD;

void printQTables(const unsigned char *buf, unsigned len){
    
    unsigned char zigzag[] = {0,11,15,16,14,15,27,28,2,4,7,13,16,26,29,42,3,8,12,17,25,30,41,43,9,11,18,24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,33,38,46,51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63};
    
    unsigned Height,Width;
    int k = 4;
    JFIFHEAD *jc = (JFIFHEAD *) (buf);
    unsigned short block_length = jc->Length[0] * 256 + jc->Length[1];
    do{
        k += block_length;
        if(buf[k + 1] == 0xC0){
            Height = buf[k + 5] * 256 + buf[k + 6];
            Width =  buf[k + 7] * 256 + buf[k + 8];
            k += 2;
            block_length = buf[k] * 256 + buf[k + 1];
        }else if(buf[k + 1] == 0xDB){
            printf("\n--------- QUANTIZATION TABLE ---------\n");
            for (int j = 0; j < 8; j++) {
                for (int i = 0; i < 8; i++) {
                    printf("%u\t", buf[k + 4 +  zigzag[j * 8 + i]]);
                }
                printf("\n");
            }
            k += 2;
            block_length = buf[k] * 256 + buf[k + 1];
        }else{
            k += 2;
            block_length = buf[k] * 256 + buf[k + 1];
        }
        
    }while (k < len);
}


Resample::Resample(VideoCommunication *vc):vc(vc){
}


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

Block &Resample::decompress(Block &bl){
    unsigned unCompressedSized = bl.getBlockSize() * (1 << bl.getCompParams());
    cv::Mat temp;
    cv::resize(bl.mat, temp, cv::Size(unCompressedSized, unCompressedSized)); //, 0, 0, cv::INTER_LANCZOS4  //Upsample
    Block compressed_block(temp);
    compressed_block.setSqId(bl.getSqId());
    compressed_block.setCompParams(bl.getCompParams());
    bl = compressed_block;
    bl.total_block_length = unCompressedSized * unCompressedSized * bl.mat.elemSize() + BLOCK_HEADER_LENGTH;
    return bl;
}


JPEGCompression::JPEGCompression(VideoCommunication *vc):vc(vc){
    cv::Mat t = cv::Mat(16,16, CV_8UC3, cv::Scalar(0,0,0));
    
    jpg_headers = (uchar **) malloc(101 * sizeof(uchar *));
    cv::vector<int> params;

    for (int q = 1; q <= 100; q++) {
        params.push_back(CV_IMWRITE_JPEG_QUALITY);
        params.push_back(q);
        
        jpg_headers[q] = (uchar *)malloc(612); //610 bytes is occupied by the JPG header
        cv::imencode(".jpg", t, buf, params);
        memcpy(jpg_headers[q], buf.data(), 612);
        printf("q = %d", q);
        printQTables(buf.data(), 612);
        
        params.pop_back();
        params.pop_back();
    }
    
    block_buffer = (uchar *)malloc(128 * 128 * 3); // just allocote memory to store compressed block
}



Block &JPEGCompression::compress(Block &blk){
    

//    struct jpeg_compress_struct cinfo;
//    struct jpeg_error_mgr jerr;
//    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
//    
//    cinfo.err = jpeg_std_error(&jerr);
//    jpeg_create_compress(&cinfo);
//    unsigned long buf_size = 0;
//    unsigned char *buf = NULL;
//    jpeg_mem_dest(&cinfo, &buf, &buf_size);
//    cinfo.image_width = blk.getBlockSize(); 	/* image width and height, in pixels */
//    cinfo.image_height = blk.getBlockSize();
//    cinfo.input_components = 3;		/* # of color components per pixel */
//    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
//    jpeg_set_defaults(&cinfo);
//    jpeg_set_quality(&cinfo, 10, TRUE /* limit to baseline-JPEG values */);
//    jpeg_start_compress(&cinfo, TRUE);
//    int row_stride = blk.getBlockSize() * 3;
//    
//    while (cinfo.next_scanline < cinfo.image_height) {
//        /* jpeg_write_scanlines expects an array of pointers to scanlines.
//         * Here the array is only one element long, but you could pass
//         * more than one scanline at a time if that's more convenient.
//         */
//        row_pointer[0] = (blk.data.get() + BLOCK_HEADER_LENGTH + cinfo.next_scanline * row_stride);
//        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
//    }
//    jpeg_finish_compress(&cinfo);
//    memcpy(blk.data.get(), buf, buf_size);
//    free(buf);
//    jpeg_destroy_compress(&cinfo);
//    return blk;
    
    cv::vector<int> params;
    params.push_back(CV_IMWRITE_JPEG_QUALITY);
    params.push_back(20 * blk.getCompParams());
    cv::imencode(".jpg", blk.mat, buf, params);

//    for (int i = 0; i < 612; i++) {
//        printf("%x ", (unsigned)buf[i]);
//    }
    
//    for (int i = 612; i < buf.size(); i++) {
//        printf("%x ", (unsigned)buf[i]);
//    }
    for (int i = 612; i < buf.size(); i++) {
        blk.mat.data[i - 612] = buf[i];
    }
    blk.total_block_length = buf.size() + BLOCK_HEADER_LENGTH - 612;
    return blk;
 
}



Block &JPEGCompression::decompress(Block &blk){
//    struct jpeg_decompress_struct cinfo;
//    JSAMPARRAY buffer;		/* Output row buffer */
//    int row_stride;		/* physical row width in output buffer */
//    jpeg_create_decompress(&cinfo);
//    unsigned long buf_size = NULL;
//    unsigned char *buf = NULL;
//    jpeg_mem_src(&cinfo, blk.data.get() + BLOCK_HEADER_LENGTH, 100);
//    (void) jpeg_read_header(&cinfo, TRUE);
//    (void) jpeg_start_decompress(&cinfo);
//     row_stride = cinfo.output_width * cinfo.output_components;
//    buffer = (*cinfo.mem->alloc_sarray)
//    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
//    while (cinfo.output_scanline < cinfo.output_height) {
//        /* jpeg_read_scanlines expects an array of pointers to scanlines.
//         * Here the array is only one element long, but you could ask for
//         * more than one scanline at a time if that's more convenient.
//         */
//        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
//        /* Assume put_scanline_someplace wants a pointer and sample count. */
//        //put_scanline_someplace(*blk.data.get(), row_stride);
//    }
//    (void) jpeg_finish_decompress(&cinfo);
//    //jpeg_destroy_decompress(&cinfo); // do not call it, the memory will be reused
//    memcpy(blk.data.get(), buf, buf_size);
//    free(buf);
//    return blk;
//    
    memcpy(block_buffer, jpg_headers[20 * blk.getCompParams()], 612);
    memcpy(block_buffer + 612, blk.mat.data, blk.total_block_length - BLOCK_HEADER_LENGTH);

//    for (int i = 0; i < 612; i++) {
//        printf("%x ", (unsigned)block_buffer[i]);
//    }

    
//    for (int i = 0; i < blk.total_block_length - BLOCK_HEADER_LENGTH + 612; i++) {
//        printf("%x ", (unsigned)block_buffer[i]);
//    }
    
//    for (int i = 612; i < blk.total_block_length - BLOCK_HEADER_LENGTH + 612; i++) {
//        printf("%x ", (unsigned)block_buffer[i]);
//    }
    
    cv::Mat temp_mat = cv::imdecode(cv::Mat(1, blk.total_block_length - BLOCK_HEADER_LENGTH + 612, CV_8UC1,block_buffer), CV_LOAD_IMAGE_COLOR);
    memcpy(blk.mat.data, temp_mat.data, temp_mat.cols * temp_mat.rows * 3);
    blk.total_block_length = blk.getBlockSize() * blk.getBlockSize() * blk.mat.elemSize() + BLOCK_HEADER_LENGTH;
    return blk;
     
}


VideoCommunication::VideoCommunication():roi(this),compress(this){
    socket_to_send = 0;
}

VideoCommunication::VideoCommunication(unsigned short port_send, unsigned short port_recv, const std::string ip_address):roi(this),compress(this),block_length(0),contBlock(0){
    socket_to_send = 0;
    socket_to_recv = 0;
    curBeginning = 0;
    numOfPackRecv = 0;
    numOfBlocksPerFrame = 0;
    nextFrameBlock = 0;
    
    if(ip_address.length() > 0)
        networkInit(port_recv, port_send, ip_address);
    else
        networkInit(port_recv, port_send, "127.0.0.1");//10.72.51.86 //10.72.51.103 maxim
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
    
//    int tos=5;
//    socklen_t toslen=0;
//    
//    if (setsockopt(socket_to_recv, IPPROTO_IP, IP_TOS,  &tos, sizeof(toslen)) < 0) {
//        perror("error to get option");
//    }else {
//        printf ("changing tos opt = %d\n",tos);
//    }
//
//    
//    tos = 0;
//    if (getsockopt(socket_to_recv, IPPROTO_IP, IP_TOS,  &tos, &toslen) < 0) {
//        perror("error to get option");
//    }else {
//        printf ("current tos opt = %d\n",tos);
//    }
//    
    
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
    err = getaddrinfo(ip_address.c_str(), port_str, &clie_addr, &res_clie_addr); /////////////
    if (err!=0) {
        printf("%s", gai_strerror(err));
        abort();
    }
    if((socket_to_send = socket(res_clie_addr->ai_family, res_clie_addr->ai_socktype, res_clie_addr->ai_protocol)) < 0){
        printf("\n Error : Could not create socket \n");
        abort();
    }
    
//    if (setsockopt(socket_to_send, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(int)) == -1) {
//        fprintf(stderr, "Error setting socket opts: %s\n", strerror(errno));
//    }
//    
//    tos=5;
//    if (setsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, sizeof(toslen)) < 0) {
//        perror("error to get option");
//    }else {
//        printf ("changing tos opt = %d\n",tos);
//    }
//    
//    
//    //tos = 0;
//    if (getsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, &toslen) < 0) {
//        perror("error to get option");
//    }else {
//        printf ("current tos opt = %d\n",tos);
//    }
    
}

void VideoCommunication::setVideoSource(cv::VideoCapture vs){
    videoSource = vs;
}

void VideoCommunication::initializeVideoParams(unsigned bs, unsigned width, unsigned height){
    params.blockSize = bs;
    
    //videoSource.grab();
    //videoSource.retrieve(sendFrame);
    params.width = width;//sendFrame.cols;
    params.height = height;//sendFrame.rows;
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
    cv::Mat temp(bl.getBlockSize(), bl.getBlockSize(), CV_8UC3, bl.mat.data);
    bl.mat = temp;
    //bl.mat = bl.mat.reshape(0, bl.getBlockSize());
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
//    int tos;
//    if (bl.getCompParams() != 0)
//        tos = 0;
//    else
//        tos = 7;
//    
//    if (setsockopt(socket_to_send, IPPROTO_IP, IP_TOS,  &tos, sizeof(tos)) < 0)
//        perror("error to get option");
//    else
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
    unsigned tmp;
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
            
            //if(roi.isInRoi(bl))
            //    bl.setCompParams(0);
            //else
            //    bl.setCompParams(3);
            tmp = roi.isInRoi(bl);
            bl.setCompParams(5 - roi.isInRoi(bl));
            
            imageBlocks.at(sq_number) = bl;
            sq_number++;
        }
    }
    
//    printf("(%d)\n", imageBlocks.at(100).getSqId());
//    printf("(%d)\n", imageBlocks.at(100).getBlockSize());
//    printf("(%d)\n", imageBlocks.at(100).getCompParams());
    //for (unsigned i = 0; i < numOfBlocksPerFrame; i++)
    //    compress.decompress(compress.compress(imageBlocks.at(i))); //////////////////////// check is we send sqe = 0
//    printf("(%d)\n", imageBlocks.at(100).getSqId());
//    printf("(%d)\n", imageBlocks.at(100).getBlockSize());
//    printf("(%d)\n", imageBlocks.at(100).getCompParams());
    
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
    cv::Mat mask3(baseFrame.size(), CV_8UC1);
    cv::Mat mask2(baseFrame.size(), CV_8UC1);
    mask3 = cv::Scalar(0);
    mask2 = cv::Scalar(0);
    cv::Mat smoothed3(baseFrame.size(), CV_8UC3);
    cv::Mat smoothed2(baseFrame.size(), CV_8UC3);
    
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
//        if (bl->getCompParams() == 3 ) {
//            mask3(cv::Rect(block_x, block_y, bl->getBlockSize(), bl->getBlockSize())).setTo(cv::Scalar(1));
//        }
//        if (bl->getCompParams() == 2 || bl->getCompParams() == 1) {
//            mask2(cv::Rect(block_x, block_y, bl->getBlockSize(), bl->getBlockSize())).setTo(cv::Scalar(1));
//        }
//        if(bl->getCompParams() != 0 ){
//            printf("(%d)\n", bl->getSqId());
//            printf("(%d)\n", bl->getBlockSize());
//            printf("(%d)\n", bl->getCompParams());
//            //printf("(%d,\t%d)\n", block_x, block_y);
//            //bl->print();
//        }
    }
    
    //std::string windowName = "Mask";
    //cv::namedWindow( windowName, CV_WINDOW_FREERATIO);

    //GaussianBlur(baseFrame, smoothed3, cv::Size(0, 0), 3, 3);
    //GaussianBlur(baseFrame, smoothed2, cv::Size(0, 0), 1, 1);
    //cv::imshow( windowName, smoothed);
    //smoothed3.copyTo(baseFrame, mask3);
    //smoothed2.copyTo(baseFrame, mask2);
}


void VideoCommunication::transmit(){
    std::string windowName = "Broadcating Video Stream";
    cv::namedWindow( windowName, CV_WINDOW_FREERATIO);
    videoSource.grab();
    videoSource.retrieve(sendFrame);
    int blocks_sent = 0;
    //unsigned long t1;
    do{
        //t1 = clock();
        getBlocks(sendFrame);
        blocks_sent = sendBlocks();
        //printf("Blocks sent: %d\n", blocks_sent);
        cv::imshow( windowName, sendFrame ); // visualize frame to be set
        if( cv::waitKey(33) >= 0 ) break;
        sendFrame.release();
        videoSource.grab();
        //printf("%lf\n",(double)(clock() - t1)/CLOCKS_PER_SEC);
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
