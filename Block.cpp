//
//  Block.cpp
//  SmartVideoChat
//
//  Created by Artem Lenskiy on 7/15/14.
//  Copyright (c) 2014 Artem Lenskiy. All rights reserved.
//

#include "Block.h"

/*
Block::Block(){
    
}
*/

Block::Block(unsigned blkSize, unsigned matType){
    //mat = cv::Mat(0, 0, matType, data);
    total_block_length = blkSize * blkSize * 3 + BLOCK_HEADER_LENGTH;
    //data = cv::Ptr<uchar>(new uchar[total_block_length]);
    data.reset( new uchar[total_block_length], std::default_delete<uchar[]>() );
    //data = new uchar[total_block_length];
    //mat.release();
    mat = cv::Mat(blkSize, blkSize, matType, data.get() + BLOCK_HEADER_LENGTH);
    p_blk_header = (unsigned *) data.get();
    //p_sq_number = (unsigned short *) data.get();
    //p_compr_factor = (unsigned short *) (data.get() + 2);
    //*p_compr_factor = 1;
    setCompParams(0);
    setSqId(0);
    setBlockSize(blkSize);

}

Block::Block( const Block& other){
    //counter++;
    total_block_length = other.total_block_length;
    data = other.data;
    mat = other.mat;
    p_blk_header = other.p_blk_header;
    //p_sq_number = other.p_sq_number;
    //p_compr_factor = other.p_compr_factor;
 }

Block::Block(cv::Mat& inmat){
    //mat = cv::Mat(0, 0, matType, data);
    total_block_length = inmat.cols * inmat.rows * inmat.elemSize() + BLOCK_HEADER_LENGTH;
    //data = cv::Ptr<uchar>(new uchar[total_block_length]);
    data.reset( new uchar[total_block_length], std::default_delete<uchar[]>() );
    //data = new uchar[total_block_length];
    //mat.release();
    mat = cv::Mat(inmat.cols, inmat.rows, inmat.type(), data.get() + BLOCK_HEADER_LENGTH);
    inmat.copyTo(mat);
    p_blk_header = (unsigned *) data.get();
    //p_sq_number = (unsigned short *) data.get();
    //p_compr_factor = (unsigned short *) (data.get() + 2);
    //*p_compr_factor = 1;
    setCompParams(0);
    setSqId(0);
    setBlockSize(MAX(inmat.cols, inmat.rows));
}


Block& Block::operator=(const Block &rhs){
    
    if (this == &rhs)
        return *this;
    
    if(this->getBlockSize() != rhs.getBlockSize()){
        mat.release();
        data.reset( new uchar[rhs.mat.cols * rhs.mat.rows * rhs.mat.elemSize() + BLOCK_HEADER_LENGTH], std::default_delete<uchar[]>() );
        p_blk_header = (unsigned *) data.get();
        mat = cv::Mat(rhs.mat.cols, rhs.mat.rows, rhs.mat.type(), data.get() + BLOCK_HEADER_LENGTH);
    }
    
    memcpy(data.operator->(), rhs.data.operator->(), BLOCK_HEADER_LENGTH);
    rhs.mat.copyTo(mat);
    return *this;
}

void Block::setCompParams(unsigned params){
    *p_blk_header = *p_blk_header & 0xFFFFFE00;
    *p_blk_header = *p_blk_header | params;
}

unsigned Block::getCompParams() const{
    return  *p_blk_header & 0x000001FF;
}

void Block::setBlockSize(unsigned size){
    unsigned block_size_code = 0;
    *p_blk_header = *p_blk_header & 0xFFFFF1FF;
    while (size >>= 1) { ++block_size_code; } // calculate log_2(size), os if size is 16 then block_size_code is 4
    *p_blk_header = *p_blk_header | block_size_code << 9;
}
unsigned Block::getBlockSize() const{
    return  1 << ((*p_blk_header & 0x00000E00) >> 9);
}

void Block::setSqId(unsigned sq_number){
    *p_blk_header = *p_blk_header & 0x00000FFF;
    *p_blk_header = *p_blk_header | sq_number << 12;
}

