# include <opencv2/opencv.hpp>
# include <iostream>
#include <time.h>
#include<armor.h>

using namespace cv;
using namespace std;

#define CV_RETR_EXTERNAL 0
#define CV_RETR_LIST 1
#define CV_RETR_CCOMP 2
#define CV_RETR_TREE 3
#define CV_CHAIN_CODE 0
#define CV_CHAIN_APPROX_NONE 1
#define CV_CHAIN_APPROX_SIMPLE 2
#define CV_CHAIN_APPROX_TC89_L1 3
#define CV_CHAIN_APPROX_TC89_KCOS 4
#define CV_LINK_RUNS 5

enum
{
    WIDTH_GREATER_THAN_HEIGHT,
    ANGLE_TO_UP
};

/*
*	@Brief:		// regulate the rotated rect
*	@Input:		// rotated rec
*				// regulation mode
*	@Return:	// regulated rec
*/
cv::RotatedRect& adjustRec(cv::RotatedRect& rec, const int mode)
{
    using std::swap;

    float& width = rec.size.width;
    float& height = rec.size.height;
    float& angle = rec.angle;

    if (mode == WIDTH_GREATER_THAN_HEIGHT)
    {
        if (width < height)
        {
            swap(width, height);
            angle += 90.0;
        }
    }

    while (angle >= 90.0) angle -= 180.0;
    while (angle < -90.0) angle += 180.0;

    if (mode == ANGLE_TO_UP)
    {
        if (angle >= 45.0)
        {
            swap(width, height);
            angle -= 90.0;
        }
        else if (angle < -45.0)
        {
            swap(width, height);
            angle += 90.0;
        }
    }

    return rec;
}
cv::Mat _roiImg;
Mat shibie(Mat src) {
    // 图片路径换成本地的图片路径，注意是两个斜杠
    //Mat src = imread("C:\\Users\\79230\\Pictures\\zhuangjia.png");
    //请一定检查是否成功读图
    //if (src.empty())
    //{
    //cout << "Can not load image." << endl;
    //return 1;
    //}
    //imshow("input image", src);

    ///*
    //	将RGB颜色空间转到HSV颜色空间 灰度：CV_BGR2GRAY
    //*/
    //Mat hsvimg;
    //cvtColor(src, hsvimg, COLOR_BGR2HSV);
    //imshow("test", hsvimg);
    //waitKey(0);
    // 把一个3通道图像转换成3个单通道图像
#define _enemy_color 1  //blue:1 red:0
    vector<Mat> channels;
    Mat _grayImg;
    std::vector<LightDescriptor> lightInfos;  //rec xiangliang
    split(src, channels);
    //预处理删除己方装甲板颜色
    if (_enemy_color == 0)
        _grayImg = channels.at(2) - channels.at(0);//Get red-blue image;
    else _grayImg = channels.at(0) - channels.at(2);//Get blue-red image;
    //imshow("2", channels.at(2));
    //imshow("0", channels.at(0));
    //imshow("预处理删除己方装甲板颜色", _grayImg);
    //waitKey(0);

    //	二值化

    //int LowH = 60;
    //int LowS = 120;
    //int LowV = 245;
    //int HighH = 120;
    //int HighS = 255;
    //int HighV = 255;
    //vector<Mat> hsvsplit;
    //split(hsvimg, hsvsplit);
    //equalizeHist(hsvsplit[2], hsvsplit[2]);

    //merge(hsvsplit, hsvimg);

    //Mat thresholdimg;
    //inRange(hsvimg, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), thresholdimg);
    //imshow("二值化后的图片", thresholdimg);
    //waitKey(0);


    Mat binBrightImg;
    //阈值化
    threshold(_grayImg, binBrightImg, 210
              , 255, cv::THRESH_BINARY);
    //imshow("阈值化", binBrightImg);
    //waitKey(0);
    //膨胀
    Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    dilate(binBrightImg, binBrightImg, element);
    //imshow("膨胀", binBrightImg);
    //waitKey(0);
    /*
        找轮廓
    */
    vector<vector<Point>> lightContours;
    findContours(binBrightImg.clone(), lightContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    //drawContours(src, lightContours,  -1, Scalar(0, 255, 0),1);
    //imshow("轮廓", src);
    //waitKey(0);

    for (const auto& contour : lightContours)
    {
        //得到面积
        float lightContourArea = contourArea(contour);
        //面积太小的不要
        if (contour.size() <= 5 || lightContourArea < 10) continue;
        //椭圆拟合区域得到外接矩形
        RotatedRect lightRec = fitEllipse(contour);
        //矫正灯条
        adjustRec(lightRec, ANGLE_TO_UP);
        //宽高比、凸度筛选灯条
        if (lightRec.size.width / lightRec.size.height >
                1.0 ||
                lightContourArea / lightRec.size.area() <
                0.5
                )continue;
        //对灯条范围适当扩大
        lightRec.size.width *= 1.1;
        lightRec.size.height *= 1.1;
        Rect lightRect = lightRec.boundingRect();
        //        const Rect srcBound(Point(0, 0), _roiImg.size());
        //        lightRect &= srcBound;
        rectangle(src, lightRect, cv::Scalar(0, 255, 0), 2, 4);
        lightInfos.push_back(LightDescriptor(lightRec));
    }

    //
    sort(lightInfos.begin(), lightInfos.end(), [](const LightDescriptor& ld1, const LightDescriptor& ld2)
    {
        return ld1.center.x < ld2.center.x;
    });


    return src;
}

int main()
{
    VideoCapture capture(0);
    //    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    //    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    Mat frame;
    time_t start, end;
    int n=0;
    //capture.set(CV_CAP_PROP_FPS, 330);//fps
    time(&start);
    while (capture.read(frame))
    {
        n++;
        if(n%120==0)
        {
            time(&end);
            double seconds = difftime (end, start);
            double fps  = n / seconds;
            cout << "Estimated frames per second : " << fps << endl;
        }
        frame = shibie(frame);
        imshow("video-demo", frame);
        char c = waitKey(1);
        if (c == 27)
        {
            break;
        }
    }
    return 0;
}
