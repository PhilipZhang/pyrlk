/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : match.cpp

* Purpose :

* Creation Date : 20-05-2013

* Last Modified : Thu 30 May 2013 04:21:30 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include <iostream>
#include <string.h>
#include <cv.hpp>
#include <highgui.h>

using namespace std;
using namespace cv;
int gb_max_corners = 4000;
int gb_win_size = 32;
int gb_layer_count = 5;
int gb_error_value = 550;
char gb_output_name[256] = "output.jpg";

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
	char status[gb_max_corners];
	float errors[gb_max_corners];
	CvPoint2D32f dest_features[gb_max_corners];
	int cur_count = gb_max_corners;
	cvCalcOpticalFlowPyrLK(img1, img2, NULL, NULL, corners, dest_features, cur_count,
						   cvSize(gb_win_size, gb_win_size), gb_layer_count, status,
						   errors, cvTermCriteria(CV_TERMCRIT_ITER, 5, 0.1), 0);
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
		if(status[i] == 1 && errors[i] <= gb_error_value)
		{
			cvCircle(pResult, cvPointFrom32f(corners[i]), 1, CV_RGB(255, 255, 0), -1, CV_AA);
			cvLine(pResult, cvPointFrom32f(corners[i]), cvPointFrom32f(dest_features[i]),
				   CV_RGB(255, 0, 0), 1, CV_AA);
			total_match ++;
		}
	}
	cout << total_match << " matches found." << endl;
	cvSaveImage(gb_output_name, pResult);

	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&tmpImage);
	cvReleaseImage(&tmpImage2);
	cvReleaseImage(&pResult);
	return 0;
}