unsigned  Block::getSqId() const{
    return  (*p_blk_header & 0xFFFFF000) >> 12;
}

void Block::setToZero(){
    memset(data.operator->(), 0, total_block_length);
}

size_t Block::getSize() const{
    return total_block_length;
}



void Block::print() const{
    if (&mat == NULL)
        return;
    unsigned char *ptr;
    for (int  j = 0; j < mat.rows; j++) {
        for (int  i = 0; i < mat.cols; i++){
           // ptr = (mat.row(j).data + i*3);
           // printf("\t<%d %d %d>", *(ptr), *(ptr + 1), *(ptr + 2));
            ptr = mat.data + j * mat.cols * mat.elemSize() + i*3;
            printf("\t[%d %d %d]", *(ptr), *(ptr + 1), *(ptr + 2));
        }
        printf("\n");
    }
}

Block::~Block(){
    data.reset();
    mat.release();
}
/*
long Block::pack(char *contBlock, unsigned short comp_coef) const{
    if (!contBlock)
        return -1;
    
    Block bl(block.cols/comp_coef, block.type());
    cv::resize(block, bl.block, cv::Size(block.cols/comp_coef, block.rows/comp_coef));
    
    // *((unsigned short *)contBlock) = sq_number;
    // *((unsigned short *)(contBlock + sizeof(Block::sq_number))) = comp_coef;
    
    //for (int i = 0; i < bl.block.rows; i++){
    //    memcpy(contBlock + sizeof(Block::sq_number) + sizeof(comp_coef) + i * bl.block.cols * bl.block.elemSize(), bl.block.row(i).data, bl.block.cols * bl.block.elemSize());
    //}
    
    memcpy(contBlock + sizeof(Block::sq_number) + sizeof(comp_coef), bl.block.data, bl.block.cols * bl.block.rows * bl.block.elemSize());
    
    return (long) sizeof(Block::sq_number) + sizeof(comp_coef) + bl.block.cols * bl.block.rows * bl.block.elemSize();
}
*/
/*
long Block::unpack(const char *contBlock){
    if (!contBlock) //contBlock is zero
        return -1;
    
    if (block.cols == 0 || block.rows == 0) //the block is empty
        return -2;

    sq_number = *((unsigned short *)contBlock);
    unsigned short comp_coef = *((unsigned short *)(contBlock + sizeof(Block::sq_number)));
    
    Block bl(block.cols/comp_coef, block.type());
    
    //for (int i = 0; i < bl.block.rows; i++)
    //    memcpy(bl.block.row(i).data, contBlock + sizeof(Block::sq_number)+ sizeof(comp_coef)  + i * bl.block.cols*block.elemSize(), bl.block.cols * bl.block.elemSize());
    
    memcpy(bl.block.data, contBlock + sizeof(Block::sq_number)+ sizeof(comp_coef), bl.block.rows * bl.block.cols * bl.block.elemSize());
    
    cv::resize(bl.block, block, cv::Size(block.cols, block.rows));
    return 0;
}
*/
/*
void Block::compress(std::vector<Block> &blocksToSend, unsigned comp_coef){
    if(blocksToSend.size() == 0) return;
    std::vector<Block>::iterator it1 = blocksToSend.begin();
    cv::Size reduced(it1->block.cols/comp_coef, it1->block.rows/comp_coef);
    
    cv::Mat temp(reduced.height, reduced.width, it1->block.type());
    
    for (;it1 != blocksToSend.end(); ++it1){
        cv::resize(it1->block, temp, reduced);
        it1->block.release();
        temp.copyTo(it1->block);
    }
    temp.release();
}

void Block::decompress(std::vector<Block> &receivedBlocks, unsigned comp_coef){
    if(receivedBlocks.size() == 0) return;
    std::vector<Block>::iterator it2 = receivedBlocks.begin();
    cv::Size increase(it2->block.cols*comp_coef, it2->block.rows*comp_coef);
    
    cv::Mat temp(increase.height, increase.width, it2->block.type());
    
    for (;it2 != receivedBlocks.end(); ++it2){
        cv::resize(it2->block, temp, increase);
        it2->block.release();
        temp.copyTo(it2->block);
    }
    temp.release();
}
*/






