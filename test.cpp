#include <stdio.h>
#include <stdlib.h>

//------------------------------------------------------------------------
void **convolve2D(float **image, float **newImage, int im_rows, int im_cols, float **kernel, int ker_rows, int ker_cols, int *min, int *max){
    int ker_centerX = ker_cols / 2;
    int ker_centerY = ker_rows / 2;
    int i, j, m, n, ii, jj, mm, nn;
    
    for(i=0; i < im_rows; ++i)
    {
        for(j=0; j < im_cols; ++j)         // counts for columns
        {
            for(m=0; m < ker_rows; ++m)     // counts for kernel rows
            {
                mm = ker_rows - 1 - m;      // index of a flipped kernel row
                
                for(n=0; n < ker_cols; ++n) // counts for kernel columns
                {
                    nn = ker_cols - 1 - n;  // index of flipped kernel column
                    
                    // checks boundary
                    ii = i + (m - ker_centerY);
                    jj = j + (n - ker_centerX);
                    
                    // removes input values that are out of boundary
                    if( ii >= 0 && ii < im_rows && jj >= 0 && jj < im_cols )
                    				newImage[i][j] += image[ii][jj] * kernel[mm][nn];
                }
            }
        }
    }
    // calculates min and max values
    for(i=0; i < im_rows; i++){
        for(j=0; j < im_cols; j++){
            if ( newImage[i][j] <  *min)
                *min =  newImage[i][j];
            else if (newImage[i][j] > *max)
                *max =  newImage[i][j];
        }
    }
    
}

void normalize(int im_rows, int im_cols, float **newImage, int min, int max){
    int newMin=0, newMax=255, newRange=255, range, i, j;
    float rate;
    // finds the range
    range = max - min;
    // uses range to calcuate rate of current image and then uses this rate to normalize the picture
    for(i = 0; i < im_rows; i++){
        for(j=0; j < im_cols; j++){
            rate = (newImage[i][j]- min) / range;
            newImage[i][j]=(int)(newRange * rate);
        }
    }
}



float** mat_create_float(int rows, int cols){
    int j;
    float **mat = (float **)malloc(rows * sizeof(float *));
    for (j = 0; j < rows; j++){
        mat[j] = (float *)malloc(cols * sizeof(float));
    }
    return mat;
}
//-----------------------------------------------------------------------------------
void mat_release_float(float **mat, int rows){
    int j;
    for(j = 0; j < rows; j++)
        free(mat[j]);
    free(mat);
}
//-----------------------------------------------------------------------------------
float **readImage(const char *file_name, int *rows, int *cols, int *max_gray){
    FILE *pgmFile;
    char version[3];
    int i,j;
    
    pgmFile = fopen(file_name,"rb");
    if(pgmFile == NULL){
        fprintf(stderr, "cannot open file to read");
        exit(EXIT_FAILURE);
    }
    
    fgets(version, sizeof(version), pgmFile);
    fscanf(pgmFile, "%d %d %d", cols, rows, max_gray);
    fgetc(pgmFile);
    
    float **mat = mat_create_float(*rows, *cols);
    
    for (j = 0; j < *rows; j++){
        for(i = 0; i < *cols; ++i){
            mat[j][i] = (float)fgetc(pgmFile);
        }
    }
    fclose(pgmFile);
    return mat;
}
//-----------------------------------------------------------------------------------
void writeImage(const char *file_name, int rows, int cols, int levels, float **mat){
    FILE *pgmFile;
    int i,j;
    
    pgmFile = fopen(file_name,"wb");
    if(pgmFile == NULL){
        fprintf(stderr, "cannot open file to read");
        exit(EXIT_FAILURE);
    }
    
    fprintf(pgmFile, "P5 ");
    fprintf(pgmFile, "%d %d %d ", cols, rows, levels);
    
    for (j = 0; j < rows; j++){
        for(i = 0; i < cols; ++i){
            fputc(mat[j][i], pgmFile);
        }
    }
    fclose(pgmFile);
    mat_release_float(mat, rows);
}
//-----------------------------------------------------------------------------------
float **readMatrix(const char *file_name, int *rows, int *cols){
    FILE *matFile;
    char version[3];
    int i,j;
    
    matFile = fopen(file_name,"r");
    if(matFile == NULL){
        fprintf(stderr, "cannot open file to read");
        exit(EXIT_FAILURE);
    }
    
    fscanf(matFile, "%d", cols);
    fscanf(matFile, "%d", rows);
    fgetc(matFile);
    
    float **mat = mat_create_float(*rows, *cols);
    
    for (j = 0; j < *rows; j++){
        for(i = 0; i < *cols; ++i){
            fscanf(matFile, "%f", &mat[j][i]);
        }
    }
    fclose(matFile);
    return mat;
}
//-----------------------------------------------------------------------------------
void print_float_matrix(float **mat, int rows, int cols){
    int i,j;
    for (j = 0; j < rows; j++){
        for(i = 0; i < cols; i ++)
            printf("%4.2f\t", mat[j][i]);
        printf("\n");
    }
}
//-----------------------------------------------------------------------------------
int main(int argc, char* argv[]){
    
    int imcols, imrows, imlevels, kercols, kerrows, min = 0, max =0;
    if(argc != 3){
        printf("Usage: %s <image_file> <kernel_file>", argv[0]);
        exit(0);
    }
    
    float **image  = readImage(argv[1], &imrows, &imcols, &imlevels);
    float **newImage = mat_create_float(imrows, imcols);
    float **kernel = readMatrix(argv[2], &kerrows, &kercols);
    
    convolve2D(image, newImage, imrows, imcols, kernel, kerrows, kercols, &min, &max);
    //writeImage("frog_kernel.pbm", imrows, imcols, imlevels, newImage);
    
    normalize(imrows,imcols,newImage, min, max);
    writeImage("frog_copy.pbm", imrows, imcols, imlevels, newImage);
    return 1;
}
