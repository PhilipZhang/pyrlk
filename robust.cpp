/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : robust.cpp

* Purpose :

* Creation Date : 20-05-2013

* Last Modified : Mon 20 May 2013 07:05:20 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include <iostream>
#include <string.h>
#include <cv.hpp>
#include <highgui.h>
#include <algorithm>

using namespace std;
using namespace cv;
int gb_max_corners = 4000;
int gb_win_size = 64;
int gb_layer_count = 5;
int gb_error_value = 550;
char gb_output_name[256] = "output_robust.jpg";

void ShowHelp()
{
	cout << "Usage: pyrlk [Option] namelist" << endl
		 << "Option:" << endl
		 << " -c	maximum corner count(default 4000)" << endl
		 << " -e	error value(default 550)" << endl
		 << " -h	show help message" << endl
		 << " -l	pyramid layer count(default 5)" << endl
		 << " -o	set output file name" << endl
		 << " -w	window size(default 64)" << endl;

}

void ProcessParams(int argc, char **argv)
{
	int ch;
	while((ch = getopt(argc, argv, "c:e:hl:o:w:")) != -1)
	{
		switch(ch)
		{
		case 'c':
			gb_max_corners = atoi(optarg);
			break;
		case 'e':
			gb_error_value = atoi(optarg);
			break;
		case 'h':
			ShowHelp();
			exit(0);
			break;
		case 'l':
			gb_layer_count = atoi(optarg);
			break;
		case 'o':
			strcpy(gb_output_name, optarg);
			break;
		case 'w':
			gb_win_size = atoi(optarg);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	ProcessParams(argc, argv);
	IplImage *img1, *img2;
	// load 2 grayscale images into memory
	img1 = cvLoadImage("images/1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	img2 = cvLoadImage("images/2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	// find good features
	IplImage *tmpImage, *tmpImage2;
	tmpImage = cvCreateImage(cvGetSize(img1), IPL_DEPTH_32F, 1);
	tmpImage2 = cvCreateImage(cvGetSize(img2), IPL_DEPTH_32F, 1);
	CvPoint2D32f corners[gb_max_corners];
	int prev_count = gb_max_corners;
	cvGoodFeaturesToTrack(img1, tmpImage, tmpImage2, corners, &prev_count, 0.01, 5);
	cout << prev_count << " corners have been found in images/1.jpg" << endl;
	// track features in img2, input image is image1
	char status[gb_max_corners];
	float errors[gb_max_corners];
	CvPoint2D32f dest_features[gb_max_corners];
	int cur_count = gb_max_corners;
	cvCalcOpticalFlowPyrLK(img1, img2, NULL, NULL, corners, dest_features, cur_count,
						   cvSize(gb_win_size, gb_win_size), gb_layer_count, status,
						   errors, cvTermCriteria(CV_TERMCRIT_ITER, 5, 0.1), 0);
	// back track features in img1, input image is img2
	CvPoint2D32f back_features[gb_max_corners];
	int back_count = gb_max_corners;
	cvCalcOpticalFlowPyrLK(img2, img1, NULL, NULL, dest_features, back_features, back_count,
						   cvSize(gb_win_size, gb_win_size), gb_layer_count, status,
						   errors, cvTermCriteria(CV_TERMCRIT_ITER, 5, 0.1), 0);
	// Calculate the errors between source point and corresponding back track point
	float bk_errors[gb_max_corners];
	for(int i = 0; i < prev_count; i++)
	{
		bk_errors[i] = sqrt(
					(back_features[i].x - corners[i].x) * (back_features[i].x - corners[i].x)
				  + (back_features[i].y - corners[i].y) * (back_features[i].y - corners[i].y));
	}
	// only back track errors less than median is accepted.
	float errorsort[gb_max_corners];
	memcpy(errorsort, bk_errors, sizeof(float) * gb_max_corners);
	sort(errorsort, errorsort + prev_count);
	float emid = errorsort[(prev_count + 1) / 2];
	bool *is_bkerror_valids = new bool[prev_count];
	for(int i = 0; i < prev_count; i++)
		is_bkerror_valids[i] = false;
	for(int i = 0; i < prev_count; i++)
	{
		if(bk_errors[i] < emid)
		{
			is_bkerror_valids[i] = true;	
		}
	}
	float similarity[gb_max_corners];
	// calculate the similarity
	IplImage *src_square = cvCreateImage(cvSize(10, 10), IPL_DEPTH_8U, 1);
	IplImage *dest_square = cvCreateImage(cvSize(10, 10), IPL_DEPTH_8U, 1);
	IplImage *result = cvCreateImage(cvSize(1, 1), IPL_DEPTH_32F, 1);

	for(int i = 0; i < prev_count; i++)
	{
		cvGetRectSubPix(img1, src_square, corners[i]);
		cvGetRectSubPix(img2, dest_square, dest_features[i]);
		cvMatchTemplate(src_square, dest_square, result, CV_TM_CCOEFF_NORMED);
		similarity[i] = ((float *)(result->imageData))[0];
	}
	// only similarity greater than median is accepted.
	float simsort[gb_max_corners];
	memcpy(simsort, similarity, sizeof(float) * prev_count);
	sort(simsort, simsort + prev_count);
	float mid = simsort[(prev_count + 1) / 2];
	bool *is_sim_valids = new bool[prev_count];
	for(int i = 0; i < prev_count; i++)
		is_sim_valids[i] = false;
	for(int i = 0; i < prev_count; i++)
	{
		if(similarity[i] > mid)
		{
			is_sim_valids[i] = true;	
		}
	}

	cvReleaseImage(&src_square);
	cvReleaseImage(&dest_square);
	cvReleaseImage(&result);

	// Create visual effect of matching pairs
	IplImage *pResult = cvCreateImage(cvGetSize(img1), IPL_DEPTH_8U, 3);

	IplImage *pRed = cvCreateImage(cvGetSize(img1), IPL_DEPTH_8U, 1);
	IplImage *pGreen = cvCreateImage(cvGetSize(img1), IPL_DEPTH_8U, 1);
	IplImage *pBlue = cvCreateImage(cvGetSize(img1), IPL_DEPTH_8U, 1);
	IplImage *pPrev3C = cvLoadImage("images/1.jpg");
	IplImage *pCur3C = cvLoadImage("images/2.jpg");
	cvSplit(pPrev3C, pRed, NULL, pBlue, NULL);
	cvSplit(pCur3C, NULL, pGreen, NULL, NULL);
	cvMerge(pRed, pGreen, pBlue, NULL, pResult);
	cvReleaseImage(&pRed);
	cvReleaseImage(&pGreen);
	cvReleaseImage(&pBlue);
	cvReleaseImage(&pPrev3C);
	cvReleaseImage(&pCur3C);

	int total_match = 0;
	for(int i = 0; i < prev_count; i++)
	{
		if(status[i] == 1 && errors[i] <= gb_error_value && is_sim_valids[i] == true
		   && is_bkerror_valids[i] == true)
		{
			cvCircle(pResult, cvPointFrom32f(corners[i]), 1, CV_RGB(255, 255, 0), -1, CV_AA);
			cvLine(pResult, cvPointFrom32f(corners[i]), cvPointFrom32f(dest_features[i]),
				   CV_RGB(255, 0, 0), 1, CV_AA);
			total_match ++;
		}
	}
	cout << total_match << " robust matches." << endl;
	cvSaveImage(gb_output_name, pResult);
	delete is_sim_valids;
	delete is_bkerror_valids;

	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&tmpImage2);
	cvReleaseImage(&pResult);
	return 0;
}
